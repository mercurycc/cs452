/* Train_auto receives sensor and control data to make judgements */
/* It is not implemented strictly as a administrator because incoming
   requests should be rare relative to its speed (sensor every 60 ms,
   control command every second, such and such) */

#include <types.h>
#include <err.h>
#include <lib/str.h>
#include <user/syscall.h>
#include <user/assert.h>
#include <user/time.h>
#include <user/name_server.h>
#include <config.h>
#include "inc/train.h"
#include "inc/config.h"
#include "inc/sensor_data.h"
#include "inc/track_data.h"
#include "inc/track_node.h"
#include "inc/warning.h"
#include "inc/train_location.h"
#include "inc/train_types.h"


enum Train_state {
	TRAIN_STATE_INIT,             /* Init */
	TRAIN_STATE_TRACKING,         /* Normal state */
	TRAIN_STATE_REVERSE,          /* Just reversed direction */
	TRAIN_STATE_SPEED_CHANGE,     /* Just changed speed */
	TRAIN_STATE_SPEED_ERROR,      /* Speed prediction out of bound error */
	TRAIN_STATE_SWITCH_ERROR,     /* Switch prediction error */
	TRAIN_STATE_UNKNOW            /* Unknown state */
};

enum Train_pickup {
	TRAIN_PICKUP_FRONT,
	TRAIN_PICKUP_BACK,
	TRAIN_PICKUP_UNKONWN
};

enum Train_auto_request_type {
	TRAIN_AUTO_INIT,
	TRAIN_AUTO_NEW_SENSOR_DATA,
	TRAIN_AUTO_NEW_TRAIN,
	TRAIN_AUTO_SET_TRAIN_SPEED,
	TRAIN_AUTO_SET_TRAIN_REVERSE,
	TRAIN_AUTO_SET_SWITCH_DIR,
	TRAIN_AUTO_SET_ALL_SWITCH,
	TRAIN_AUTO_QUERY_SWITCH_DIR,
	TRAIN_AUTO_QUERY_LAST_SENSOR
};

typedef struct Train_auto_request_s {
	uint type;
	union {
		struct {
			uint track_id;
		} init;
		Sensor_data sensor_data;
		struct {
			uint train_id;
			uint pickup;
			uint next_group;  /* The sensor the pickup is heading towards */
			uint next_id;
		} new_train;
		struct {
			uint train_id;
			uint speed_level;
		} set_speed;
		struct {
			uint train_id;
		} set_reverse;
		struct {
			uint switch_id;
			char direction;
		} set_switch;
		struct {
			char direction;
		} set_switch_all;
		struct {
			uint switch_id;
		} query_switch;
		struct {
			uint dummy;
		} query_sensor;
	} data;
} Train_auto_request;

typedef struct Train_auto_reply_s {
	int group;
	int id;
} Train_auto_reply;

void train_auto()
{
	Train_auto_request request;
	Train_auto_reply reply;
	Sensor_data sensor_data;
	int last_sensor_group = -1;
	int last_sensor_id = -1;
	int tid;
	int i;
	int temp;
	track_node track_graph[ TRACK_NUM_NODES ];
	int node_map[ GROUP_COUNT ][ TRACK_GRAPH_NODES_PER_GROUP ];
	Train_data trains[ MAX_NUM_TRAINS ] = { { 0 } };
	int train_map[ MAX_TRAIN_ID ] = { 0 };
	int available_train = 1;     /* Record 0 is not used, to distinguish non-registered car */
	Train_data* current_train;
	int switch_table[ NUM_SWITCHES ] = { 0 };
	int module_tid;
	int status;

	/* Receive initialization data */
	status = Receive( &tid, ( char* )&request, sizeof( request ) );
	assert( status == sizeof( int ) + sizeof( request.data.init ) );
	status = Reply( tid, ( char* )&reply, sizeof( reply ) );
	assert( status == SYSCALL_SUCCESS );

	switch( request.data.init.track_id ){
	case TRACK_A:
		init_tracka( track_graph, node_map );
		//init_trackb( track_graph, node_map );// for test
		break;
	case TRACK_B:
		init_trackb( track_graph, node_map );
		break;
	default:
		assert( 0 );
	}

	status = RegisterAs( TRAIN_AUTO_NAME );
	assert( status == REGISTER_AS_SUCCESS );

	module_tid = WhoIs( TRAIN_MODULE_NAME );
	assert( module_tid > 0 );
	
	

	// test
	Train_data test_train_data;
	Train_data* test_train = &test_train_data;
	test_train->distance = 0;
	test_train->speed_numerator = 0;
	test_train->speed_denominator = 1;
	test_train->speed_level = 0;
	test_train->old_speed_level = 0;
	test_train->last_sensor_time = 0;
	test_train->last_sensor = track_graph + node_map[ 1 ][ 4 ];
	assert( test_train->last_sensor );
	//test_train->check_point = track_graph + node_map[ 6 ][ 4 ];
	track_node* test_sensor = 0;
	
	
	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );

		status -= sizeof( uint );
		/* Size check */
		switch( request.type ){
		case TRAIN_AUTO_INIT:
			/* No more init should be received */
			assert( 0 );
			break;
		case TRAIN_AUTO_NEW_SENSOR_DATA:
			assert( status == sizeof( request.data.sensor_data ) );
			break;
		case TRAIN_AUTO_NEW_TRAIN:
			assert( status == sizeof( request.data.new_train ) );
			break;
		case TRAIN_AUTO_SET_TRAIN_SPEED:
			assert( status == sizeof( request.data.set_speed ) );
			break;
		case TRAIN_AUTO_SET_TRAIN_REVERSE:
			assert( status == sizeof( request.data.set_reverse ) );
			break;
		case TRAIN_AUTO_SET_SWITCH_DIR:
			assert( status == sizeof( request.data.set_switch ) );
			break;
		case TRAIN_AUTO_SET_ALL_SWITCH:
			assert( status == sizeof( request.data.set_switch_all ) );
			break;
		case TRAIN_AUTO_QUERY_SWITCH_DIR:
			assert( status == sizeof( request.data.query_switch ) );
			break;
		case TRAIN_AUTO_QUERY_LAST_SENSOR:
			assert( status == sizeof( request.data.query_sensor ) );
			break;
		}

		/* Quick reply */
		switch( request.type ){
		case TRAIN_AUTO_NEW_SENSOR_DATA:
		case TRAIN_AUTO_NEW_TRAIN:
		case TRAIN_AUTO_SET_TRAIN_SPEED:
		case TRAIN_AUTO_SET_TRAIN_REVERSE:
		case TRAIN_AUTO_SET_SWITCH_DIR:
		case TRAIN_AUTO_SET_ALL_SWITCH:
			status = Reply( tid, ( char* )&reply, sizeof( reply ) );
			assert( status == SYSCALL_SUCCESS );
		default:
			break;
		}

		/* Process request */
		switch( request.type ){
		case TRAIN_AUTO_NEW_SENSOR_DATA:
			/* Unfortuate memcpy */
			memcpy( ( uchar* )&sensor_data, ( uchar* )&request.data.sensor_data, sizeof( sensor_data ) );
			
			/* Update last triggered sensor */
			for( temp = 0; temp < SENSOR_BYTE_COUNT; temp += 1 ){
				if( sensor_data.sensor_raw[ temp ] ){
					last_sensor_group = temp;
					for( i = 0; i < BITS_IN_BYTE; i += 1 ){
						if( sensor_data.sensor_raw[ temp ] & ( ( 1 << 7 ) >> i ) ){
							last_sensor_id = i;
						}
					}
				}
			}
			/* TODO: Need to process new sensor data */
			
			// test version: only one train
			//WAR_PRINT( "new sensor: G%d-%d    ", sensor_data.last_sensor_group, sensor_data.last_sensor_id );

			test_sensor = track_graph + node_map[ sensor_data.last_sensor_group/2 ][ sensor_data.last_sensor_id+(last_sensor_group%2*8) ];
			if ( test_sensor == test_train->last_sensor ) break;
			
			status == update_train_speed( test_train, test_sensor, sensor_data.last_sensor_time );
			assert( status == 0 );
			
			WAR_PRINT( "new sensor: train speed %d / %d    ", test_train->speed_numerator, test_train->speed_denominator );
			
			
			break;
		case TRAIN_AUTO_NEW_TRAIN:
			WAR_NOTICE( "new train registered" );
			current_train = trains + available_train;
			train_map[ request.data.new_train.train_id ] = available_train;
			available_train += 1;
			current_train->id = request.data.new_train.train_id;
			current_train->state = TRAIN_STATE_INIT;
			current_train->pickup = request.data.new_train.pickup;
			current_train->distance = 0;
			current_train->speed_numerator = 0;
			current_train->speed_denominator = 1;
			current_train->speed_level = 0;
			current_train->old_speed_level = 0;
			current_train->last_sensor_time = 0;
			current_train->last_sensor = track_graph + node_map[ request.data.new_train.next_group ][ request.data.new_train.next_id ];
			current_train->check_point = track_graph + node_map[ request.data.new_train.next_group ][ request.data.new_train.next_id ];
			break;
		case TRAIN_AUTO_SET_TRAIN_SPEED:
			current_train = trains + train_map[ request.data.set_speed.train_id ];
			current_train->old_speed_level = current_train->speed_level;
			current_train->state = TRAIN_STATE_SPEED_CHANGE;
			break;
		case TRAIN_AUTO_SET_TRAIN_REVERSE:
			current_train = trains + train_map[ request.data.set_speed.train_id ];
			if( current_train->pickup == TRAIN_PICKUP_FRONT ){
				current_train->pickup = TRAIN_PICKUP_BACK;
			} else if( current_train->pickup == TRAIN_PICKUP_BACK ){
				current_train->pickup = TRAIN_PICKUP_FRONT;
			}
			/* TODO: This will lose precision */
			current_train->check_point = current_train->check_point->reverse;
			current_train->state = TRAIN_STATE_REVERSE;
			break;
		case TRAIN_AUTO_SET_SWITCH_DIR:
			switch_table[ SWID_TO_ARRAYID( request.data.set_switch.switch_id ) ] = request.data.set_switch.direction;
			break;
		case TRAIN_AUTO_SET_ALL_SWITCH:
			for( i = 0; i < NUM_SWITCHES; i += 1 ){
				switch_table[ i ] = request.data.set_switch_all.direction;
			}
		case TRAIN_AUTO_QUERY_SWITCH_DIR:
			reply.group = switch_table[ SWID_TO_ARRAYID( request.data.set_switch.switch_id ) ];
			break;
		case TRAIN_AUTO_QUERY_LAST_SENSOR:
			reply.group = last_sensor_group;
			reply.id = last_sensor_id;
			break;
		}

		/* Late reply */
		switch( request.type ){
		case TRAIN_AUTO_QUERY_SWITCH_DIR:
		case TRAIN_AUTO_QUERY_LAST_SENSOR:
			status = Reply( tid, ( char* )&reply, sizeof( reply ) );
			assert( status == SYSCALL_SUCCESS );
		default:
			break;
		}

		/* Process train states */
		for( temp = 0; temp < available_train; temp += 1 ){
			current_train = trains + temp;
			switch( current_train->state ){
			case TRAIN_STATE_INIT:
				current_train->state = TRAIN_STATE_TRACKING;
				break;
			case TRAIN_STATE_TRACKING:
				break;
			case TRAIN_STATE_REVERSE:
				break;
			case TRAIN_STATE_SPEED_CHANGE:
				break;
			case TRAIN_STATE_SPEED_ERROR:
				break;
			case TRAIN_STATE_SWITCH_ERROR:
				break;
			case TRAIN_STATE_UNKNOW:
				break;
			}
		}
	}
}

static int train_auto_request( int tid, Train_auto_request* data, uint size, int* group, int* id )
{
	int status;
	Train_auto_reply reply;

	status = Send( tid, ( char* )data, size, ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );

	if( group ){
		*group = reply.group;
	}
	if( id ){
		*id = reply.id;
	}

	return ERR_NONE;
}

int train_auto_new_sensor_data( int tid, Sensor_data* data )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_NEW_SENSOR_DATA;

	/* Very unfortunate */
	memcpy( ( uchar* )&request.data.sensor_data, ( uchar* )data, sizeof( Sensor_data ) );

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.sensor_data ), 0, 0 );
}

int train_auto_init( int tid, uint track )
{
	Train_auto_request request;
	
	request.type = TRAIN_AUTO_INIT;
	request.data.init.track_id = track;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.init ), 0, 0 );
}

int train_auto_new_train( int tid, uint id, uint pickup, uint pre_grp, uint pre_id )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_NEW_TRAIN;
	request.data.new_train.train_id = id;
	request.data.new_train.pickup = pickup;
	request.data.new_train.next_group = pre_grp;
	request.data.new_train.next_id = pre_id;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.new_train ), 0, 0 );
}

int train_auto_set_speed( int tid, uint id, uint speed_level )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_SET_TRAIN_SPEED;
	request.data.set_speed.train_id = id;
	request.data.set_speed.speed_level = speed_level;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.set_speed ), 0, 0 );
}

int train_auto_set_reverse( int tid, uint id )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_QUERY_SWITCH_DIR;

	request.data.set_reverse.train_id = id;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.set_reverse ), 0, 0 );
}

int train_auto_set_switch( int tid, uint id, char direction )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_SET_SWITCH_DIR;
	request.data.set_switch.switch_id = id;
	request.data.set_switch.direction = direction;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.set_switch ), 0, 0 );
}

int train_auto_set_switch_all( int tid, char direction )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_SET_ALL_SWITCH;
	request.data.set_switch_all.direction = direction;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.set_switch_all ), 0, 0 );
}

int train_auto_query_switch( int tid, uint id, int* direction )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_QUERY_SWITCH_DIR;
	request.data.query_switch.switch_id = id;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.query_switch ), direction, 0 );
}

int train_auto_query_sensor( int tid, int* group, int* id )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_QUERY_LAST_SENSOR;
	
	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.query_sensor ), group, id );
}


