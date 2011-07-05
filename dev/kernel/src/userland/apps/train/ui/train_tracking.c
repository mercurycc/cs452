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

typedef struct Tracking_ui_s Tracking_ui_request;
struct Tracking_ui_s {
	uint type;
	uint train_id;
	union {
		struct {
			int group;
			int id;
			int predict;
			int diff;
		} chkpnt;
		struct {
			int group;
			int id;
		} landmrk;
		struct {
			int dist;
		} dist;
		struct {
			int group;
			int id;
			int eta;
		} nextmrk;
		struct {
			int speed;
		} speed;
	} data;
};

enum Tracking_ui_request_type {
	TRACKING_UI_NEW_TRAIN,
	TRACKING_UI_CHKPNT,
	TRACKING_UI_LANDMRK,
	TRACKING_UI_DIST,
	TRACKING_UI_NEXT,
	TRACKING_UI_SPEED
};

void tracking_ui()
{
	Tracking_ui_request request;
	int available_slot = 1;
	/* UI specific, see UIDesign */
	Region title_reg = { 28, 3, 1, 73 - 28, 1, 0 };
	Region data_reg = { 28, 4, 12 - 4, 78 - 28, 1, 1 };
	Region entry;
	int train_map[ MAX_NUM_TRAINS ] = { 0 };
	char mark_name[ 16 ];
	Timespec time;
	int i;
	int tid;
	int status;

	status = RegisterAs( TRACKING_UI_NAME );
	assert( status == REGISTER_AS_SUCCESS );

	region_init( &title_reg );
	region_printf( &title_reg, "Train Tracking / d.(mm), t.(10ms), v.(mm/s)\n" );
	region_init( &data_reg );
	region_printf( &data_reg,
		       " #|Chk| Pred  |Diff | Mrk |Dist| Nxt | ETA|Spd" );

	entry.col = 30;
	entry.height = 1;
	entry.width = 76 - 30;
	entry.margin = 0;
	entry.boundary = 0;
	for( i = 0; i < 5; i += 1 ){
		entry.row = 6 + i;
		region_init( &entry );
		region_printf( &entry, "  |   |       |     |     |    |     |    |   " );
	}
	
	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		status -= sizeof( uint ) * 2;

		entry.boundary = 0;
		
		/* Size check */
		switch( request.type ){
		case TRACKING_UI_NEW_TRAIN:
			assert( status == 0 );
			break;
		case TRACKING_UI_CHKPNT:
			assert( status == sizeof( request.data.chkpnt ) );
			break;
		case TRACKING_UI_LANDMRK:
			assert( status == sizeof( request.data.landmrk ) );
			break;
		case TRACKING_UI_DIST:
			assert( status == sizeof( request.data.dist ) );
			break;
		case TRACKING_UI_NEXT:
			assert( status == sizeof( request.data.nextmrk ) );
			break;
		case TRACKING_UI_SPEED:
			assert( status == sizeof( request.data.speed ) );
			break;
		}
		
		status = Reply( tid, ( char* )&status, sizeof( status ) );
		assert( status == SYSCALL_SUCCESS );

		switch( request.type ){
		case TRACKING_UI_NEW_TRAIN:
			train_map[ request.train_id ] = available_slot;
			available_slot += 1;
		default:
			break;
		}

		entry.row = train_map[ request.train_id ] + 5;

		switch( request.type ){
		case TRACKING_UI_NEW_TRAIN:
			entry.col = 30;
			entry.width = 2;
			region_printf( &entry, "%2d\n", request.train_id );
			WAR_PRINT( "train %d registered, r %d c %d h %d w %d m %d b %d\n", request.train_id,
				   entry.row, entry.col, entry.height, entry.width, entry.margin, entry.boundary );
			break;
		case TRACKING_UI_CHKPNT:
			entry.col = 33;
			entry.width = 3;
			track_node_id2name( mark_name, request.data.chkpnt.group, request.data.chkpnt.id );
			region_printf( &entry, "%s\n", mark_name );
			entry.col = 37;
			entry.width = 44 - 37;
			time_tick_to_spec( &time, request.data.chkpnt.predict );
			region_printf( &entry, "%02u:%02u:%02u\n", time.minute, time.second, time.fraction );
			entry.col = 45;
			entry.width = 5;
			time_tick_to_spec( &time, request.data.chkpnt.diff < 0 ? - request.data.chkpnt.diff : request.data.chkpnt.diff );
			region_printf( &entry, "%c%u:%u\n", request.data.chkpnt.diff < 0 ? '-' : '+', time.second, time.fraction );
			assert( ! time.minute );
			break;
		case TRACKING_UI_LANDMRK:
			entry.col = 51;
			entry.width = 5;
			track_node_id2name( mark_name, request.data.landmrk.group, request.data.landmrk.id );
			region_printf( &entry, "%s\n", mark_name );
			break;
		case TRACKING_UI_DIST:
			entry.col = 57;
			entry.width = 4;
			region_printf( &entry, "%4d\n", request.data.dist.dist );
			break;
		case TRACKING_UI_NEXT:
			entry.col = 62;
			entry.width = 5;
			track_node_id2name( mark_name, request.data.nextmrk.group, request.data.nextmrk.id );
			region_printf( &entry, "%s\n", mark_name );
			entry.col = 68;
			entry.width = 4;
			region_printf( &entry, "%4d\n", request.data.nextmrk.eta );
			break;
		case TRACKING_UI_SPEED:
			entry.col = 73;
			entry.width = 3;
			region_printf( &entry, "%3d\n", request.data.speed.speed );
			break;
		default:
			assert( 0 );
		}
	}	
}

static int tracking_ui_request( int tid, Tracking_ui_request* request, uint size )
{
	int status;
	int reply;

	size += 2 * sizeof( uint );

	status = Send( tid, ( char* )request, size, ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );

	return ERR_NONE;
}

int tracking_ui_new_train( int tid, int train_id )
{
	Tracking_ui_request request;

	request.type = TRACKING_UI_NEW_TRAIN;
	request.train_id = train_id;

	return tracking_ui_request( tid, &request, 0 );
}

int tracking_ui_chkpnt( int tid, int train_id, int group, int id, int predict, int diff )
{
	Tracking_ui_request request;

	request.type = TRACKING_UI_CHKPNT;
	request.data.chkpnt.group = group;
	request.data.chkpnt.id = id;
	request.data.chkpnt.predict = predict;
	request.data.chkpnt.diff = diff;
	request.train_id = train_id;

	return tracking_ui_request( tid, &request, sizeof( request.data.chkpnt ) );
}

int tracking_ui_landmrk( int tid, int train_id, int group, int id )
{
	Tracking_ui_request request;

	request.type = TRACKING_UI_LANDMRK;
	request.data.landmrk.group = group;
	request.data.landmrk.id = id;
	request.train_id = train_id;

	return tracking_ui_request( tid, &request, sizeof( request.data.landmrk ) );
}

int tracking_ui_dist( int tid, int train_id, int dist )
{
	Tracking_ui_request request;

	request.type = TRACKING_UI_DIST;
	request.data.dist.dist = dist;
	request.train_id = train_id;

	return tracking_ui_request( tid, &request, sizeof( request.data.dist ) );

}

int tracking_ui_nextmrk( int tid, int train_id, int group, int id, int eta )
{
	Tracking_ui_request request;

	request.type = TRACKING_UI_NEXT;
	request.data.nextmrk.group = group;
	request.data.nextmrk.id = id;
	request.data.nextmrk.eta = eta;
	request.train_id = train_id;

	return tracking_ui_request( tid, &request, sizeof( request.data.nextmrk ) );

}

int tracking_ui_speed( int tid, int train_id, int speed )
{
	Tracking_ui_request request;

	request.type = TRACKING_UI_SPEED;
	request.data.speed.speed = speed;
	request.train_id = train_id;

	return tracking_ui_request( tid, &request, sizeof( request.data.speed ) );
}
