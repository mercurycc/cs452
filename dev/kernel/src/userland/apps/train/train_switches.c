#include <types.h>
#include <err.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/clock_server.h>
#include <user/name_server.h>
#include <user/devices/clock.h>
#include <user/train.h>
#include <user/uart.h>
#include <user/display.h>
#include <user/lib/sync.h>

#define TRAIN_SWITCH_NAME "train switch"

typedef struct Switch_event_s Switch_event;
typedef struct Switch_reply_s Switch_reply;

enum Switch_event_type {
	SWITCH_UPDATE_ID,
	SWITCH_UPDATE_ALL,
	SWITCH_CHECK_ID
};

struct Switch_event_s {
	uint event_type;
	int id;
	char direction;
};

struct Switch_reply_s {
	int result;
};

void train_switches() {
	int quit = 0;
	int status;
	int tid;
	int i;
	
	char table[22];
	for ( i = 0; i < 22; i++ ){
		table[i] = 'C';
	}

	Switch_event event;
	Switch_reply reply;

	status = RegisterAs( TRAIN_SWITCH_NAME );
	assert( status == ERR_NONE );
	
	Region switch_rect = { 5, 1, 14, 54, 2, 1 };
	Region *switch_region = &switch_rect;
	status = region_init( switch_region );
	assert( status == ERR_NONE );
	status = region_clear( switch_region );
	assert( status == ERR_NONE );

	while ( !quit ) {
		int status = region_printf( switch_region,
			"Switch Table:\n\n   1: %c    2: %c    3: %c    4: %c    5: %c    6: %c\n\n   7: %c    8: %c    9: %c   10: %c   11: %c   12: %c\n\n  13: %c   14: %c   15: %c   16: %c   17: %c   18: %c\n\n 153: %c  154: %c  155: %c  156: %c",
			table[0], table[1], table[2], table[3], table[4], table[5],
			table[6], table[7], table[8], table[9], table[10], table[11],
			table[12], table[13], table[14], table[15], table[16], table[17],
			table[18], table[19], table[20], table[21] );
		assert( status == ERR_NONE );

		status = Receive( &tid, (char*)&event, sizeof(event) );
		assert( status == sizeof(event) );
		
		switch ( event.event_type ){
		case SWITCH_CHECK_ID:
			break;
		default:
			reply.result = 0;
			status = Reply( tid, (char*)&reply, sizeof(reply) );
			assert( status == 0 );
		}
		
		switch ( event.event_type ){
		case SWITCH_UPDATE_ID:
			table[event.id] = event.direction;
			assert( (event.id >= 0 )&&(event.id < 22) );
			break;
		case SWITCH_UPDATE_ALL:
			for ( i = 0; i < 22; i++ ){
				table[i] = event.direction;
			}
			break;
		case SWITCH_CHECK_ID:
			reply.result = table[event.id];
			status = Reply( tid, (char*)&reply, sizeof(reply) );
			assert( status == 0 );
			continue;
		default:
			assert(0);
		}

	}

	Exit();
}


int switch_event( uint type, int id, char direction ) {
	Switch_event event;
	Switch_reply reply;

	event.event_type = type;
	event.id = id;
	event.direction = direction;

	int module_tid = WhoIs( TRAIN_SWITCH_NAME );

	int status = Send( module_tid, (char*)&event, sizeof( event ), (char*)&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );
	
	return reply.result;
}

int switch_update_id( int id, char direction ){
	return switch_event( SWITCH_UPDATE_ID, id, direction );
}

int switch_update_all( char direction ){
	return switch_event( SWITCH_UPDATE_ALL, 0, direction );
}

int switch_check_id( int id ){
	return switch_event( SWITCH_CHECK_ID, id, 0 );
}

