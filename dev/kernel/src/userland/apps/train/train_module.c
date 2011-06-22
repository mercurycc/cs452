#include <types.h>
#include <err.h>
#include <user/apps_entry.h>
#include <user/assert.h>
#include <user/display.h>
#include <user/syscall.h>
#include <user/uart.h>
#include <user/name_server.h>
#include <user/lib/sync.h>
#include "inc/config.h"
#include "inc/train.h"

#define SWITCH_XMIT_DELAY 15
#define SWITCH_OFF_DELAY 20
#define TRAIN_XMIT_DELAY 20

typedef struct Train_event_s Train_event;
typedef struct Train_reply_s Train_reply;

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
	TRAIN_SWITCH_ALL
};

struct Train_event_s {
	uint event_type;
	uint args[2];
};

struct Train_reply_s {
	int result;
};

static inline void tr( int train, int speed ){
	int status = Putc( COM_1, (char)speed );
	assert( status == ERR_NONE );
	status = Putc( COM_1, (char)train );
	assert( status == ERR_NONE );
	status = Delay( TRAIN_XMIT_DELAY );
	assert( status == ERR_NONE );
}

static inline void rv( int train ){
	int status = Putc( COM_1, (char)15 );
	assert( status == ERR_NONE );
	status = Putc( COM_1, (char)train );
	assert( status == ERR_NONE );
	status = Delay( TRAIN_XMIT_DELAY );
	assert( status == ERR_NONE );
}

static inline void sw( int n, char d ){
	int status = Putc( COM_1, d );
	assert( status == ERR_NONE );
	status = Putc( COM_1, (char)n );
	assert( status == ERR_NONE );
	status = Delay( SWITCH_XMIT_DELAY );
	assert( status == ERR_NONE );
}

static inline void sw_off() {
	int status = Delay( SWITCH_OFF_DELAY );
	assert( status == ERR_NONE );
	status = Putc( COM_1, (char)32 );
	assert( status == ERR_NONE );
	status = Delay( SWITCH_OFF_DELAY );
	assert( status == ERR_NONE );
}

static inline void switch_all( char d ){
	int i;
	// reset all switches
	for ( i = 1; i < 19; i++ ) {
		sw( i, d );
	}
	for ( i = 153; i < 157; i++ ) {
		sw( i, d );
	}
	sw_off();
}

static inline void delay( int ticks ) {
	int status = Delay( ticks );
	assert( status == ERR_NONE );

}



void train_module() {

	int tid;
	int ptid = MyParentTid();
	int quit = 0;
	int status;
	int i;
	int last_speed = 5;
	char direction;
	int last_sensor = 0;
	char switch_table[22];
	Train_event event;
	Train_reply reply;

	status = RegisterAs( TRAIN_MODULE_NAME );
	assert( status == ERR_NONE );

	for ( i = 0; i < 22; i++ ) {
		switch_table[i] = 'C';
	}

	/* Sync for switch update */
	sync_responde( ptid );

	while ( !quit ) {
		status = Receive( &tid, (char*)&event, sizeof(event) );
		assert( status == sizeof(event) );

		// early reply
		switch ( event.event_type ){
		case TRAIN_CHECK_SWITCH:
		case TRAIN_LAST_SENSOR:
		case TRAIN_ALL_SENSORS:
			break;
		default:
			reply.result = 0;
			status = Reply( tid, (char*)&reply, sizeof( reply ) );
			assert( status == 0 );
			break;
		}

		switch (event.event_type) {
		case TRAIN_UPDATE_TIME:
			break;
		case TRAIN_SET_SPEED:
			last_speed = event.args[1];
			tr( event.args[0], event.args[1] );
			break;
		case TRAIN_REVERSE:
			rv( event.args[0] );
			tr( event.args[0], last_speed );
			break;
		case TRAIN_SWITCH:
			direction = (event.args[1] % 2) * 'S' + ((event.args[1]+1) % 2) * 'C';
			if ( event.args[0] < 19 ) {
				i = event.args[0]-1;
			} else {
				i = event.args[0]-135;
			}
			status = switch_update_id( i, direction );
			assert( status == 0 );
			sw( event.args[0], event.args[1] );
			sw_off();
			break;
		case TRAIN_CHECK_SWITCH:
			if ( event.args[0] < 19 ) {
				i = event.args[0] - 1;
			} else {
				i = event.args[0] - 135;
			}
			reply.result = switch_check_id( i );
			status = Reply( tid, (char*)&reply, sizeof( reply ) );
			assert( status == 0 );
			break;
		case TRAIN_SWITCH_ALL:
			direction = (event.args[0] % 2) * 'S' + ((event.args[0]+1) % 2) * 'C';
			switch_all( event.args[0] );
			delay( 180 );
			break;
		case TRAIN_LAST_SENSOR:
			reply.result = last_sensor;
			status = Reply( tid, (char*)&reply, sizeof( reply ) );
			assert( status == 0 );
			break;
		case TRAIN_ALL_SENSORS:
			status = Putc( COM_1, (char)133 );
			assert( status == ERR_NONE );
			reply.result = 0;
			status = Reply( tid, (char*)&reply, sizeof( reply ) );
			assert( status == 0 );
			// last_sensor = sensor_last( sensor_tid );
			break;
		case TRAIN_MODULE_SUICIDE:
			if ( tid == ptid ) {
				quit = 1;
				break;
			}
			// warning message
			break;
		case TRAIN_PRESSURE_TEST:
			for ( i = 0; i < 10; i++ ) {
				tr( 22, i+4 );
				rv( 22 );
			}
			tr( 22, 0 );
			break;
		default:
			// should not get to here
			assert(0);
		}

	}

	// tell anything produced by this to exit
	Exit();
}

int train_event( int tid, uint type, int arg0, int arg1 ) {
	Train_event event;
	Train_reply reply;

	event.event_type = type;
	event.args[0] = arg0;
	event.args[1] = arg1;

	int status = Send( tid, (char*)&event, sizeof( event ), (char*)&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );
	
	return reply.result;
}


int train_update_time( int tid, int ticks )
{
	return train_event( tid, TRAIN_UPDATE_TIME, ticks, 0 );
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

int train_check_switch( int tid, int switch_id )
{
	return train_event( tid, TRAIN_CHECK_SWITCH, switch_id, 0 );
}

int train_last_sensor( int tid )
{
	return train_event( tid, TRAIN_LAST_SENSOR, 0, 0 );
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
