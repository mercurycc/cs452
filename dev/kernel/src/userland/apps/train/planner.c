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
#include "inc/train_tracking.h"
#include <perf.h>

#define LOCAL_DEBUG
#include <user/dprint.h>

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

static inline void train_planner_update_cost( uint* cost, int parent[][ 2 ], uint new_cost, int index, int parent_index, int direction )
{
	if( cost[ index ] > new_cost ){
		cost[ index ] = new_cost;
		parent[ index ][ 0 ] = parent_index;
		parent[ index ][ 1 ] = direction;
	}
}

#define DST_DIRECT( current_node, dst )      ( current_node == dst )
#define DST_REVERSE( current_node, dst )     ( current_node == dst->edge[ DIR_AHEAD ].dest->reverse )
#define DST_REVERSE_BR( current_node, dst )  ( dst->type == NODE_BRANCH && current_node == dst->edge[ DIR_CURVED ].dest->reverse )
#define MAP_NODE( group, id )                ( track_graph + group * TRACK_GRAPH_NODES_PER_GROUP + ( ( group == GROUPMR || group == GROUPBR || id < 18 ) ? id : id - 134 ) )

static inline int train_planner_is_dst( const track_node* current, const track_node* dst )
{
	return  DST_DIRECT( current, dst ) || DST_REVERSE( current, dst ) || DST_REVERSE_BR( current, dst );
}

static int train_planner_plan( const track_node* dst, int* dist_pass, const Train_data* train, Rbuf* path, uint* direction )
{
	uint cost[ TRACK_NUM_NODES ];                /* ~0 for infinity */
	int parent[ TRACK_NUM_NODES ][ 2 ];          /* -1 for no parent, second element for direction */
	int mark[ TRACK_NUM_NODES ];
	uint min_cost;
	int min_index;
	int temp;
	int i;
	const track_node* current_node;
	const track_node* next_node;
	const track_node* track_graph = train->track_graph;
	const track_node* check_point = train->check_point;
	const track_node* next_check_point = train->next_check_point;
	Region path_display = { 28, 13, 20 - 13, 78 - 28, 1, 1 };
	Train_path path_node;
	char name[ 6 ];
	int status;

	/* Initialize cost */
	for( i = 0; i < TRACK_NUM_NODES; i += 1 ){
		cost[ i ] = ~0;
		mark[ i ] = 0;
	}

	/* Find true destination */
	if( dist_pass < 0 ){
		*dist_pass = -*dist_pass;
		dst = dst->reverse;
	}
	
	while( dst->type != NODE_EXIT && *dist_pass > dst->edge[ DIR_AHEAD ].dist ){
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
	cost[ check_point->reverse->index ] = train_tracking_position( train );
	parent[ check_point->reverse->index ][ 0 ] = -1;
	cost[ next_check_point->index ] = train_tracking_remaining_distance( train );
	parent[ next_check_point->index ][ 0 ] = -1;

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
			if( ! DST_DIRECT( current_node, dst ) ){
				*dist_pass = current_node->edge[ DIR_AHEAD ].dist - *dist_pass;
			}
			if( current_node->type == NODE_BRANCH ){
				if( current_node->edge[ DIR_AHEAD ].dest == dst ){
					path_node.direction = 'S';
				} else {
					path_node.direction = 'C';
				}
			}
			break;
		}

		temp = min_cost;
		/* Update neighbor cost */
		/* Reverse */
		next_node = current_node->reverse;
		if( ! mark[ next_node->index ] ){
			/* TODO: this 1500 is not accurate.  Should be around 2 stop distance at stop speed */
			temp += 1500;
			train_planner_update_cost( cost, parent, temp, next_node->index, min_index, 'S' );
		}

		if( current_node->type != NODE_EXIT ){
			/* Ahead */
			next_node = current_node->edge[ DIR_AHEAD ].dest;
			if( ! mark[ next_node->index ] ){
				temp += current_node->edge[ DIR_AHEAD ].dist;
				train_planner_update_cost( cost, parent, temp, next_node->index, min_index, 'S' );
			}

			/* Curved, if brancher */
			if( current_node->type == NODE_BRANCH ){
				next_node = current_node->edge[ DIR_CURVED ].dest;
				if( ! mark[ next_node->index ] ){
					temp += current_node->edge[ DIR_CURVED ].dist;
					train_planner_update_cost( cost, parent, temp, next_node->index, min_index, 'C' );
				}
			}
		}
	}

	i = min_index;

	region_clear( &path_display );

	/* Fill in path */
	do {
		current_node = track_graph + i;
		path_node.node = current_node;

		track_node_id2name( name, current_node->group, current_node->id );
		region_append( &path_display, "<-%s", name );
		if( current_node->type == NODE_BRANCH ){
			region_append( &path_display, "%c", path_node.direction );
		}

		status = rbuf_put_front( path, ( uchar* )&path_node );
		if( status == ERR_RBUF_FULL ){
			region_printf( &path_display, "Train %d path finding failed: buffer full\n", train->id );
			Delay( 100 );
			return ERR_FAIL;
		}

		path_node.direction = parent[ i ][ 1 ];
		i = parent[ i ][ 0 ];
	} while( i >= 0 );

	if( current_node == check_point->reverse ){
		*direction = PLANNER_BACKWARD;
	} else if( current_node == next_check_point ){
		*direction = PLANNER_FORWARD;
	}

	return ERR_NONE;
}

static int train_forward_stop_cannot_match( volatile const Train* train, int module_tid, int auto_tid )
{
	// scroll_printf( "Train %d path cannot match with current check point.  Consider abort plan.\n", train->id );

	return -1;
}

static int train_forward_stop( volatile const Train_data* train, Rbuf* path, volatile const int* switch_table, int stop_dist, int module_tid, int auto_tid )
{
	const track_node* match_node;
	int look_ahead = 0;
	int matched_dist = 0;
	Train_path path_node_body;
	Train_path * const path_node = &path_node_body;
	Train_path path_buf[ 2 ][ PATH_LOOK_AHEAD_BUFFER ];
	int i;
	Rbuf path_body;
	Rbuf * const local_path = &path_body;
	Rbuf path_stack_body;
	Rbuf * const local_path_stack = &path_stack_body;
	int path_matched;
	enum { TRAVEL, STOP } state;
	char name[ 6 ];
	int done = 0;
	
	rbuf_init( local_path, ( uchar* )path_buf[ 0 ], sizeof( Train_path ), sizeof( path_buf[ 0 ] ) );
	rbuf_init( local_path_stack, ( uchar* )path_buf[ 1 ], sizeof( Train_path ), sizeof( path_buf[ 1 ] ) );
	
	look_ahead = 0;
	path_matched = 0;

	state = STOP;

	while( ! done ){
		while( look_ahead < PATH_LOOK_AHEAD_DIST && ! rbuf_empty( path ) ){
			rbuf_get( path, ( uchar* )path_node );
			rbuf_put( local_path, ( uchar* )path_node );

			if( path_node->direction == 'C' ){
				look_ahead += path_node->node->edge[ DIR_CURVED ].dist;
			} else {
				look_ahead += path_node->node->edge[ DIR_AHEAD ].dist;
			}

			if( path_node->node->type == NODE_BRANCH ){
				train_switch( module_tid, path_node->node->id + 1, path_node->direction );
			}
		}

		/* Travel is here because we want to set the switches before the train starts */
		if( state != TRAVEL ){
			if( look_ahead < PATH_LOOK_AHEAD_ADJUST_DIST ){
				train_set_speed( module_tid, train->id, TRAIN_CLOSE_TRAVEL_SPEED );
				train_auto_set_speed( auto_tid, train->id, TRAIN_CLOSE_TRAVEL_SPEED );
			} else {
				train_set_speed( module_tid, train->id, TRAIN_TRAVEL_SPEED );
				train_auto_set_speed( auto_tid, train->id, TRAIN_TRAVEL_SPEED );
			}

			state = TRAVEL;
		}

		path_matched = 0;

		matched_dist = 0;

		/* Wait for update */
		sem_acquire_all( train->update );
		
		sem_acquire( train->sem );
		match_node = train->check_point;
		
		do {
			rbuf_get( local_path, ( uchar* )path_node );
			rbuf_put_front( local_path_stack, ( uchar* )path_node );
			/* If the node could be matched, count into the matched distance */
			if( path_node->direction == 'C' ){
				matched_dist += path_node->node->edge[ DIR_CURVED ].dist;
			} else {
				matched_dist += path_node->node->edge[ DIR_AHEAD ].dist;
			}
			if( path_node->node == match_node ){
				rbuf_reset( local_path_stack );
				path_matched = 1;
				break;
			}
		} while( ! rbuf_empty( local_path ) );

		if( ! path_matched ){
			while( ! rbuf_empty( local_path_stack ) ){
				rbuf_get( local_path_stack, ( uchar* )path_node );
				rbuf_put_front( local_path, ( uchar* )path_node );
			}
			train_forward_stop_cannot_match( train, module_tid, auto_tid );
		} else {
			track_node_id2name( name, path_node->node->group, path_node->node->id );
			dprintf( "Train %d last matched %s, %d mm\n", train->id, name, matched_dist );
			look_ahead -= matched_dist;
		}

		if( train_tracking_trav_time( train, look_ahead ) < PATH_LOOK_AHEAD_ADJUST_TIME && state == TRAVEL ){
			assert( ! rbuf_empty( local_path ) );
			dprintf( "Train %d reaching dest\n", train->id );
			state = STOP;

			dprintf( "Train %d stop distance: %d, wait %d ticks\n", train->id,
				 train_tracking_stop_distance( train ),
				 train_tracking_trav_time( train, look_ahead + stop_dist - train_tracking_stop_distance( train ) ) );
			Delay( train_tracking_trav_time( train, look_ahead + stop_dist - train_tracking_stop_distance( train ) ) );

			train_set_speed( module_tid, train->id, 0 );
			train_auto_set_speed( auto_tid, train->id, 0 );

			/* Execution should be completed */
			done = 1;
		}

		sem_release( train->sem );
	}
}

void train_planner()
{
	/* TODO: rewrite the whole thing to accomodate multiple trains */
	Planner_request request;
	int tid;
	uint path_length;
	Train_data* train;
	volatile const int* switch_table;
	const track_node* track_graph;
	const track_node* backward_node;
	const track_node* targets[ TARGET_BUFFER_SIZE ];
	const int* node_map;
	const track_node* final_node;
	Rbuf path_body;
	Rbuf * const path = &path_body;
	Train_path path_buf[ PATH_BUFFER_SIZE ];
	Rbuf forward_path_body;
	Rbuf * const forward_path = &forward_path_body;
	track_node * previous_node = 0;
	Train_path path_node_body;
	Train_path * const path_node = &path_node_body;
	Train_path forward_path_buf[ PATH_BUFFER_SIZE ];
	uint plan_direction;
	uint remain_distance;
	uint dist_pass;
	int module_tid;
	int auto_tid;
	int planning;
	int i, j;
	int distance;
	int near_end;
	Region path_display = { 28, 13, 20 - 13, 78 - 28, 1, 1 };
	char name[ 6 ];
	uint count = 0;
	int status;

	region_init( &path_display );

	/* Receive init */
	status = Receive( &tid, ( char* )&request, sizeof( request ) );
	assert( status == sizeof( request ) );
	status = Reply( tid, ( char* )&status, sizeof( status ) );
	assert( status == SYSCALL_SUCCESS );

	module_tid = WhoIs( TRAIN_MODULE_NAME );
	assert( module_tid > 0 );

	auto_tid = WhoIs( TRAIN_AUTO_NAME );
	assert( module_tid > 0 );
	
	train = ( Train_data* )request.dst;
	track_graph = train->track_graph;
	node_map = train->node_map;
	switch_table = train->switch_table;

	/* Initialize command ring */
	status = rbuf_init( path, ( uchar* )path_buf, sizeof( Train_path ), sizeof( Train_path ) * PATH_BUFFER_SIZE );
	assert( status == ERR_NONE );

	/* Initialize forward buffer */
	status = rbuf_init( forward_path, ( uchar* )forward_path_buf, sizeof( Train_path ), sizeof( Train_path ) * PATH_BUFFER_SIZE );
	assert( status == ERR_NONE );

	while( 1 ){
		train->planner_control = 0;
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		status = Reply( tid, ( char* )&status, sizeof( status ) );
		assert( status == SYSCALL_SUCCESS );
		train->planner_control = 1;

		switch( request.type ){
		case PLANNER_PATH_PLAN:
			dist_pass = request.dist_pass;
			planning = 1;
			rbuf_reset( path );
			rbuf_reset( forward_path );
			
			status = train_planner_plan( request.dst, &dist_pass, train, path, &plan_direction );
			assert( status == ERR_NONE );

			assert( ! rbuf_empty( path ) );

			dprintf( "Planner path planned for %d\n", train->id );

			while( ! rbuf_empty( path ) ){
				dprintf( "Planner filling forward path for train %d\n", train->id );
			
				if( plan_direction == PLANNER_BACKWARD ){
					train_reverse( module_tid, train->id );
					train_auto_set_reverse( auto_tid, train->id );
				}

				previous_node = 0;

				while( ! rbuf_empty( path ) ){
					/* Obtain the next forward path, i.e. till path is empty or a merger reverse*/
					rbuf_get( path, ( uchar* )path_node );

					track_node_id2name( name, path_node->node->group, path_node->node->id );

					dprintf( "%s[%c]", name, path_node->direction );
					
					if( path_node->node->reverse == previous_node ){
						dnotice( "Found reverse node\n" );

						rbuf_put_front( path, ( uchar* )path_node );

						break;
					} else {
						rbuf_put( forward_path, ( uchar* )path_node );
					}

					dnotice( " <- " );

					previous_node = path_node->node;
				}

				dprintf( "Planner forward path filled for %d\n", train->id );
							
				if( train->planner_control ){
					/* If path is empty, then we are at the end of the journey.  Therefore respect required stop distance. */
					dprintf( "Planner forward path executing for %d\n", train->id );
					status = train_forward_stop( train, forward_path, switch_table,
								     ! rbuf_empty( path ) ? PATH_LOOK_AHEAD_DEFAULT_STOP : dist_pass,
								     module_tid, auto_tid );
					dprintf( "Planner forward path executing for %d completed\n", train->id );
					if( status < 0 ){
						break;
					}
				}
				
				plan_direction = PLANNER_BACKWARD;
			}
		default:
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


#if 0
#endif
