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
#include <lib/rbuf.h>
#include "../inc/sensor_data.h"
#include "../inc/config.h"
#include "../inc/train.h"
#include "../inc/warning.h"
#include "../inc/train_types.h"
#include "../inc/track_node.h"

#define LOCAL_DEBUG
#include <user/dprint.h>

enum Sched_ui_request_type {
	SCHED_UI_NEW_PLAN,
	SCHED_UI_ASSIGN,
	SCHED_UI_COMPLETE
};

typedef struct Sched_ui_entry_s {
	int ticket;
	int train_id;
	int dest_group;
	int dest_id;
	int dest_dist;
	int src_group;
	int src_id;
	int src_dist;
} Sched_ui_entry;

typedef struct Sched_ui_request_s {
	uint type;
	int ticket;
	union {
		struct {
			int dest_group;
			int dest_id;
			int dest_dist;
			int src_group;
			int src_id;
			int src_dist;
		} new_plan;
		struct {
			int train_id;
		} assign;
	} data;
} Sched_ui_request;

void sched_ui()
{
	Sched_ui_request request;
	/* UI specific, see UIDesign */
	Region title_reg = { 47, 12, 1, 10, 1, 0 };
	Region data_reg = { 47, 13, 20 - 13, 78 - 47, 1, 1 };
	Region entry_reg = { 49, 15, 4, 76 - 48, 0, 0 };
	Region src_reg = { 55, 0, 1, 5, 0, 0 };
	Region dest_reg = { 66, 0, 1, 5, 0, 0 };
	char dest_name[ 7 ];
	char src_name[ 7 ];
	Sched_ui_entry entry;
	Sched_ui_entry assigned_buf[ SCHEDULER_BUFFER ];
	Rbuf assigned_body;
	Rbuf* assigned = &assigned_body;
	Sched_ui_entry plans_buf[ SCHEDULER_BUFFER ];
	Rbuf plans_body;
	Rbuf* plans = &plans_body;
	int temp;
	int tid;
	int status;

	status = RegisterAs( SCHED_UI_NAME );
	assert( status == REGISTER_AS_SUCCESS );

	rbuf_init( assigned, ( uchar* )assigned_buf, sizeof( Sched_ui_entry ), sizeof( Sched_ui_entry ) * SCHEDULER_BUFFER );
	rbuf_init( plans, ( uchar* )plans_buf, sizeof( Sched_ui_entry ), sizeof( Sched_ui_entry ) * SCHEDULER_BUFFER );

	region_init( &title_reg );
	region_printf( &title_reg, "Schedule\n" );
	region_init( &data_reg );
	region_printf( &data_reg,
		       " #|By| Src.|Dist|Dest.|Dist" );
	region_init( &entry_reg );
	region_printf( &entry_reg,
		       "  |  |     |    |     |\n"
		       "  |  |     |    |     |\n"
		       "  |  |     |    |     |\n"
		       "  |  |     |    |     |\n" );

	entry_reg.height = 1;

	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		status -= sizeof( int ) * 2;

		switch ( request.type ) {
		case SCHED_UI_NEW_PLAN:
			assert( status == sizeof( request.data.new_plan ) );
			break;
		case SCHED_UI_ASSIGN:
			assert( status == sizeof( request.data.assign ) );
			break;
		case SCHED_UI_COMPLETE:
			assert( status == 0 );
			break;
		default:
			assert( 0 );
		}
		
		status = Reply( tid, ( char* )&status, sizeof( status ) );
		assert( status == SYSCALL_SUCCESS );

		switch ( request.type ) {
		case SCHED_UI_NEW_PLAN:
			entry.ticket = request.ticket;
			entry.train_id = 0;
			entry.dest_group = request.data.new_plan.dest_group;
			entry.dest_id = request.data.new_plan.dest_id;
			entry.dest_dist = request.data.new_plan.dest_dist;
			entry.src_group = request.data.new_plan.src_group;
			entry.src_id = request.data.new_plan.src_id;
			entry.src_dist = request.data.new_plan.src_dist;

			rbuf_put_front( plans, ( uchar* )&entry );
			break;
		case SCHED_UI_ASSIGN:
			if( !rbuf_empty( plans ) ){
				temp = 0;
				while( 1 ){
					rbuf_get( plans, ( uchar* )&entry );
					if( ! temp ){
						temp = entry.ticket;
					} else if( entry.ticket == temp ){
						rbuf_put( plans, ( uchar* )&entry );
						break;
					}
					if( entry.ticket == request.ticket ){
						entry.train_id = request.data.assign.train_id;
						rbuf_put_front( assigned, ( uchar* )&entry );
						break;
					} else {
						rbuf_put( plans, ( uchar* )&entry );
					}
				}
			}
			break;
		case SCHED_UI_COMPLETE:
			if( !rbuf_empty( plans ) ){
				temp = 0;
				while( 1 ){
					rbuf_get( plans, ( uchar* )&entry );
					if( ! temp ){
						temp = entry.ticket;
					} else if( entry.ticket == temp ){
						rbuf_put( plans, ( uchar* )&entry );
						break;
					}
					if( entry.ticket == request.ticket ){
						break;
					} else {
						rbuf_put( plans, ( uchar* )&entry );
					}
				}
			}
			if( !rbuf_empty( assigned ) ){
				temp = 0;
				while( 1 ){
					rbuf_get( assigned, ( uchar* )&entry );
					if( ! temp ){
						temp = entry.ticket;
					} else if( entry.ticket == temp ){
						rbuf_put( assigned, ( uchar* )&entry );
						break;
					}
					if( entry.ticket == request.ticket ){
						break;
					} else {
						rbuf_put( assigned, ( uchar* )&entry );
					}
				}
			}
			break;
		}

		entry_reg.row = 15;

		if( !rbuf_empty( assigned ) ){
			temp = 0;
			while( entry_reg.row < 19 ){
				rbuf_get( assigned, ( uchar* )&entry );
				rbuf_put( assigned, ( uchar* )&entry );
				if( ! temp ){
					temp = entry.ticket;
				} else if( entry.ticket == temp ){
					break;
				}
				track_node_id2name( src_name, entry.src_group, entry.src_id );
				track_node_id2name( dest_name, entry.dest_group, entry.dest_id );
				
				region_printf( &entry_reg, "%2d|%2d|     |%4d|     |%4d",
					       entry.ticket, entry.train_id, entry.src_dist, entry.dest_dist );

				src_reg.row = entry_reg.row;
				dest_reg.row = entry_reg.row;

				region_printf( &src_reg, "%s", src_name );
				region_printf( &dest_reg, "%s", dest_name );

				entry_reg.row += 1;
			}
		}

		if( !rbuf_empty( plans ) ){
			temp = 0;
			while( entry_reg.row < 19 ){
				rbuf_get( plans, ( uchar* )&entry );
				rbuf_put( plans, ( uchar* )&entry );
				if( ! temp ){
					temp = entry.ticket;
				} else if( entry.ticket == temp ){
					break;
				}
				track_node_id2name( src_name, entry.src_group, entry.src_id );
				track_node_id2name( dest_name, entry.dest_group, entry.dest_id );
				region_printf( &entry_reg, "%2d|  |     |%4d|     |%4d",
					       entry.ticket, entry.src_dist, entry.dest_dist );

				src_reg.row = entry_reg.row;
				dest_reg.row = entry_reg.row;

				region_printf( &src_reg, "%s", src_name );
				region_printf( &dest_reg, "%s", dest_name );
				
				entry_reg.row += 1;
			}
		}

		while( entry_reg.row < 19 ){
			region_printf( &entry_reg, "  |  |     |    |     |\n" );
			entry_reg.row += 1;
		}
			
	}
}

static int sched_ui_request( int tid, Sched_ui_request* request, uint size )
{
	int status;
	int reply;

	size += 2 * sizeof( uint );

	status = Send( tid, ( char* )request, size, ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );

	return ERR_NONE;
}

int sched_ui_new_plan( int tid, int ticket, int dest_group, int dest_id, int dest_dist, int src_group, int src_id, int src_dist )
{
	Sched_ui_request request;

	request.type = SCHED_UI_NEW_PLAN;
	request.ticket = ticket;
	request.data.new_plan.dest_group = dest_group;
	request.data.new_plan.dest_id = dest_id;
	request.data.new_plan.dest_dist = dest_dist;
	request.data.new_plan.src_group = src_group;
	request.data.new_plan.src_id = src_id;
	request.data.new_plan.src_dist = src_dist;

	return sched_ui_request( tid, &request, sizeof( request.data.new_plan ) );
}

int sched_ui_assign( int tid, int ticket, int train_id )
{
	Sched_ui_request request;

	request.type = SCHED_UI_ASSIGN;
	request.ticket = ticket;
	request.data.assign.train_id = train_id;

	return sched_ui_request( tid, &request, sizeof( request.data.assign ) );	
}

int sched_ui_complete( int tid, int ticket )
{
	Sched_ui_request request;

	request.type = SCHED_UI_COMPLETE;
	request.ticket = ticket;

	return sched_ui_request( tid, &request, 0 );
}
