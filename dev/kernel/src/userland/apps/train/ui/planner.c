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

typedef struct Planner_ui_s Planner_ui_request;

enum Planner_ui_request_type {
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
			uint pred_time;
		} new_plan;
		struct {
			uint actual_time;
		} arrival;
	} data;
};

void planner_ui(){
	Planner_ui_request request;
	int available_slot = 1;
	/* UI specific, see UIDesign */
	Region title_reg = { 28, 12, 1, 44 - 28, 1, 0 };
	Region data_reg = { 28, 13, 20 - 13, 78 - 28, 1, 1 };
	Region entry;
	int train_map[ MAX_NUM_TRAINS ] = { 0 };
	char mark_name[ 16 ];
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

	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		status -= sizeof( uint ) * 2;

		switch ( request.type ) {
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
	}
}



