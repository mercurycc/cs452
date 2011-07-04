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
#include "inc/train_location.h"
#include "inc/warning.h"
#include "inc/train_types.h"


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
	int status;

	// get distance and time
	track_node* last_sensor = train->last_sensor;
	int distance = -1;
	uint time = 1;
	
	if ( train->last_sensor_time ) {
		distance = sensor_distance( last_sensor, next_sensor );
		/* If this time is used then we could have atmost 70 ms of time error which could results in 4 cm of location error */
		time = time_stamp - train->last_sensor_time;
	}
	
	if ( distance != -1 ) {
		// calculate new speed with avg
		uint level = train->speed_level;

		//WAR_PRINT( "old speed %d / %d\n", train->speed_table[level].numerator, train->speed_table[level].denominator );

		unsigned long long int bottom = train->speed_table[level].denominator * time * (train->speed_count[level] + 1);
		unsigned long long int top = distance * train->speed_table[level].denominator + train->speed_count[level] * time * train->speed_table[level].numerator;
		
		while( ( bottom > 10000 ) || ( top > 10000 ) ){
			top = top / 10;
			bottom = bottom / 10;
		}
		assert( bottom );
		train->speed_table[level].numerator = top;
		train->speed_table[level].denominator = bottom;
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

int node_distance( track_node* src, int* switch_table )
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
	int out[144];
	int queue[144]; // smallest to the end;
	int size;
	int max_dist;
	
	for ( i = 0; i < 144; i++ ) {
		dist[i] = INFINITY;
		prev[i] = -1;
		out[i] = 0;
		queue[i] = i;
	}
	size = 144;
	
	current_node = src;
	i = src->num;
	dist[i] = 0;
	// sort queue;
	for ( i = 1; i < size; i++ ) {
		if ( dist[queue[i]] > dist[queue[i-1]] ) {
			int temp = queue[i];
			queue[i] = queue[i-1];
			queue[i-1] = temp;
		}
	}
	
	while (1) {
		if (size == 0) return -1; // ERROR, should not happen
		
		index = queue[size-1];
		max_dist = dist[ index ];
		if ( index == dst->num ) {
			break;
		}
		
		assert( max_dist != INFINITY );
		out[index] = 1;
		size--;
		
		// for each neighbour update the distance
		int next;
		
		switch ( (track_graph + index)->type ) {
		case NODE_BRANCH:
			next = (track_graph + index)->edge[1].dest->num;
			if (( !out[next] )&&( dist[next] < dist[index] + (track_graph + index)->edge[1].dist )) {
				dist[next] = dist[index] + (track_graph + index)->edge[1].dist;
				prev[next] = index;
				for ( i = 1; i < size; i++ ) {
					if ( dist[queue[i]] > dist[queue[i-1]] ) {
						int temp = queue[i];
						queue[i] = queue[i-1];
						queue[i-1] = temp;
					}
				}
			}
		case NODE_SENSOR:
		case NODE_MERGE:
		case NODE_ENTER:
			next = (track_graph + index)->edge[0].dest->num;
			if (( !out[next] )&&( dist[next] < dist[index] + (track_graph + index)->edge[0].dist )) {
				dist[next] = dist[index] + (track_graph + index)->edge[0].dist;
				prev[next] = index;
				for ( i = 1; i < size; i++ ) {
					if ( dist[queue[i]] > dist[queue[i-1]] ) {
						int temp = queue[i];
						queue[i] = queue[i-1];
						queue[i-1] = temp;
					}
				}
			}
		default:
			// checking reverse comes to here
			break;
		}
		
	}
	
	index = dst->num;
	route->node[route->node_count] = track_graph + index;
	route->node_count++;
	
	
	while ( index != src->num ) {
		route->node[route->node_count] = track_graph + prev[index];
		route->node_count++;
		index = prev[index];
	}
	
	return 0;
}

/* find the expected target to hit by a train */
int train_detective( Train_data* train, track_node** next_sensor_ahead, track_node** next_sensor_skipped, track_node** next_sensor_curved ){
	// TODO
	return 0;
}

int train_next_sensor( Train_data* train, int* switch_table ){
	train->next_sensor = track_next_sensor( train->last_sensor, switch_table );
	if ( train->next_sensor ) return 0;
	return -1;
}

track_node* track_next_node( track_node* node, int* switch_table )
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

track_node* track_previous_node( track_node* node, int* switch_table )
{
	track_node* reverse_next = track_next_node( node->reverse, switch_table );
	
	return reverse_next ? reverse_next->reverse : 0;
}

track_node* track_next_sensor( track_node* sensor, int* switch_table ){
	track_node* ptr = sensor;
	int id;

	do {
		ptr = track_next_node( ptr, switch_table );
	} while ( ptr && ptr->type != NODE_SENSOR );

	return ptr;
}

int train_loc_is_sensor_tripped( Sensor_data* sensor_data, track_node* sensor )
{
	uint group;
	uint id;

	group = sensor->group * 2 + sensor->id / BITS_IN_BYTE;
	id = sensor->id % BITS_IN_BYTE;

	return ( ( sensor_data->sensor_raw[ group ] ) & ( 0x80 >> id ) );
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


int clear_sensor_data( Sensor_data* sensor_data, track_node* current_sensor ){
	int group = current_sensor->group * 2 + current_sensor->id / 8;
	int id = current_sensor->id % 8;

	sensor_data->sensor_raw[group] = sensor_data->sensor_raw[group] & ~(0x80 >> id);
	return 0;
}

int train_loc_dist( track_node* node, int* switch_table )
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
