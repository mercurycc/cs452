#include <user/syscall.h>
#include <user/name_server.h>
#include <user/time.h>
#include "inc/train_types.h"
#include "inc/track_node.h"
#include "inc/track_reserve.h"
#include "inc/train.h"
#include "inc/config.h"

enum Track_reserve_type {
	TRACK_RESERVE_GET_RANGE,
	TRACK_RESERVE_GET,
	TRACK_RESERVE_PUT
};

typedef struct Track_reserve_request_s {
	uint type;
	Train* train;
	track_node* node;
	int range_dir;
} Track_request;

typedef struct Track_reserve_reply_s {
	Train* train;           /* The train that reserved the node */
	int direction;          /* Againist or same side */
} Track_reply;

Train* track_reserved( track_node* node, int direction )
{
	track_edge* edge;
	Train* train = 0;
	
	assert( ( direction == DIR_CURVED && node.type == NODE_BRANCH ) || direction = DIR_AHEAD );

	edge = node->edge + direction;

	if( edit->train && edge->reserve_version == edge->train->reserve_version ){
		train = edit->train;
	} else if( edit->reverse->train && edge->reverse->reserve_version == edge->reverse->train->reserve_version ){
		train = edit->reverse->train;
	}

	return train;
}

void track_reserve()
{
	Track_request request;
	Track_reply reply;
	int range;
	int* switch_table;
	track_node* node;
	Train* regtrain;
	int status;
	int direction;
	int tid;

	status = RegisterAs( RESERVE_NAME );
	assert( status == REGISTERAS_SUCCESS );

	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		assert( status == sizeof( request ) );

		range = request.range;
		node = request.node;
		if( ! switch_table ){
			switch_table = request.train->switch_table;
		}

		/* Mark as not occupied */
		reply.train = 0;

		switch( request.type ){
		case TRACK_RESERVE_GET_RANGE:
			range = request.range;
			do {
				direction = DIR_AHEAD;
				if( node->type == NODE_BRANCH && switch_table[ SWID_TO_ARRAYID( node->id + 1 ) ] == 'C' ){
					direction = DIR_CURVED;
				}
				if( regtrain = track_reserved( node ) ){
					reply.train = regtrain;
					reply.
					break;
				} else {
					node->edge[ direction ].train = request.train;
					node->edge[ direction ].reserve_version = request.train->reserve_version;
				}
			} while( range > 0 );
			break;
		case TRACK_RESERVE_GET:
			/* Not implemented */
			assert( 0 );
			break;
		case TRACK_RESERVE_PUT:
			/* Not implemented */
			break;
		}

		status = Reply( tid, ( char* )&reply, sizeof( reply ) );
		assert( status == SYSCALL_SUCCESS );
	}
}

static int track_reserve_request( int tid, Track_request* request, Track_reply* reply )
{
	int status;

	status = Send( tid, ( char* )request, sizeof( request ), ( char* )reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );

	return ERR_NONE;
}

Train* track_reserve_get_range( int tid, Train* train, track_node* node, int dist )
{
	Track_request request;
	Track_reply reply;

	
	
	return 
}
