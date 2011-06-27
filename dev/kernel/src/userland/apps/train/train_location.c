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
	uint distance = time * (train->speed.numerator) / (train->speed.denominator);
	
	// update train's distance
	train->distance = distance;
	
	return ERR_NONE;
}

int update_train_speed( Train_data* train, track_node* next_sensor, uint time_stamp ) {
	
	int status;

	// get distance and time
	track_node* last_sensor = train->last_sensor;
	int distance = sensor_distance( last_sensor, next_sensor );
	uint time = time_stamp - train->last_sensor_time;
	
	if (distance != -1) {

		// calculate new speed with avg
		uint level = train->speed_level - 1;
		
		unsigned long long int bottom = train->speed_table[level].denominator * time * (train->speed_count[level] + 1);
		unsigned long long int top = distance * train->speed_table[level].denominator + train->speed_count[level] * time * train->speed_table[level].numerator;
		
		while (( bottom > 10000 )&&( top > 10000 )) {
			top = top / 10;
			bottom = bottom / 10;
		}
		
		train->speed_table[level].numerator = top;
		train->speed_table[level].denominator = bottom;
		if ( train->speed_count[level] < NUM_SPEED_HISTORY ) {
			train->speed_count[level] += 1;
		}

		train->speed.numerator = train->speed_table[level].numerator;
		train->speed.denominator = train->speed_table[level].denominator;

	}
	// if distance == 1 no update of speed
	
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
	track_node* next_curved = src->edge[DIR_CURVED].dest;
	
	int dist;
	
	//WAR_PRINT( "straight dist: %d    ", src->edge[DIR_STRAIGHT].dist );
	
	if ( next_straight == dst ) {
		return src->edge[DIR_STRAIGHT].dist;
	}
	
	if ( next_curved == dst ) {
		return src->edge[DIR_CURVED].dist;
	}
	
	if (( next_straight->type == NODE_SENSOR )&&( next_curved->type == NODE_SENSOR )) {
		return -1;
	}
	
	if ( next_straight->type != NODE_SENSOR ) {
		dist = sensor_distance( next_straight, dst );
		if ( dist != -1 ) {
			return dist + src->edge[DIR_STRAIGHT].dist;
		}
	}
	
	dist = sensor_distance( next_curved, dst );
	if ( dist != -1 ) {
		return dist + src->edge[DIR_CURVED].dist;
	}
	
	return -1;
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

int track_route( track_node* src, track_node* dst, track_node* track_graph, Map_route* route ){
	// TODO
	/*
	assert( route->node_count == 0 );
	
	if (src == dst) {
		route->node[route->node_count] = src;
		route->node_count++;
		return 0;
	}
	
	int i;
	int index;
	track_node* current_node;
	int dist[144]; // distance
	int prev[144]; // previous node visited ( shortest path calculated from )
	int queue[144]; // smallest to the end;
	int size;
	int max_dist;
	
	for ( i = 0; i < 144; i++ ) {
		dist[144] = INFINITY;
		prev[v] = -1;
		queue[i] = i;
	}
	size = 144;
	
	current_node = src;
	i = src->num;
	dist[i] = 0;
	// sort queue;
	
	while (1) {
		if (size == 0) return -1; // ERROR, should not happen
		
		index = queue[size-1];
		max_dist = dist[ index ];
		
		assert( max_dist != INFINITY );
		size--;
		
		// for each neighbour update the distance
		
		
	}
	
	index = dst->num;
	route->node[route->node_count] = track_graph + index;
	route->node_count++;
	
	
	while ( index != src->num ) {
		route->node[route->node_count] = track_graph + prev[index];
		route->node_count++;
		index = prev[index];
	}
	
	*/
	return 0;
}

/* find the expected target to hit by a train */
int train_detective( Train_data* train, track_node** next_sensor_ahead, track_node** next_sensor_skipped, track_node** next_sensor_curved ){
	// TODO
	return 0;
}

