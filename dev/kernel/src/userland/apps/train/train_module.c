#include <types.h>
#include <err.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/train.h>
#include <user/uart.h>
#include <user/name_server.h>

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
	TRAIN_PRESSURE_TEST
};

struct Train_event_s {
	uint event_type;
	uint args[2];
};

struct Train_reply_s {
	int result;
};

void train_module() {

	int tid;
	int quit = 0;
	int status;
	int i;
	int last_speed = 5;

	char switch_table[22];
	for ( i = 0; i < 22; i++ ) {
		switch_table[i] = 'S';
	}

	// reset all switches
	for ( i = 1; i < 19; i++ ) {
		
	}

	Train_event event;
	Train_reply reply;

	status = RegisterAs( TRAIN_MODULE_NAME );
	assert( status == ERR_NONE );

	while ( !quit ) {
		status = Receive( &tid, (char*)&event, sizeof(event) );
		assert( status == sizeof(event) );

		reply.result = 0;
		status = Reply( tid, (char*)&reply, sizeof( reply ) );
		assert( status == 0 );

		switch (event.event_type) {
		case TRAIN_UPDATE_TIME:
			break;
		case TRAIN_SET_SPEED:
			status = Putc( COM_1, (char)event.args[1] );
			assert( status == ERR_NONE );
			status = Putc( COM_1, (char)event.args[0] );
			assert( status == ERR_NONE );
			last_speed = event.args[1];
			status = Delay( TRAIN_XMIT_DELAY );
			assert( status == ERR_NONE );
			break;
		case TRAIN_REVERSE:
			status = Putc( COM_1, (char)15 );
			assert( status == ERR_NONE );
			status = Putc( COM_1, (char)event.args[0] );
			assert( status == ERR_NONE );
			status = Delay( TRAIN_XMIT_DELAY );
			assert( status == ERR_NONE );
			status = Putc( COM_1, last_speed );
			assert( status == ERR_NONE );
			status = Putc( COM_1, (char)event.args[0] );
			assert( status == ERR_NONE );
			status = Delay( TRAIN_XMIT_DELAY );
			assert( status == ERR_NONE );
			break;
		case TRAIN_SWITCH:
			status = Putc( COM_1, (char)event.args[1] );
			assert( status == ERR_NONE );
			status = Putc( COM_1, (char)event.args[0] );
			assert( status == ERR_NONE );
			status = Delay( TRAIN_XMIT_DELAY );
			assert( status == ERR_NONE );
			status = Putc( COM_1, (char)32 );
			assert( status == ERR_NONE );
			status = Delay( TRAIN_XMIT_DELAY );
			assert( status == ERR_NONE );
			break;
		case TRAIN_CHECK_SWITCH:
			break;
		case TRAIN_LAST_SENSOR:
			// send to SENSOR uart
			break;
		case TRAIN_ALL_SENSORS:
			// send and receive all sensor data
			break;
		case TRAIN_MODULE_SUICIDE:
			if ( tid == MyParentTid() ) {
				quit = 1;
				break;
			}
			// warning message
			break;
		case TRAIN_PRESSURE_TEST:
			for ( i = 0; i < 15; i++ ) {
				status = Putc( COM_1, i );
				assert( status == ERR_NONE );
				status = Putc( COM_1, 22 );
				assert( status == ERR_NONE );
			}
			for ( i = 14; i >= 0; i-- ) {
				status = Putc( COM_1, i );
				assert( status == ERR_NONE );
				status = Putc( COM_1, 22 );
				assert( status == ERR_NONE );
			}
			break;
		default:
			// should not get to here
			// TODO: change to uart
			assert(0);
		}
	}
	
/*
	// wait to sell sensor and clock task to exit
	int i = 0;
	for ( i = 0; i < 2; i++ ) {
		status = Receive( &tid, (char*)&event, sizeof(event) );
		assert( status == sizeof(event) );
		reply.result = -1;
		status = Reply( tid, (char*)&reply, sizeof( reply ) );
		assert( status == 0 );
	}
*/
	
	// tell anything produced by this to exit
	Exit();

}

int train_event( uint type, int arg0, int arg1 ) {
	Train_event event;
	Train_reply reply;

	event.event_type = type;
	event.args[0] = arg0;
	event.args[1] = arg1;

	int module_tid = WhoIs( TRAIN_MODULE_NAME );

	int status = Send( module_tid, (char*)&event, sizeof( event ), (char*)&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );
	
	return reply.result;
}


int train_update_time( int ticks ){
	return train_event( TRAIN_UPDATE_TIME, ticks, 0 );
}

int train_set_speed( int train, int speed ){
	return train_event( TRAIN_SET_SPEED, train, speed );
}

int train_reverse( int train ){
	return train_event( TRAIN_REVERSE, train, 0 );
}

int train_switch( int switch_id, int direction ){
	return train_event( TRAIN_SWITCH, switch_id, direction );
}

int train_check_switch( int switch_id ){
	return train_event( TRAIN_CHECK_SWITCH, switch_id, 0 );
}

int train_last_sensor(){
	return train_event( TRAIN_LAST_SENSOR, 0, 0 );
}

int train_all_sensor(){
	return train_event( TRAIN_ALL_SENSORS, 0, 0 );
}

int train_module_suicide(){
	return train_event( TRAIN_MODULE_SUICIDE, 0, 0 );
}

int train_pressure_test(){
	return train_event( TRAIN_PRESSURE_TEST, 0, 0 );
}

