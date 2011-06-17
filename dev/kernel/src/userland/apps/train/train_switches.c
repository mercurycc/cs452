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

#define TRAIN_SWITCH_NAME "train switch"

#define struct Switch_event_s Switch_event
#define struct Switch_reply_s Switch_reply

enum Switch_event_type {
	SWITCH_UPDATE_ID,
	SWITCH_UPDATE_ALL,
	SWITCH_CHECK_ID
};

struct Switch_event_s {
	unit event_type;
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
	
	char table[22] = "CCCCCCCCCCCCCCCCCCCCCC";

	Switch_event event;
	Switch_reply reply;

	status = RegisterAs( TRAIN_SWITCH_NAME );
	assert( status == ERR_NONE );
	
	Region switch_rect = { 1, 3, 5, 50, 0, 0 };
	Region *switch_region = &switch_rect;
	status = region_init( switch_region );
	assert( status == ERR_NONE );

	while ( !quit ) {
		int status = region_printf( switch_region,
									"Switch Table:\n     1: %c    2: %c    3: %c    4: %c    5: %c    6: %c\n    7: %c    8: %c    9: %c   10: %c   11: %c   12: %c\n   13: %c   14: %c   15: %c   16: %c   17: %c   18: %c\n  153: %c  154: %c  155: %c  156: %c",
									table[0], table[1], table[2], table[3], table[4], table[5],
									table[6], table[7], table[8], table[9], table[10], table[11],
									table[12], table[13], table[14], table[15], table[16], table[17],
									table[18], table[19], table[20], table[21] );
		assert( status == ERR_NONE );
	
		status = Receive( &tid, (char*)&event, sizeof(event) );
		assert( status == sizeof(event) );
		
		switch ( event.evnet_type ){
		case SWITCH_UPDATE_ID:
		case SWITCH_UPDATE_ALL:
		default:
			reply.result = 0;
			status = Reply( tid, (char*)&reply, sizeof(reply) );
			assert( status == 0 );
		}
		
		switch ( event.evnet_type ){
		case SWITCH_UPDATE_ID:
			table[event.id] = event.direction;
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
	return switch_event( TRAIN_UPDATE_ID, id, direction );
}

int switch_update_all( char direction ){
	return switch_event( TRAIN_UPDATE_ALL, 0, direction );
}

int switch_check_id( int id ){
	return switch_event( TRAIN_CHECK_ID, id, 0 );
}

