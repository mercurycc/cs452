#include <types.h>
#include <err.h>
#include <lib/str.h>
#include <user/syscall.h>
#include <user/assert.h>
#include <user/time.h>
#include <user/name_server.h>
#include <config.h>
#include <lib/rbuf.h>
#include "inc/train.h"
#include "inc/config.h"
#include "inc/sensor_data.h"
#include "inc/track_data.h"
#include "inc/track_node.h"
#include "inc/train_location.h"
#include "inc/warning.h"
#include "inc/train_types.h"

enum Train_planner_request_type {
	PLANNER_INIT,
	PLANNER_PATH_PLAN,
	PLANNER_WAKEUP
};

typedef struct Planner_request_s {
	uint type;
	track_node* dst;
	uint dist_pass;           /* Distance needed to pass dst */
} Planner_request;

static inline void train_planner_update_cost( uint* cost, int* parent, uint new_cost, int index, int parent_index )
{
	if( cost[ index ] > new_cost ){
		cost[ index ] = new_cost;
		parent[ index ] = parent_index;
	}
}

#define DST_DIRECT( current_node, dst )      ( current_node == dst )
#define DST_REVERSE( current_node, dst )     ( current_node == dst->reverse->edge[ DIR_AHEAD ].dest->reverse )
#define DST_REVERSE_BR( current_node, dst )  ( dst->reverse->type == NODE_BRANCH && current_node == dst->reverse->edge[ DIR_CURVED ].dest->reverse )

static inline int train_planner_is_dst( const track_node* current, const track_node* dst )
{
	return  DST_DIRECT( current, dst ) || DST_REVERSE( current, dst ) || DST_REVERSE_BR( current, dst );
}

static int train_planner_plan( const track_node* dst, int dist_pass, volatile const Train_data* train, Rbuf* path, uint* path_length )
{
	uint cost[ TRACK_NUM_NODES ];                /* ~0 for infinity */
	int parent[ TRACK_NUM_NODES ];               /* -1 for no parent */
	int mark[ TRACK_NUM_NODES ];
	uint min_cost;
	int min_index;
	int temp;
	int i;
	const track_node* current_node;
	const track_node* next_node;
	const track_node* track_graph = train->track_graph;
	Region path_display = { 28, 13, 20 - 13, 78 - 28, 1, 1 };
	Train_path path_node;
	char name[ 6 ];
	int status;

	track_node_id2name( name, dst->group, dst->id );
	region_printf( &path_display, "Request to dst %s, dist %d for %d\n", name, dist_pass, train->id );

	Delay( 100 );

	/* Initialize cost */
	for( i = 0; i < TRACK_NUM_NODES; i += 1 ){
		cost[ i ] = ~0;
		mark[ i ] = 0;
	}

	/* Find true destination */
	if( dist_pass < 0 ){
		dist_pass = -dist_pass;
		dst = dst->reverse;
	}
	
	while( dst->type != NODE_EXIT && dist_pass > dst->edge[ DIR_AHEAD ].dist ){
		switch( dst->type ){
		case NODE_SENSOR:
		case NODE_MERGE:
		case NODE_ENTER:
			break;
		case NODE_BRANCH:
		case NODE_EXIT:
			WAR_NOTICE( "Brancher or exit cannot be destination\n" );
			return 0;
		case NODE_NONE:
		default:
			assert( 0 );
		}
		dist_pass -= dst->edge[ DIR_AHEAD ].dist;
		dst = dst->edge[ DIR_AHEAD ].dest;
	}

	if( dst->type == NODE_EXIT ){
		dist_pass = 0;
	}
	
	/* Assign initial cost */
	cost[ train->check_point->reverse->index ] = train->distance;
	parent[ train->check_point->reverse->index ] = -1;
	cost[ train->next_check_point->index ] = train->remaining_distance;
	parent[ train->check_point->reverse->index ] = -1;

	/* Find path */
	while( 1 ){
		min_cost = ~0;
		min_index = 0;
		
		/* Find minimum cost */
		/* TODO: use heap */
		for( i = 0; i < TRACK_NUM_NODES; i += 1 ){
			if( ( ! mark[ i ] ) && cost[ i ] < min_cost ){
				min_index = i;
				min_cost = cost[ i ];
			}
		}

		mark[ min_index ] = 1;

		current_node = track_graph + min_index;
		if( train_planner_is_dst( current_node, dst ) ){
			track_node_id2name( name, current_node->group, current_node->id );
			region_printf( &path_display, "Got to the END: %s\n", name );
			Delay( 100 );
			break;
		}

		track_node_id2name( name, current_node->group, current_node->id );
		region_printf( &path_display, "Current min: %s\n", name );
		// Delay( 100 );

		/* Update neighbor cost */
		/* Reverse */
		next_node = current_node->reverse;
		if( ! mark[ next_node->index ] ){
			/* TODO: this 100 is not correct.  Should be around 2 stop distance at stop speed */
			temp = min_cost + 100;
			train_planner_update_cost( cost, parent, temp, next_node->index, min_index );
		}

		if( current_node->type != NODE_EXIT ){
			/* Ahead */
			next_node = current_node->edge[ DIR_AHEAD ].dest;
			if( ! mark[ next_node->index ] ){
				temp = min_cost + current_node->edge[ DIR_AHEAD ].dist;
				train_planner_update_cost( cost, parent, temp, next_node->index, min_index );
			}

			/* Curved, if brancher */
			if( current_node->type == NODE_BRANCH ){
				next_node = current_node->edge[ DIR_CURVED ].dest;
				if( ! mark[ next_node->index ] ){
					temp = min_cost + current_node->edge[ DIR_CURVED ].dist;
					train_planner_update_cost( cost, parent, temp, next_node->index, min_index );
				}
			}
		}
	}

	i = min_index;

	region_clear( &path_display );

	/* Fill in path */
	do {
		current_node = track_graph + i;
		path_node.group = current_node->group;
		path_node.id = current_node->id;

		track_node_id2name( name, path_node.group, path_node.id );
		region_append( &path_display, "<-%s", name );

		status = rbuf_put_front( path, ( uchar* )&path_node );
		if( status == ERR_RBUF_FULL ){
			region_printf( &path_display, "Train %d path finding failed: buffer full\n", train->id );
			Delay( 100 );
			return ERR_FAIL;
		}

		i = parent[ i ];
	} while( i >= 0 );

	return ERR_NONE;
}

void train_planner()
{
	/* TODO: rewrite the whole thing to accomodate multiple trains */
	Planner_request request;
	int tid;
	int status;
	Train_path path_node;
	uint path_length;
	volatile Train_data* train;
	Rbuf path;
	int distance;
	Train_path path_buf[ PATH_BUFFER_SIZE ];
	Region path_display = { 28, 13, 20 - 13, 78 - 28, 1, 1 };
	char name[ 6 ];

	region_init( &path_display );

	/* Initialize command ring */
	status = rbuf_init( &path, ( uchar* )&path_buf, sizeof( Train_path ), PATH_BUFFER_SIZE );
	assert( status == ERR_NONE );

	/* Receive init */
	status = Receive( &tid, ( char* )&request, sizeof( request ) );
	assert( status == sizeof( request ) );
	status = Reply( tid, ( char* )&status, sizeof( status ) );
	assert( status == SYSCALL_SUCCESS );
	
	train = ( Train_data* )request.dst;

	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		if( request.type == PLANNER_WAKEUP ){
			distance = train->distance;
		}
		status = Reply( tid, ( char* )&status, sizeof( status ) );
		assert( status == SYSCALL_SUCCESS );

		switch( request.type ){
		case PLANNER_PATH_PLAN:
			rbuf_reset( &path );
			status = train_planner_plan( request.dst, request.dist_pass, train, &path, &path_length );
			assert( status == ERR_NONE );
			break;
		case PLANNER_WAKEUP:
			
			break;
		}
	}
}

static int train_planner_request( int tid, uint type, Planner_request* request, uint size )
{
	int reply;
	int status;

	request->type = type;

	status = Send( tid, ( char* )request, size, ( char* )&reply, sizeof( reply ) );
	ASSERT_M( status == sizeof( reply ), "got %d\n", status );

	return ERR_NONE;
}

int train_planner_path_plan( int tid, track_node* dst, int dist_pass )
{
	Planner_request request;

	request.dst = dst;
	request.dist_pass = dist_pass;

	return train_planner_request( tid, PLANNER_PATH_PLAN, &request, sizeof( request ) );
}

int train_planner_wakeup( int tid )
{
	Planner_request request;
	return train_planner_request( tid, PLANNER_WAKEUP, &request, sizeof( uint ) );
}

int train_planner_init( int tid, Train_data* train )
{
	int size;
	Planner_request request;

	request.dst = ( track_node* )train;

	return train_planner_request( tid, PLANNER_INIT, &request, sizeof( request ) );
}
