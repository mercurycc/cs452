#include <types.h>
#include <err.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/clock_server.h>
#include <user/devices/clock.h>
#include <user/uart.h>
#include <user/display.h>
#include <user/time.h>
#include <user/name_server.h>
#include "../inc/sensor_data.h"
#include "../inc/config.h"
#include "../inc/train.h"
#include "../inc/warning.h"
#include "../inc/train_types.h"
#include "../inc/track_node.h"

#define LOCAL_DEBUG
#include <user/dprint.h>

typedef struct Planner_ui_s Planner_ui_request;

enum Planner_ui_request_type {
	PLANNER_UI_NEW_TRAIN,
	PLANNER_UI_NEW_PLAN,
	PLANNER_UI_ARRIVAL
};

struct Planner_ui_s {
	uint type;
	uint train_id;
	union {
		struct {
			int group;
			int id;
			uint deadline;
		} new_plan;
		struct {
			uint deadline;
		} arrival;
	} data;
};

void planner_ui(){
	Planner_ui_request request;
	int available_slot = 1;
	/* UI specific, see UIDesign */
	Region title_reg = { 28, 12, 1, 44 - 28, 1, 0 };
	Region data_reg = { 28, 13, 20 - 13, 78 - 28, 1, 1 };
	Region entry = { 30, 15, 1, 75 - 29, 0, 0 };
	int train_map[ MAX_TRAIN_ID ] = { 0 };
	char mark_name[ 16 ];
	Timespec time;
	int i;
	int tid;
	int status;

	status = RegisterAs( PLANNER_UI_NAME );
	assert( status == REGISTER_AS_SUCCESS );

	region_init( &title_reg );
	region_printf( &title_reg, "Train Schedule\n" );
	region_init( &data_reg );
	region_printf( &data_reg,
		       " #|Dest.|Deadl|  #|Dest.|Deadl|  #|Dest.|Deadl" );
	region_init( &entry );
	region_printf( &entry,
		       "  |     |     |   |     |     |   |     |     " );

	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		status -= sizeof( uint ) * 2;

		switch ( request.type ) {
		case PLANNER_UI_NEW_TRAIN:
			assert( status == 0 );
			break;
		case PLANNER_UI_NEW_PLAN:
			assert( status == sizeof( request.data.new_plan ) );
			break;
		case PLANNER_UI_ARRIVAL:
			assert( status == sizeof( request.data.arrival ) );
			break;
		default:
			assert( 0 );
		}
		
		status = Reply( tid, ( char* )&status, sizeof( status ) );
		assert( status == SYSCALL_SUCCESS );

		switch( request.type ){
		case PLANNER_UI_NEW_TRAIN:
			if (  train_map[ request.train_id ] ) {
				/* re-register */
				
			}
			else {
				train_map[ request.train_id ] = available_slot;
				available_slot += 1;
			}
		default:
			break;
		}
		
		entry.col = train_map[ request.train_id ] * 16 + 14;
		
		switch ( request.type ) {
		case PLANNER_UI_NEW_TRAIN:
			entry.width = 2;
			// dprintf( "entry region c %d r %d h %d w %d m %d b %d", entry.col, entry.row, entry.height, entry.width, entry.margin, entry.boundary );
			region_printf( &entry, "%d", request.train_id);
			break;
		case PLANNER_UI_NEW_PLAN:
			entry.col += 3;
			entry.width = 5;
			track_node_id2name( mark_name, request.data.new_plan.group, request.data.new_plan.id );
			region_printf( &entry, "%s\n", mark_name);
			entry.col += 6;
			entry.width = 5;
			time_tick_to_spec( &time, request.data.new_plan.deadline );
			region_printf( &entry, "%2u:%2u\n", time.minute, time.second );
			break;
		case PLANNER_UI_ARRIVAL:
			entry.col += 9;
			entry.width = 5;
			time_tick_to_spec( &time, request.data.arrival.deadline );
			region_printf( &entry, "%2u:%2u\n", time.minute, time.second );
			break;
		}
		
	}
}

static int planner_ui_request( int tid, Planner_ui_request* request, uint size )
{
	int status;
	int reply;

	size += 2 * sizeof( uint );

	status = Send( tid, ( char* )request, size, ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );

	return ERR_NONE;
}

int planner_ui_new_train( int tid, int train_id ) {
	Planner_ui_request request;

	request.type = PLANNER_UI_NEW_TRAIN;
	request.train_id = train_id;

	return planner_ui_request( tid, &request, 0 );
}

int planner_ui_new_plan( int tid, int train_id, int group, int id, int deadline ){
	Planner_ui_request request;

	request.type = PLANNER_UI_NEW_PLAN;
	request.train_id = train_id;
	request.data.new_plan.group = group;
	request.data.new_plan.id = id;
	request.data.new_plan.deadline = deadline;

	return planner_ui_request( tid, &request, sizeof( request.data.new_plan ) );
}

int planner_ui_arrival( int tid, int train_id, int deadline ){
	Planner_ui_request request;

	request.type = PLANNER_UI_NEW_PLAN;
	request.train_id = train_id;
	request.data.arrival.deadline = deadline;

	return planner_ui_request( tid, &request, sizeof( request.data.arrival ) );
}
