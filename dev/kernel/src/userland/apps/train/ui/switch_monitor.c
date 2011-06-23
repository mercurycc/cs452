#include <types.h>
#include <err.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/clock_server.h>
#include <user/name_server.h>
#include <user/devices/clock.h>
#include <user/uart.h>
#include <user/display.h>
#include <user/lib/sync.h>
#include "../inc/train.h"

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

void switches_ui()
{
	int quit = 0;
	int status;
	int tid;
	int i;
	int temp;
	char table[ NUM_SWITCHES ];
	char modified[ NUM_SWITCHES ];
	Switch_event event;
	Switch_reply reply;
	Region switch_title = { 2, 12, 1, 18 - 2, 1, 0 };
	Region switch_reg = { 2, 13, 20 - 13, 27 - 2, 0, 1 };
	Region switch_single = { 0, 0, 1, 1, 0, 0 };

	status = RegisterAs( TRAIN_SWITCH_NAME );
	assert( status == ERR_NONE );

	status = region_init( &switch_title );
	assert( status == ERR_NONE );
	status = region_init( &switch_reg );
	assert( status == ERR_NONE );

	region_printf( &switch_title, "Switch Monitor" );

	region_printf( &switch_reg,
		       " [0]1:  2:  3:  4:  5:\n"
		       "    6:  7:  8:  9:\n"
		       " [1]0:  1:  2:  3:  4:\n"
		       "    5:  6:  7:  8:\n"
		       "[15]3:  4:  5:  6:\n" );

	for ( i = 0; i < 22; i++ ){
		modified[ i ] = 0;
	}

	while ( !quit ) {
		status = Receive( &tid, (char*)&event, sizeof(event) );
		assert( status == sizeof(event) );

		reply.result = 0;

		switch ( event.event_type ){
		case SWITCH_UPDATE_ID:
			event.id = SWID_TO_ARRAYID( event.id );
			table[ event.id ] = event.direction;
			modified[ event.id ] = 1;
			assert( ( event.id >= 0 ) && ( event.id < 22 ) );
			break;
		case SWITCH_UPDATE_ALL:
			for ( i = 0; i < 22; i++ ){
				table[ i ] = event.direction;
				modified[ i ] = 1;
			}
			break;
		default:
			assert(0);
		}

		status = Reply( tid, (char*)&reply, sizeof(reply) );
		assert( status == 0 );

		/* Update UI */
		/* Constants are UI specific, see UIDesign */
		for( i = 0; i < NUM_SWITCHES; i += 1 ){
			if( modified[ i ] ){
				if( i < 9 ){
					temp = i;
				} else if( i < 18 ){
					temp = i + 1;   /* To compensate the empty space on row 15 */
				} else {
					temp = i + 2;   /* To compensate the empty space on row 15 and 16 */
				}

				switch_single.row = 14 + temp / 5;
				switch_single.col = 9 + ( temp % 5 ) * 4;

				region_printf( &switch_single, "%c", table[ i ] );

				modified[ i ] = 0;
			}
		}

	}

	Exit();
}


static int switch_ui_event( int tid, uint type, int id, char direction )
{
	Switch_event event;
	Switch_reply reply;

	event.event_type = type;
	event.id = id;
	event.direction = direction;

	int status = Send( tid, (char*)&event, sizeof( event ), (char*)&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );
	
	return reply.result;
}

int switch_ui_update_id( int tid, int id, char direction ){
	return switch_ui_event( tid, SWITCH_UPDATE_ID, id, direction );
}

int switch_ui_update_all( int tid, char direction ){
	return switch_ui_event( tid, SWITCH_UPDATE_ALL, 0, direction );
}
