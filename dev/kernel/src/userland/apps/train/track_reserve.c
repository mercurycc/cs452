#include <user/syscall.h>
#include <user/name_server.h>
#include <user/time.h>
#include <user/dprint.h>
#include <user/assert.h>
#include "inc/train_types.h"
#include "inc/track_node.h"
#include "inc/train_location.h"
#include "inc/track_reserve.h"
#include "inc/train.h"
#include "inc/config.h"

#define LOCAL_DEBUG
#include <user/dprint.h>

enum Track_reserve_type {
	TRACK_RESERVE_GET_RANGE,
	TRACK_RESERVE_MAY_I,
	TRACK_RESERVE_MAY_I_RANGE,
	TRACK_RESERVE_GET,
	TRACK_RESERVE_PUT,
	TRACK_RESERVE_FREE,
	TRACK_RESERVE_HOLDS
};

typedef struct Track_reserve_request_s {
	uint type;
	Train* train;
	track_node* node;
	int from;
	int range;
	int dir;
} Track_request;

typedef struct Track_reserve_reply_s {
	int direction;          /* Againist or same side, see track_reserve.h */
} Track_reply;

static inline int track_reserve_in_use( Track_reserve* reserve, Train* train )
{
	return reserve->train && reserve->train != train && reserve->version == reserve->train->reserve_version;
}

static inline int track_reserve_no_overlap( Track_reserve* reserve, int from, int to )
{
	return ( ( from > reserve->to ) || ( reserve->from > to ) );
}

static inline int track_find_free_reserve( Train* train, track_edge* edge, int from, int to )
{
	int i;
	Track_reserve* reserve;
	int in_use;
	int overlap;
	int index = 0;

	assert( from <= to );
	
	for( i = 0; i < MAX_NUM_TRAINS; i += 1 ){
		reserve = edge->close_reserves + i;

		in_use = track_reserve_in_use( reserve, train );
		overlap = ! track_reserve_no_overlap( reserve, from, to );

		if( in_use && overlap ){
			return -1;
		}

		if( ! in_use ){
			index = i;
		}
	}

	return index;
}

static int track_reserved( Train* train, track_node* node, int direction, int from, int to, int* index )
{
	track_edge* edge;
	int reverse_from;
	int reverse_to;
	int temp;

	assert( train && node );
	
	assert( ( direction == DIR_CURVED && node->type == NODE_BRANCH ) || direction == DIR_AHEAD );

	switch ( node->type ){
	case NODE_EXIT:
		return RESERVE_FAIL_EXIT;
	default:
		break;
	}

	edge = node->edge + direction;
	*index = track_find_free_reserve( train, edge, from, to );
	if( *index < 0 ){
		return RESERVE_FAIL_SAME_DIR;
	}
	
	edge = edge->reverse;
	reverse_from = edge->dist - to;
	reverse_to = edge->dist - from;
	temp = track_find_free_reserve( train, edge, reverse_from, reverse_to );
	if( temp < 0 ){
		return RESERVE_FAIL_AGAINST_DIR;
	}

	return RESERVE_SUCCESS;
}

static inline void track_take_reserve( track_node* node, int direction, int index, Train* train, int from, int to )
{
	Track_reserve* reserve;
	track_edge* edge;

	edge = node->edge + direction;

	assert( from >= 0 && from <= to );
	assert( from <= edge->dist );
	if( to > edge->dist ){
		to = edge->dist;
	}
	
	reserve = edge->close_reserves + index;
	
	reserve->train = train;
	reserve->version = train->reserve_version;
	reserve->from = from;
	reserve->to = to;
}

static int track_reserve_node( Train* train, track_node* node, int direction, int from, int to )
{
	int status;
	int other_direction;
	Track_reserve* reserve;
	int index;
	
	/* reserve desired direction */
	status = track_reserved( train, node, direction, from, to, &index );

	if( status != RESERVE_SUCCESS ){
		return status;
	} else {
		reserve = node->edge[ direction ].close_reserves + index;
		if( !( reserve->train == train && reserve->version == train->reserve_version && reserve->from <= from && reserve->to >= to ) ){
			track_take_reserve( node, direction, index, train, from, to );
		}
		/* Do not take action if the reserve is already in hold, so we don't decrease our reservation */
	}

	/* reserve the other direction */
	if ( node->type == NODE_BRANCH ) {
		if ( direction == DIR_AHEAD ) {
			other_direction = DIR_CURVED;
		}
		else {
			other_direction = DIR_AHEAD;
		}
		status = track_reserved( train, node, other_direction, 0, TRACK_RESERVE_SAFE_DISTANCE, &index );
		if( status != RESERVE_SUCCESS ){
			return status;
		} else {
			track_take_reserve( node, direction, index, train, 0, TRACK_RESERVE_SAFE_DISTANCE );
		}
	}

	/* reserve merge */
	if ( node->edge[ direction ].dest->type == NODE_MERGE ) {
		node = node->edge[ direction ].dest->reverse;
		assert( node->type == NODE_BRANCH );

		direction = DIR_STRAIGHT;
		status = track_reserved( train, node, direction, 0, TRACK_RESERVE_SAFE_DISTANCE, &index );
		if( status != RESERVE_SUCCESS ){
			return status;
		} else {
			track_take_reserve( node, direction, index, train, 0, TRACK_RESERVE_SAFE_DISTANCE );
		}

		direction = DIR_CURVED;
		status = track_reserved( train, node, direction, 0, TRACK_RESERVE_SAFE_DISTANCE, &index );
		if( status != RESERVE_SUCCESS ){
			return status;
		} else {
			track_take_reserve( node, direction, index, train, 0, TRACK_RESERVE_SAFE_DISTANCE );
		}
	}

	return RESERVE_SUCCESS;
}

static int track_reserve_hold_node( Train* train, track_node* node )
{
	Track_reserve* reserve;
	int in_use;
	int overlap;
	int index = 0;
	track_edge* edge = node->edge + DIR_AHEAD;
	int i;

	for( i = 0; i < MAX_NUM_TRAINS; i += 1 ){
		reserve = edge->close_reserves + i;

		in_use = track_reserve_in_use( reserve, train );

		if( ! ( in_use ) ){
			if( reserve->train == train && reserve->version == train->reserve_version && reserve->from == 0 ){
				return RESERVE_HOLD;
			}
		}
	}

	return RESERVE_NOT_HOLD;
}

void track_reserve()
{
	Track_request request;
	Track_reply reply;
	int range;
	volatile int* switch_table = 0;
	track_node* node;
	int status;
	int direction;
	int section_from;
	int section_to;
	int edge_length;
	int index;
	int tid;

	status = RegisterAs( RESERVE_NAME );
	assert( status == REGISTER_AS_SUCCESS );

	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		assert( status == sizeof( request ) );

		node = request.node;
		switch_table = request.train->switch_table;

		/* Convert direction */
                if( node ){
			direction = DIR_AHEAD;
			if( node->type == NODE_BRANCH && switch_table[ SWID_TO_ARRAYID( node->id + 1 ) ] == 'C' ){
				direction = DIR_CURVED;
			}
		}

		section_from = request.from;

		/* Given the inaccuracy of tracking system, these kind of stupid things are all over the place */
		if( section_from < 0 ){
			section_from = 0;
		}
		
		switch( request.type ){
		case TRACK_RESERVE_MAY_I_RANGE:
			range = request.range;
			direction = request.dir;
			do {
				edge_length = node->edge[ direction ].dist;

				if( section_from > edge_length ){
					section_from = edge_length;
				}

				section_to = range + section_from;
				
				if( section_to > edge_length ){
					section_to = edge_length;
				}

				reply.direction = track_reserved( request.train, node, direction, section_from, section_to, &index );

				if( reply.direction != RESERVE_SUCCESS ){
					break;
				}
				
				range -= section_to - section_from;

				node = track_next_node( node, switch_table );
				if( ! node ){
					reply.direction = RESERVE_FAIL_AGAINST_DIR;
					range = 0;
				}

				section_from = 0;

				direction = DIR_AHEAD;
				if( node->type == NODE_BRANCH && switch_table[ SWID_TO_ARRAYID( node->id + 1 ) ] == 'C' ){
					direction = DIR_CURVED;
				}
			} while( range > 0 );
			break;
		case TRACK_RESERVE_GET_RANGE:
			range = request.range;
			do {
				direction = DIR_AHEAD;
				if( node->type == NODE_BRANCH && switch_table[ SWID_TO_ARRAYID( node->id + 1 ) ] == 'C' ){
					direction = DIR_CURVED;
				}

				edge_length = node->edge[ direction ].dist;

				if( section_from > edge_length ){
					section_from = edge_length;
				}
				
				section_to = range + section_from;

				if( section_to > edge_length ){
					section_to = edge_length;
				}

				reply.direction = track_reserve_node( request.train, node, direction, section_from, section_to );
				if( reply.direction != RESERVE_SUCCESS ){
					break;
				}
				
				range -= section_to - section_from;

				node = track_next_node( node, switch_table );
				if( ! node ){
					reply.direction = RESERVE_FAIL_AGAINST_DIR;
					range = 0;
				}

				section_from = 0;				
			} while( range > 0 );
			break;
		case TRACK_RESERVE_MAY_I:
			direction = request.dir;
			reply.direction = track_reserved( request.train, node, direction, request.from, request.from + request.range, &index );
			break;
		case TRACK_RESERVE_GET:
			assert( 0 );
			/* This is not safe at the moment, given the range can go beyond the edge */
			reply.direction = track_reserve_node( request.train, node, direction, request.from, request.from + request.range );
			break;
		case TRACK_RESERVE_PUT:
			assert( 0 );
			break;
		case TRACK_RESERVE_FREE:
			request.train->reserve_version += 1;
			break;
		case TRACK_RESERVE_HOLDS:
			reply.direction = track_reserve_hold_node( request.train, node );
			break;
		}

		status = Reply( tid, ( char* )&reply, sizeof( reply ) );
		assert( status == SYSCALL_SUCCESS );
	}
}

static int track_reserve_request( int tid, Track_request* request, Track_reply* reply )
{
	int status;

	status = Send( tid, ( char* )request, sizeof( Track_request ), ( char* )reply, sizeof( Track_reply ) );
	assert( status == sizeof( reply ) );

	return ERR_NONE;
}

int track_reserve_may_i_range( int tid, Train* train, track_node* node, int from, int dist, int direction )
{
	Track_request request;
	Track_reply reply;
	int status;

	assert( train );

	request.type = TRACK_RESERVE_MAY_I_RANGE;
	request.train = train;
	request.node = node;
	request.from = from;
	request.range = dist;
	request.dir = direction;

	status = track_reserve_request( tid, &request, &reply );
	
	return reply.direction;
}

int track_reserve_get_range( int tid, Train* train, int from, int dist )
{
	Track_request request;
	Track_reply reply;
	int status;

	assert( train );

	request.type = TRACK_RESERVE_GET_RANGE;
	request.train = train;
	request.node = train->check_point;
	assert( request.node );
	request.from = from;
	request.range = dist;

	status = track_reserve_request( tid, &request, &reply );
	
	return reply.direction;
}

int track_reserve_may_i( int tid, Train* train, track_node* node, int direction, int from, int dist )
{
	Track_request request;
	Track_reply reply;
	int status;

	assert( train && node );

	request.type = TRACK_RESERVE_MAY_I;
	request.train = train;
	request.node = node;
	request.dir = direction;

	status = track_reserve_request( tid, &request, &reply );
	
	return reply.direction;
}

int track_reserve_get( int tid, Train* train, track_node* node, int from, int dist )
{
	Track_request request;
	Track_reply reply;
	int status;

	assert( train && node );

	request.type = TRACK_RESERVE_GET;
	request.train = train;
	request.node = node;
	request.from = from;
	request.range = dist;

	status = track_reserve_request( tid, &request, &reply );
	
	return reply.direction;
}

#if 0 /* No single section free allowed */
int track_reserve_put( int tid, Train* train, track_node* node )
{
	Track_request request;
	Track_reply reply;
	int status;

	assert( train && node );

	request.type = TRACK_RESERVE_PUT;
	request.train = train;
	request.node = node;

	status = track_reserve_request( tid, &request, &reply );
	
	return reply.direction;
}
#endif /* if 0 */

int track_reserve_free( int tid, Train* train )
{
	Track_request request;
	Track_reply reply;
	int status;

	assert( train );
	
	request.type = TRACK_RESERVE_FREE;
	request.train = train;
	request.node = 0;

	status = track_reserve_request( tid, &request, &reply );
	
	return reply.direction;
}

int track_reserve_holds( int tid, Train* train, track_node* node ) {
	Track_request request;
	Track_reply reply;
	int status;

	request.type = TRACK_RESERVE_HOLDS;
	request.train = train;
	request.node = node;

	status = track_reserve_request( tid, &request, &reply );
	
	return reply.direction;
}
