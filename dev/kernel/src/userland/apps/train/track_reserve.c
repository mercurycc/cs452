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
	TRACK_RESERVE_GET,
	TRACK_RESERVE_PUT,
	TRACK_RESERVE_FREE
};

typedef struct Track_reserve_request_s {
	uint type;
	Train* train;
	track_node* node;
	int range_dir;
} Track_request;

typedef struct Track_reserve_reply_s {
	int direction;          /* Againist or same side, see track_reserve.h */
} Track_reply;

static int track_reserved( Train* train, track_node* node, int direction )
{
	track_edge* edge;
	
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
	if( edge->train && edge->train != train && edge->reserve_version == edge->train->reserve_version ){
		return RESERVE_FAIL_AGAINST_DIR;
	}

	return RESERVE_SUCCESS;
}

static int track_reserve_node( Train* train, track_node* node, int direction )
{
	int status;
	
	status = track_reserved( train, node, direction );

	if( status != RESERVE_SUCCESS ){
		return status;
	} else {
		node->edge[ direction ].train = train;
		node->edge[ direction ].reserve_version = train->reserve_version;
	}

	return RESERVE_SUCCESS;
}

void track_reserve()
{
	Track_request request;
	Track_reply reply;
	int range;
	int* switch_table = 0;
	track_node* node;
	int status;
	int direction;
	int tid;
	char* name;

	status = RegisterAs( RESERVE_NAME );
	assert( status == REGISTER_AS_SUCCESS );

	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		assert( status == sizeof( request ) );

		node = request.node;
		if( ! switch_table ){
			switch_table = request.train->switch_table;
		}

		/* Convert direction */
		direction = DIR_AHEAD;
		if( node->type == NODE_BRANCH && switch_table[ SWID_TO_ARRAYID( node->id + 1 ) ] == 'C' ){
			direction = DIR_CURVED;
		}

		switch( request.type ){
		case TRACK_RESERVE_GET_RANGE:
			range = request.range_dir;
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
			} while( range > 0 );
			break;
		case TRACK_RESERVE_MAY_I:
			direction = request.range_dir;
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

int track_reserve_get_range( int tid, Train* train, int dist )
{
	Track_request request;
	Track_reply reply;
	int status;

	request.type = TRACK_RESERVE_GET_RANGE;
	request.train = train;
	request.node = train->check_point;
	request.range_dir = dist;

	status = track_reserve_request( tid, &request, &reply );
	
	return reply.direction;
}

int track_reserve_may_i( int tid, Train* train, track_node* node, int direction )
{
	Track_request request;
	Track_reply reply;
	int status;

	request.type = TRACK_RESERVE_MAY_I;
	request.train = train;
	request.node = node;
	request.range_dir = direction;

	status = track_reserve_request( tid, &request, &reply );
	
	return reply.direction;
}

int track_reserve_get( int tid, Train* train, track_node* node )
{
	Track_request request;
	Track_reply reply;
	int status;

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

	request.type = TRACK_RESERVE_FREE;
	request.train = train;

	status = track_reserve_request( tid, &request, &reply );
	
	return reply.direction;
}
