#include <types.h>
#include <err.h>
#include <lib/str.h>
#include <user/syscall.h>
#include <user/assert.h>
#include <user/time.h>
#include <user/name_server.h>
#include <user/lib/math.h>
#include <config.h>
#include "inc/train.h"
#include "inc/config.h"
#include "inc/sensor_data.h"
#include "inc/track_data.h"
#include "inc/track_node.h"
#include "inc/train_location.h"
#include "inc/warning.h"
#include "inc/train_types.h"
#include "inc/error_tolerance.h"

#define LOCAL_DEBUG
#include <user/dprint.h>

int node_distance( const track_node* src, const int* switch_table )
{
	int swid;
	int length;
	
	switch( src->type ){
	case NODE_SENSOR:
	case NODE_MERGE:
	case NODE_ENTER:
		length = src->edge[ DIR_AHEAD ].dist;
		break;
	case NODE_BRANCH:
		swid = SWID_TO_ARRAYID( src->id + 1 );
		if( switch_table[ swid ] == 'C' ){
			length = src->edge[ DIR_CURVED ].dist;
		} else {
			length = src->edge[ DIR_STRAIGHT ].dist;
		}
		break;
	default:
		return -1;
		break;
	}

	return length;
}

int track_next_sensor_distance( const track_node* ptr, const int* switch_table )
{
	int length = 0;

	do {
		if( ptr->type == NODE_BRANCH && switch_table[ SWID_TO_ARRAYID( ptr->id + 1 ) ] == 'C' ){
			length += ptr->edge[ DIR_CURVED ].dist;
		} else {
			length += ptr->edge[ DIR_AHEAD ].dist;
		}
		ptr = track_next_node( ptr, switch_table );
	} while ( ptr && ( ptr->type != NODE_SENSOR || !sensor_trustable( ptr ) ) );

	return length;
}

track_node* track_next_node( const track_node* node, const int* switch_table )
{
	int id;
	
	switch ( node->type ) {
	case NODE_SENSOR:
	case NODE_MERGE:
	case NODE_ENTER:
		node = node->edge[DIR_AHEAD].dest;
		break;
	case NODE_BRANCH:
		id = SWID_TO_ARRAYID( node->id + 1 );
		assert( id >= 0 );
		assert( id < 22 );
		if ( switch_table[id] == 'S' ) {
			node = node->edge[DIR_STRAIGHT].dest;
		} else {
			node = node->edge[DIR_CURVED].dest;
		}
		break;
	default:
		return 0;
		break;
	}

	return node;
}

track_node* track_previous_node( const track_node* node, const int* switch_table )
{
	track_node* reverse_next = track_next_node( node->reverse, switch_table );
	
	return reverse_next ? reverse_next->reverse : 0;
}

track_node* track_next_sensor( const track_node* sensor, const int* switch_table ){
	track_node* ptr = sensor;

	do {
		ptr = track_next_node( ptr, switch_table );
	} while ( ptr && ptr->type != NODE_SENSOR );

	return ptr;
}

int train_loc_is_sensor_tripped( const Sensor_data* sensor_data, const track_node* sensor )
{
	uint group;
	uint id;

	group = sensor->group * 2 + sensor->id / BITS_IN_BYTE;
	id = sensor->id % BITS_IN_BYTE;

	return ( ( sensor_data->sensor_raw[ group ] ) & ( 0x80 >> id ) );
}

int clear_sensor_data( Sensor_data* sensor_data, const track_node* current_sensor ){
	int group = current_sensor->group * 2 + current_sensor->id / 8;
	int id = current_sensor->id % 8;

	sensor_data->sensor_raw[group] = sensor_data->sensor_raw[group] & ~(0x80 >> id);
	return 0;
}

int mark_sensor_data( Sensor_data* sensor_data, const track_node* current_sensor ){
	int group = current_sensor->group * 2 + current_sensor->id / 8;
	int id = current_sensor->id % 8;

	sensor_data->sensor_raw[group] = sensor_data->sensor_raw[group] | (0x80 >> id);
	return 0;

}

int train_loc_dist( const track_node* node, const int* switch_table )
{
	switch( node->type ){
	case NODE_BRANCH:
		if( switch_table[ SWID_TO_ARRAYID( node->id + 1 ) ] == 'C' ){
			return node->edge[ DIR_CURVED ].dist;
		}
	case NODE_SENSOR:
	case NODE_MERGE:
	case NODE_ENTER:
		return node->edge[ DIR_AHEAD ].dist;
	default:
		return 0;
	}
}

uint train_time_to_distance( Train_data* train, int distance ) {

	assert( train->state == TRAIN_STATE_SPEED_CHANGE );

	float speed;
	float accel;
	uint time;
	uint result = 0;
	int change_dist = ( train->tracking.speed_change_end_time - train->tracking.speed_change_start_time ) * ( ( train->tracking.speed_stat_table[ train->tracking.speed_level ] + train->tracking.old_speed ) / 2 );
	
	if ( change_dist <= distance ) {
		time = ( distance - change_dist ) / train->tracking.speed_stat_table[ train->tracking.speed_level ];
		result = time + train->tracking.speed_change_end_time - train->tracking.speed_change_start_time;
	}
	else {
		accel = ( train->tracking.speed_stat_table[ train->tracking.speed_level ] - train->tracking.old_speed ) / ( train->tracking.speed_change_end_time - train->tracking.speed_change_start_time );
		speed = fsqrt( train->tracking.speed * train->tracking.speed + 2 * accel * distance );
		time = ( speed - train->tracking.speed ) / accel;
		result = time;
	}
	
	
	//dprintf( "time to get to %c%d distance %d is time %d\n", train->next_sensor->group+'A', train->next_sensor->id+1, distance, time );
	return result;
}

#if 0
int update_train_location( Train_data* train ) {

	// get current time
	uint cur_time = Time();
	
	// calculate distance
	uint time = cur_time - (train->last_check_point_time);
	uint distance = time * (train->speed.numerator) / (train->speed.denominator);
	
	// update train's distance
	train->distance = distance;
	
	return ERR_NONE;
}

int update_train_speed( Train_data* train, track_node* next_sensor, uint time_stamp )
{
	// get distance and time
	track_node* last_sensor = train->last_sensor;
	int distance = -1;
	uint time = 1;
	
	if ( train->last_sensor_time ) {
		distance = sensor_distance( last_sensor, next_sensor );
		time = time_stamp - train->last_sensor_time;
	}
	
	if ( distance != -1 ) {
		// calculate new speed with avg
		uint level = train->speed_level;

		Speed current_speed;
		Speed temp;
		
		current_speed.numerator = distance;
		current_speed.denominator = time;
		
		get_inter_speed( &current_speed, train->speed_table + level, train->speed_table + level, train->speed_count[level], train->speed_count[level] + 1 );
		
		/*
		unsigned long long int bottom = train->speed_table[level].denominator * time * (train->speed_count[level] + 1);
		unsigned long long int top = distance * train->speed_table[level].denominator + train->speed_count[level] * time * train->speed_table[level].numerator;
		
		while( ( bottom > 10000 ) || ( top > 10000 ) ){
			top = top / 10;
			bottom = bottom / 10;
		}
		assert( bottom );
		train->speed_table[level].numerator = top;
		train->speed_table[level].denominator = bottom;
		*/
		
		if ( train->speed_count[level] < NUM_SPEED_HISTORY ) {
			train->speed_count[level] += 1;
		}

		train->speed.numerator = train->speed_table[level].numerator;
		train->speed.denominator = train->speed_table[level].denominator;

		train->speed_val = train->speed.numerator / train->speed.denominator;
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

track_node* parse_sensor( Sensor_data* sensor_data, track_node* current_sensor, track_node* track_graph, int node_map[][ TRACK_GRAPH_NODES_PER_GROUP ] )
{
	int group_start = 0;
	int id_start = -1;
	int i, j;
	int group, id;

	if ( current_sensor ) {
		group_start = current_sensor->group * 2 + current_sensor->id / 8;
		id_start = current_sensor->id % 8;
	}

	//WAR_PRINT( "sensor begin: %d-%d\n", group_start, id_start );
	for ( i = id_start + 1; i < BITS_IN_BYTE; i++ ) {
		if ( sensor_data->sensor_raw[group_start] & ( 0x80 >> i ) ) {
			group = group_start / 2;
			id = i + group_start % 2 * 8;
			//WAR_PRINT( "sensor top: %d-%d: %c%d\n", group_start, i, group+'A', id+1 );
			return track_graph + node_map[group][id];
		}
	}

	for ( i = group_start + 1; i < SENSOR_BYTE_COUNT; i++ ) {
		if ( sensor_data->sensor_raw[i] ) {
			for ( j = 0; j < BITS_IN_BYTE; j++ ) {
				if ( sensor_data->sensor_raw[i] & ( 0x80 >> j ) ) {
					int group = i/2;
					int id = j+i%2*8;
					//WAR_PRINT( "sensor bottom: %d-%d: %c%d\n", i, j, group+'A', id+1 );
					return track_graph + node_map[group][id];
				}
			}
		}
	}

	return 0;
}

#endif /* if 0 */
