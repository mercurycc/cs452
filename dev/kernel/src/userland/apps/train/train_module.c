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

void train_delay(){
	
}

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
	int status = Putc( COM_1, (char)32 );
	assert( status == ERR_NONE );
	status = Delay( SWITCH_XMIT_DELAY );
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


void train_module() {

	int tid;
	int quit = 0;
	int status;
	int i;
	int last_speed = 5;

	char switch_table[22];
	for ( i = 0; i < 22; i++ ) {
		switch_table[i] = 'C';
	}

	switch_all( (char)34 );

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
			last_speed = event.args[1];
			tr( event.args[0], event.args[1] );
			break;
		case TRAIN_REVERSE:
			rv( event.args[0] );
			tr( event.args[0], last_speed );
			break;
		case TRAIN_SWITCH:
			sw( event.args[0], event.args[1] );
			sw_off();
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
			for ( i = 0; i < 10; i++ ) {
				tr( 22, i+4 );
				rv( 22 );
			}
			tr( 22, 0 );
			break;
		case TRAIN_SWITCH_ALL:
			switch_all( event.args[0] );
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

int train_switch_all( int d ){
	return train_event( TRAIN_SWITCH_ALL, d, 0 );
}
