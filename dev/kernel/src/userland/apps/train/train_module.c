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
#include "inc/warning.h"

#define SWITCH_XMIT_DELAY 16
#define SWITCH_OFF_DELAY 20
#define TRAIN_XMIT_DELAY 20
#define SWITCH_ALL_DELAY 180

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

static inline void setspeed( int train, int speed ){
	int status = Putc( COM_1, (char)speed );
	assert( status == ERR_NONE );
	status = Putc( COM_1, (char)train );
	assert( status == ERR_NONE );
	status = Delay( TRAIN_XMIT_DELAY );
	assert( status == ERR_NONE );
}

static inline void reverse( int train ){
	int status = Putc( COM_1, (char)15 );
	assert( status == ERR_NONE );
	status = Putc( COM_1, (char)train );
	assert( status == ERR_NONE );
	status = Delay( TRAIN_XMIT_DELAY );
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
	status = Delay( SWITCH_XMIT_DELAY );
	assert( status == ERR_NONE );
}

static inline void track_switch_off() {
	int status = Delay( SWITCH_OFF_DELAY );
	assert( status == ERR_NONE );
	status = Putc( COM_1, (char)32 );
	assert( status == ERR_NONE );
	status = Delay( SWITCH_OFF_DELAY );
	assert( status == ERR_NONE );
}

static inline void track_switch_all( int ui_tid, char direction ){
	int i;
	// reset all switches
	for ( i = 1; i < 19; i++ ) {
		track_switch( ui_tid, i, direction );
	}
	for ( i = 153; i < 157; i++ ) {
		track_switch( ui_tid, i, direction );
	}
	track_switch_off();
}

static inline void delay( int ticks ) {
	int status = Delay( ticks );
	assert( status == ERR_NONE );
}


void train_module()
{
	int tid;
	int ptid = MyParentTid();
	int quit = 0;
	int status;
	int i;
	int switch_ui_tid;
	char direction;
	char switch_table[ NUM_SWITCHES ];
	Train_event event;
	Train_reply reply;

	status = RegisterAs( TRAIN_MODULE_NAME );
	assert( status == ERR_NONE );

	for( i = 0; i < NUM_SWITCHES; i += 1 ){
		switch_table[ i ] = 'C';
	}

	switch_ui_tid = WhoIs( TRAIN_SWITCH_NAME );
	assert( switch_ui_tid > 0 );

	track_switch_all( switch_ui_tid, 'C' );

	/* Sync for switch update */
	sync_responde( ptid );

	while ( ! quit ) {
		status = Receive( &tid, (char*)&event, sizeof(event) );
		assert( status == sizeof(event) );

		// early reply
		switch ( event.event_type ){
		case TRAIN_CHECK_SWITCH:
			break;
		default:
			reply.result = 0;
			status = Reply( tid, (char*)&reply, sizeof( reply ) );
			assert( status == 0 );
			break;
		}
		
		switch (event.event_type) {
		case TRAIN_CHECK_SWITCH:
			reply.result = switch_table[ SWID_TO_ARRAYID( event.args[ 0 ] ) ];
			break;
		case TRAIN_SET_SPEED:
			setspeed( event.args[0], event.args[1] );
			break;
		case TRAIN_REVERSE:
			reverse( event.args[0] );
			break;
		case TRAIN_ALL_SENSORS:
			status = Putc( COM_1, (char)133 );
			assert( status == ERR_NONE );
			break;
		case TRAIN_SWITCH:
			i = SWID_TO_ARRAYID( event.args[0] );
			switch_table[ i ] = event.args[ 1 ];
			track_switch( switch_ui_tid, event.args[0], event.args[1] );
			track_switch_off();
			break;
		case TRAIN_SWITCH_ALL:
			track_switch_all( switch_ui_tid, event.args[0] );
			for( i = 0; i < NUM_SWITCHES; i += 1 ){
				switch_table[ i ] = event.args[0];
			}
			delay( SWITCH_ALL_DELAY );
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
