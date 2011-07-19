#include <types.h>
#include <err.h>
#include <user/apps_entry.h>
#include <user/assert.h>
#include <user/display.h>
#include <user/syscall.h>
#include <user/uart.h>
#include <user/name_server.h>
#include <user/lib/sync.h>
#include "inc/train.h"
#include "inc/config.h"
#include "inc/train.h"
#include "inc/warning.h"

typedef struct Train_event_s Train_event;
typedef struct Train_reply_s Train_reply;

typedef struct Train_command_s Train_command;

enum Train_event_type {
	TRAIN_UPDATE_TIME,
	TRAIN_SET_SPEED,
	TRAIN_REVERSE,
	TRAIN_SWITCH,
	TRAIN_CHECK_SWITCH,
	TRAIN_LAST_SENSOR,
	TRAIN_ALL_SENSORS,
	TRAIN_MODULE_SUICIDE,
	TRAIN_PRESSURE_TEST,
	TRAIN_SWITCH_ALL,
	TRAIN_EXECUTE
};

struct Train_event_s {
	uint event_type;
	uint args[2];
};

struct Train_reply_s {
	int result;
};

enum Train_execute_type {
	TRAIN_EXEC_TR,
	TRAIN_EXEC_RV,
	TRAIN_EXEC_SW,
	TRAIN_EXEC_SWOFF,
	TRAIN_EXEC_SENSOR
};

struct Train_command_s {
	uint command;
	int args[2];
};

static inline void setspeed( int train, int speed ){
	int status = Putc( COM_1, (char)speed );
	assert( status == ERR_NONE );
	status = Putc( COM_1, (char)train );
	assert( status == ERR_NONE );
}

static inline void reverse( int train ){
	int status = Putc( COM_1, (char)15 );
	assert( status == ERR_NONE );
	status = Putc( COM_1, (char)train );
	assert( status == ERR_NONE );
}

static inline void track_switch( int ui_tid, int track_id, char direction ){
	int status;
	status = switch_ui_update_id( ui_tid, track_id, direction );
	assert( status == ERR_NONE );
	if( direction == 'C' ){
		direction = 34;
	} else if( direction == 'S' ){
		direction = 33;
	} else {
		assert( 0 );
	}

	status = Putc( COM_1, direction );
	assert( status == ERR_NONE );
	status = Putc( COM_1, (char)track_id );
	assert( status == ERR_NONE );
}

static inline void track_switch_off() {
	int status = Putc( COM_1, (char)32 );
	assert( status == ERR_NONE );
}

static inline void delay( int ticks ) {
	int status = Delay( ticks );
	assert( status == ERR_NONE );
}

static inline int execute_command( Rbuf* command_buffer, char* switch_table, int switch_ui_tid ){
	Train_command command;
	Train_command temp;
	int i;
	int delay;
	int status = rbuf_get( command_buffer, ( uchar* ) &command );
	assert( status == 0 );
	while ( command.command == TRAIN_EXEC_SWOFF && !rbuf_empty( command_buffer ) ){
		status = rbuf_get( command_buffer, ( uchar* ) &temp );
		assert( status == 0 );
		
		if ( temp.command != TRAIN_EXEC_SWOFF ){
			status = rbuf_put( command_buffer, ( uchar* ) &command );
			assert( status == 0 );
			command.command = temp.command;
			command.args[0] = temp.args[0];
			command.args[1] = temp.args[1];
		}
	}
	
	switch( command.command ){
	case TRAIN_EXEC_TR:
		setspeed( command.args[0], command.args[1] );
		delay = TRAIN_XMIT_DELAY;
		break;
	case TRAIN_EXEC_RV:
		reverse( command.args[0] );
		delay = TRAIN_XMIT_DELAY;
		break;
	case TRAIN_EXEC_SW:
		i = SWID_TO_ARRAYID( command.args[0] );
		switch_table[ i ] = command.args[ 1 ];
		track_switch( switch_ui_tid, command.args[0], command.args[1] );
		delay = SWITCH_XMIT_DELAY;
		break;
	case TRAIN_EXEC_SWOFF:
		track_switch_off();
		delay = SWITCH_OFF_DELAY;
		break;
	case TRAIN_EXEC_SENSOR:
		status = Putc( COM_1, (char)133 );
		delay = TRAIN_XMIT_DELAY;
	default:
		delay = 0;
		break;
	}
	
	return delay;
}

static inline void buffer_command( Rbuf* command_buffer, uint command_type, int arg0, int arg1 ){
	Train_command command;
	command.command = command_type;
	command.args[0] = arg0;
	command.args[1] = arg1;
	int status = rbuf_put( command_buffer, ( uchar* )&command );
	assert( status == 0 );
}

static inline void track_switch_all( int ui_tid, char direction ){
	int i;
	// reset all switches
	for ( i = 1; i < 19; i++ ) {
		track_switch( ui_tid, i, direction );
		delay( SWITCH_XMIT_DELAY );
	}
	for ( i = 153; i < 157; i++ ) {
		track_switch( ui_tid, i, direction );
		delay( SWITCH_XMIT_DELAY );
	}
	track_switch_off();
	delay( SWITCH_OFF_DELAY );
}

void train_module()
{
	int tid;
	int ptid = MyParentTid();
	int executor_tid;
	int quit = 0;
	int status;
	int i;
	int switch_ui_tid;
	char switch_table[ NUM_SWITCHES ];
	Train_event event;
	Train_reply reply;
	int curtime;
	int last_switch_time = 0;
	
	int executor_ready = 0;
	uchar buffer[COMMAND_BUFFER_SIZE];
	Rbuf command_buffer_var;
	Rbuf* command_buffer = &command_buffer_var;

	status = rbuf_init( command_buffer, buffer, sizeof( Train_command ), COMMAND_BUFFER_SIZE );
	assert( status == 0 );

	status = RegisterAs( TRAIN_MODULE_NAME );
	assert( status == ERR_NONE );

	for( i = 0; i < NUM_SWITCHES; i += 1 ){
		switch_table[ i ] = 'C';
	}

	executor_tid = Create( TRAIN_EXECUTOR_PRIORITY, train_module_executor );

	switch_ui_tid = WhoIs( TRAIN_SWITCH_NAME );
	assert( switch_ui_tid > 0 );

	track_switch_all( switch_ui_tid, 'C' );

	/* Sync for switch update */
	sync_responde( ptid );

	while ( ! quit ) {
		status = Receive( &tid, (char*)&event, sizeof(event) );
		assert( status == sizeof(event) );

		curtime = Time();

		/* early reply */
		switch ( event.event_type ){
		case TRAIN_EXECUTE:
		case TRAIN_CHECK_SWITCH:
			break;
		default:
			reply.result = 0;
			status = Reply( tid, (char*)&reply, sizeof( reply ) );
			assert( status == 0 );
			break;
		}

		/* Switch command timing *//*
		switch( event.event_type ){
		case TRAIN_SWITCH:
		case TRAIN_SWITCH_ALL:
			if( curtime - last_switch_time < SWITCH_ALL_DELAY ){
				Delay( curtime - last_switch_time );
			}
			break;
		}
		*/
		switch ( event.event_type ) {
		case TRAIN_EXECUTE:
			assert( executor_ready == 0 );
			if ( rbuf_empty( command_buffer ) ) {
				/* pending to wait on new taks */
				executor_ready = 1;
			}
			else {
				reply.result = execute_command( command_buffer, switch_table, switch_ui_tid );
				status = Reply( executor_tid, (char*)&reply, sizeof( reply ) );
				assert( status == 0 );
				executor_ready = 0;
			}
			break;
		case TRAIN_CHECK_SWITCH:
			reply.result = switch_table[ SWID_TO_ARRAYID( event.args[ 0 ] ) ];
			break;
		case TRAIN_SET_SPEED:
			buffer_command( command_buffer, TRAIN_EXEC_TR, event.args[0], event.args[1] );
			break;
		case TRAIN_REVERSE:
			buffer_command( command_buffer, TRAIN_EXEC_RV, event.args[0], 0 );
			break;
		case TRAIN_ALL_SENSORS:
			buffer_command( command_buffer, TRAIN_EXEC_SENSOR, 0, 0 );
			break;
		case TRAIN_SWITCH:
			buffer_command( command_buffer, TRAIN_EXEC_SW, event.args[0], event.args[1] );
			buffer_command( command_buffer, TRAIN_EXEC_SWOFF, 0, 0 );
			break;
		case TRAIN_SWITCH_ALL:
			track_switch_all( switch_ui_tid, event.args[0] );
			for( i = 0; i < NUM_SWITCHES; i += 1 ){
				switch_table[ i ] = event.args[0];
			}
			break;
		case TRAIN_MODULE_SUICIDE:
			if ( tid == ptid ) {
				quit = 1;
				break;
			}
			break;
		case TRAIN_PRESSURE_TEST:
			break;
		default:
			// should not get to here
			assert(0);
		}
		
		switch ( event.event_type ) {
		case TRAIN_SET_SPEED:
		case TRAIN_REVERSE:
		case TRAIN_SWITCH:
		// case TRAIN_SWITCH_ALL:
		case TRAIN_ALL_SENSORS:
			if ( executor_ready ){
				reply.result = execute_command( command_buffer, switch_table, switch_ui_tid );
				status = Reply( executor_tid, (char*)&reply, sizeof( reply ) );
				assert( status == 0 );
				executor_ready = 0;
			}
		default:
			break;
		}

		/* Assign switch command timing */
		switch( event.event_type ){
		case TRAIN_REVERSE:
		case TRAIN_SWITCH:
		case TRAIN_SWITCH_ALL:
			last_switch_time = Time();
			break;
		}

		switch ( event.event_type ){
		case TRAIN_CHECK_SWITCH:
			status = Reply( tid, (char*)&reply, sizeof( reply ) );
			assert( status == 0 );
			break;
		default:
			break;
		}
	}

	// tell anything produced by this to exit
	Exit();
}

int train_event( int tid, uint type, int arg0, int arg1 )
{
	Train_event event;
	Train_reply reply;

	event.event_type = type;
	event.args[0] = arg0;
	event.args[1] = arg1;

	int status = Send( tid, (char*)&event, sizeof( event ), (char*)&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );

	return reply.result;
}


int train_check_switch( int tid, int switch_id )
{
	return train_event( tid, TRAIN_CHECK_SWITCH, switch_id, 0 );
}

int train_set_speed( int tid, int train, int speed )
{
	return train_event( tid, TRAIN_SET_SPEED, train, speed );
}

int train_reverse( int tid, int train )
{
	return train_event( tid, TRAIN_REVERSE, train, 0 );
}

int train_switch( int tid, int switch_id, int direction )
{
	return train_event( tid, TRAIN_SWITCH, switch_id, direction );
}

int train_all_sensor( int tid )
{
	return train_event( tid, TRAIN_ALL_SENSORS, 0, 0 );
}

int train_module_suicide( int tid )
{
	return train_event( tid, TRAIN_MODULE_SUICIDE, 0, 0 );
}

int train_pressure_test( int tid )
{
	return train_event( tid, TRAIN_PRESSURE_TEST, 0, 0 );
}

int train_switch_all( int tid, int d )
{
	return train_event( tid, TRAIN_SWITCH_ALL, d, 0 );
}

int train_execute( int tid ){
	return train_event( tid, TRAIN_EXECUTE, 0, 0 );
}
