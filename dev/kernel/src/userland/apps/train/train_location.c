#include <types.h>
#include <err.h>
#include <lib/str.h>
#include <user/syscall.h>
#include <user/assert.h>
#include <user/time.h>
#include <user/name_server.h>
#include "inc/train.h"
#include "inc/config.h"
#include "inc/sensor_data.h"
#include "inc/track_data.h"
#include "inc/track_node.h"
#include "inc/train_location.h"
#include "inc/warning.h"
#include "inc/train_types.h"


int update_train_location( Train_data* train ) {

	// get current time
	uint cur_time = Time();
	
	// calculate distance
	uint time = cur_time - (train->last_sensor_time);
	uint distance = time * (train->speed_numerator) / (train->speed_denominator);
	
	// update train's distance
	train->distance = distance;
	
	return ERR_NONE;
}

int update_train_speed( Train_data* train, track_node* next_sensor, uint time_stamp ) {
	
	int status;

	// get distance and time
	track_node* last_sensor = train->last_sensor;
	uint distance = sensor_distance( last_sensor, next_sensor );

	
	// store old speed
	// TODO

	// calculate new speed with avg
	// TODO
	uint time = time_stamp - train->last_sensor_time;
	train->speed_numerator = distance;
	train->speed_denominator = time;

	// update location
	// assert( status == 0 );
	
	train->last_sensor = next_sensor;
	train->last_sensor_time = time_stamp;
	train->distance = 0;
	
	return 0;
}

static inline int ahead_to_sensor( track_node* src, track_node* dst ) {
	track_node* next = src->edge[DIR_AHEAD].dest;

	//WAR_PRINT( "ahead to sensor: %d-%d %d-%d   ", next->group, next->id , dst->group, dst->id );

	if ( next == dst ) {
		return src->edge[DIR_AHEAD].dist;
	}
	else if ( next->type == NODE_SENSOR ){
		return -1;
	}

	int distance = sensor_distance( next, dst );
	if ( distance == -1 ) {
		return -1;
	}

	return src->edge[DIR_AHEAD].dist + sensor_distance( next, dst );
}

static inline int branch_to_sensor( track_node* src, track_node* dst ) {

	track_node* next_straight = src->edge[DIR_STRAIGHT].dest;
	track_node* next_curve = src->edge[DIR_CURVED].dest;
	
	int dist[2];
	
	if ( next_straight == dst ) {
		return src->edge[DIR_STRAIGHT].dist;
	}
	if ( next_curve == dst ) {
		return src->edge[DIR_CURVED].dist;
	}
	if (( next_straight->type == NODE_SENSOR )&&( next_curve->type == NODE_SENSOR )) {
		return -1;
	}
	
	if ( next_straight->type != NODE_SENSOR ) {
		dist[DIR_STRAIGHT] = sensor_distance( next_straight, dst );
	}
	if ( next_curve->type != NODE_SENSOR ) {
		dist[DIR_CURVED] = sensor_distance( next_curve, dst );
	}
	
	if ( dist[DIR_STRAIGHT] == -1 ) {
		return src->edge[DIR_CURVED].dist + dist[DIR_CURVED];
	}
	else {
		return src->edge[DIR_STRAIGHT].dist + dist[DIR_STRAIGHT];
	}
}

int sensor_distance( track_node* src, track_node* dst ){

	if ( src == dst ) {
		return 0;
	}
	if ( src->reverse == dst ) {
		return 0;
	}

	/*
	WAR_PRINT( "distance: %d:%d-%d %d:%d-%d next: %d:%d-%d   ", src->num,src->group, src->id , dst->num,dst->group, dst->id, src->edge[0].dest->num, src->edge[0].dest->group, src->edge[0].dest->id );
	return 0;
	*/
	
	switch ( src->type ){
	case NODE_SENSOR:
	case NODE_MERGE:
	case NODE_ENTER:
		return ahead_to_sensor( src, dst );
		break;
	case NODE_BRANCH:
		return branch_to_sensor( src, dst );
		break;
	default:
		return -1;
		break;
	}
	
	return 0;
}

int track_route( track_node* src, track_node* dst, track_node* routine ){
	// TODO
	return 0;
}

/* find the expected target to hit by a train */
int train_detective( Train_data* train, track_node** next_sensor_ahead, track_node** next_sensor_skipped, track_node** next_sensor_curved ){
	// TODO
	return 0;
}

