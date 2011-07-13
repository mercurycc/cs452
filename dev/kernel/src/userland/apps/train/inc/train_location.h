#ifndef _TRAIN_LOCATION_H_
#define _TRAIN_LOCATION_H_

#include <types.h>
#include "train.h"
#include "config.h"
#include "sensor_data.h"
#include "track_data.h"
#include "track_node.h"
#include "train_types.h"


typedef struct Map_route_s {
	track_node* node[144];
	int node_count;
} Map_route;

typedef struct Map_node_s {
	track_node* node;
	int out;
	uint distance;
} Map_node;


/* how many historical data is kept for speed approximate */
#define NUM_SPEED_HISTORY 10
#define INIT_SPEED_COUNT 3

/* for path search */
#define INFINITY 2147483647





/* update train's location at current time */
int update_train_location( Train_data* train );
/* update train's speed (also location) when hit new sensor */
int update_train_speed( Train_data* train, track_node* next_sensor, uint time_stamp ); 

/* return the distance following the path from src to the next node */
int node_distance( const track_node* src, const int* switch_table );
/* return distance between two adjacent sesnors: has to going ahead from src to dst */
int sensor_distance( const track_node* src, const track_node* dst );

/* find next sensor on the track the train is going to hit */
int train_next_sensor( const Train_data* train, const int* switch_table );
/* Find next/previous node from a known node */
track_node* track_next_node( const track_node* node, const int* switch_table );
track_node* track_previous_node( const track_node* node, const int* switch_table );
/* find next sensor from a known sensor */
track_node* track_next_sensor( const track_node* sensor, const int* switch_table );
int track_next_sensor_distance( const track_node* ptr, const int* switch_table );

/* parse sensor data, find the triggered sensor after the given one */
track_node* parse_sensor( Sensor_data* sensor_data, track_node* current_sensor, track_node* track_graph, int node_map[ GROUP_COUNT ][ TRACK_GRAPH_NODES_PER_GROUP ] );
/* clear a specific sensor report in sensor_data */
int clear_sensor_data( Sensor_data* sensor_data, const track_node* current_sensor );
/* mark a specific sensor being triggered in sensor_data */
int mark_sensor_data( Sensor_data* sensor_data, const track_node* current_sensor );
/* Test if the requested sensor is tripped */
int train_loc_is_sensor_tripped( const Sensor_data* sensor_data, const track_node* sensor );
int train_loc_dist( const track_node* node, const int* switch_table );

/* find shortest route, and return the distance between 2 nodes on the track */
int track_route( track_node* src, track_node* dst, track_node* track_graph, Map_route* route );

/* find time to run the distance when speed is changing */
uint train_time_to_distance( Train_data* train, int distance );

#endif

