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
	int range;
	int dir;
} Track_request;

typedef struct Track_reserve_reply_s {
	int direction;          /* Againist or same side, see track_reserve.h */
} Track_reply;

static int track_reserved( Train* train, track_node* node, int direction )
{
	track_edge* edge;

	assert( train && node );
	
	assert( ( direction == DIR_CURVED && node->type == NODE_BRANCH ) || direction == DIR_AHEAD );

	switch ( node->type ){
	case NODE_EXIT:
		return RESERVE_FAIL_EXIT;
	default:
		break;
	}

	edge = node->edge + direction;
	if( edge->train && edge->train != train && edge->reserve_version == edge->train->reserve_version ){
		return RESERVE_FAIL_SAME_DIR;
	}
	
	edge = edge->reverse;
	if( edge && edge->train && edge->train != train && edge->reserve_version == edge->train->reserve_version ){
		return RESERVE_FAIL_AGAINST_DIR;
	}

	return RESERVE_SUCCESS;
}

static int track_reserve_node( Train* train, track_node* node, int direction )
{
	int status;
	int other_direction;
	
	/* reserve desired direction */
	status = track_reserved( train, node, direction );

	if( status != RESERVE_SUCCESS ){
		return status;
	} else {
		node->edge[ direction ].train = train;
		node->edge[ direction ].reserve_version = train->reserve_version;
	}

	/* reserve the other direction */
	if ( node->type == NODE_BRANCH ) {
		if ( direction == DIR_AHEAD ) {
			other_direction = DIR_CURVED;
		}
		else {
			other_direction = DIR_AHEAD;
		}
		status = track_reserved( train, node, other_direction );
		if( status != RESERVE_SUCCESS ){
			return status;
		} else {
			node->edge[ other_direction ].train = train;
			node->edge[ other_direction ].reserve_version = train->reserve_version;
		}
	}

	/* reserve merge */
	if ( node->edge[ direction ].dest->type == NODE_MERGE ) {
		node = node->edge[ direction ].dest->reverse;
		assert( node->type == NODE_BRANCH );

		direction = DIR_STRAIGHT;
		status = track_reserved( train, node, direction );
		if( status != RESERVE_SUCCESS ){
			return status;
		} else {
			node->edge[ direction ].train = train;
			node->edge[ direction ].reserve_version = train->reserve_version;
		}

		direction = DIR_CURVED;
		status = track_reserved( train, node, direction );
		if( status != RESERVE_SUCCESS ){
			return status;
		} else {
			node->edge[ direction ].train = train;
			node->edge[ direction ].reserve_version = train->reserve_version;
		}
	}

	return RESERVE_SUCCESS;
}

static int track_reserve_hold_node( Train* train, track_node* node ){
	int direction = DIR_AHEAD;

	if ( node->edge[ direction ].train == train && node->edge[ direction ].reserve_version == train->reserve_version ) {
		return RESERVE_HOLD;
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

		switch( request.type ){
		case TRACK_RESERVE_MAY_I_RANGE:
			range = request.range;
			direction = request.dir;
			do {
				reply.direction = track_reserved( request.train, node, direction );

				if( reply.direction != RESERVE_SUCCESS ){
					break;
				}
				
				range -= node->edge[ direction ].dist;

				node = track_next_node( node, switch_table );
				if( ! node ){
					reply.direction = RESERVE_FAIL_AGAINST_DIR;
					range = 0;
				}

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
				reply.direction = track_reserve_node( request.train, node, direction );

				if( reply.direction != RESERVE_SUCCESS ){
					break;
				}
				
				range -= node->edge[ direction ].dist;

				node = track_next_node( node, switch_table );
				if( ! node ){
					reply.direction = RESERVE_FAIL_AGAINST_DIR;
					range = 0;
				}
			} while( range > 0 );
			break;
		case TRACK_RESERVE_MAY_I:
			direction = request.dir;
			reply.direction = track_reserved( request.train, node, direction );
			break;
		case TRACK_RESERVE_GET:
			reply.direction = track_reserve_node( request.train, node, direction );
			break;
		case TRACK_RESERVE_PUT:
			if( track_reserved( request.train, node, direction ) == RESERVE_SUCCESS ){
				node->edge[ direction ].train = 0;
			}
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

int track_reserve_may_i_range( int tid, Train* train, track_node* node, int dist, int direction )
{
	Track_request request;
	Track_reply reply;
	int status;

	assert( train );

	request.type = TRACK_RESERVE_MAY_I_RANGE;
	request.train = train;
	request.node = node;
	request.range = dist;
	request.dir = direction;

	status = track_reserve_request( tid, &request, &reply );
	
	return reply.direction;
}

int track_reserve_get_range( int tid, Train* train, int dist )
{
	Track_request request;
	Track_reply reply;
	int status;

	assert( train );

	request.type = TRACK_RESERVE_GET_RANGE;
	request.train = train;
	request.node = train->check_point;
	assert( request.node );
	request.range = dist;

	status = track_reserve_request( tid, &request, &reply );
	
	return reply.direction;
}

int track_reserve_may_i( int tid, Train* train, track_node* node, int direction )
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

int track_reserve_get( int tid, Train* train, track_node* node )
{
	Track_request request;
	Track_reply reply;
	int status;

	assert( train && node );

	request.type = TRACK_RESERVE_GET;
	request.train = train;
	request.node = node;

	status = track_reserve_request( tid, &request, &reply );
	
	return reply.direction;
}

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
