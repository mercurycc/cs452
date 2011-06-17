#include <types.h>
#include <err.h>
#include <user/apps_entry.h>
#include <user/assert.h>
#include <user/display.h>
#include <user/syscall.h>
#include <user/train.h>
#include <user/uart.h>
#include <user/name_server.h>
#include <user/lib/sync.h>

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

static inline void print_switch_table( Region* r, char* table ){
	int status = region_printf( r, "Switch Table:\n     1: %c    2: %c    3: %c    4: %c    5: %c    6: %c\n    7: %c    8: %c    9: %c   10: %c   11: %c   12: %c\n   13: %c   14: %c   15: %c   16: %c   17: %c   18: %c\n  153: %c  154: %c  155: %c  156: %c", table[0], table[1], table[2], table[3], table[4], table[5], table[6], table[7], table[8], table[9], table[10], table[11], table[12], table[13], table[14], table[15], table[16], table[17], table[18], table[19], table[20], table[21] );
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
	int clock_tid;
	int sensor_tid;
	
	char switch_table[22];
	
	for ( i = 0; i < 22; i++ ) {
		switch_table[i] = 'C';
	}


	Region switch_rect = { 1, 3, 5, 50, 0, 0 };
	Region *switch_region = &switch_rect;
	status = region_init( switch_region );
	assert( status == ERR_NONE );
	

	switch_all( (char)34 );

	Train_event event;
	Train_reply reply;

	status = RegisterAs( TRAIN_MODULE_NAME );
	assert( status == ERR_NONE );

	clock_tid = Create( TRAIN_CLOCK_PRIORITY, train_clock );
	assert( clock_tid > 0 );

	print_switch_table( switch_region, switch_table );


	sensor_tid = Create( TRAIN_SENSOR_PRIORITY, train_sensor );
	assert( sensor_tid > 0 );

	sync_responde( ptid );

	while ( !quit ) {
		status = Receive( &tid, (char*)&event, sizeof(event) );
		assert( status == sizeof(event) );

		// early reply
		switch ( event.event_type ){
		case TRAIN_CHECK_SWITCH:
		case TRAIN_LAST_SENSOR:
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
				switch_table[event.args[0]-1] = direction;
			}
			else {
				switch_table[event.args[0]-135] = direction;
			}
			print_switch_table( switch_region, switch_table );
			sw( event.args[0], event.args[1] );
			sw_off();
			break;
		case TRAIN_CHECK_SWITCH:
			if ( event.args[0] < 19 ) {
				i = event.args[0] - 1;
			}
			else {
				i = event.args[0] - 135;
			}
			reply.result = switch_table[i];
			status = Reply( tid, (char*)&reply, sizeof( reply ) );
			assert( status == 0 );
			break;
		case TRAIN_LAST_SENSOR:
			reply.result = 0;
			status = Reply( tid, (char*)&reply, sizeof( reply ) );
			assert( status == 0 );
			break;
		case TRAIN_ALL_SENSORS:
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
		case TRAIN_SWITCH_ALL:
			direction = (event.args[0] % 2) * 'S' + ((event.args[0]+1) % 2) * 'C';
			for ( i = 0; i < 22; i++ ) {
				switch_table[i] = direction;
			}
			print_switch_table( switch_region, switch_table );
			switch_all( event.args[0] );
			delay( 180 );
			break;
		default:
			// should not get to here
			assert(0);
		}

	}

	Kill( clock_tid );
	Kill( sensor_tid );
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
