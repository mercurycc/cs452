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
#define NUM_SPEED_HISTORY 5

/* for path search */
#define INFINITY 2147483647

/* number of ticks needed to change speed */
#define SPEED_CHANGE_TIME 250

/* 999 means not measured yet */
#define TRAIN_HEAD_LENGTH( i ) ( i > 21 ? ( i == 23 ? 30 : 999 ) : ( i == 21 ? 20 : 999 ) )
#define TRAIN_TAIL_LENGTH( i ) ( i > 21 ? ( i == 23 ? 140 : 999 ) : ( i == 21 ? 120 : 999 ) )

#define TRAIN_SPEED_PRED_FACTOR( i ) ( i > 21 ? ( i == 23 ? 55 : 0 ) : ( i == 21 ? 11 : 0 ) )



/* update train's location at current time */
int update_train_location( Train_data* train );
/* update train's speed (also location) when hit new sensor */
int update_train_speed( Train_data* train, track_node* next_sensor, uint time_stamp ); 

/* return the distance following the path from src to the next node */
int node_distance( track_node* src, int* switch_table );
/* return distance between two adjacent sesnors: has to going ahead from src to dst */
int sensor_distance( track_node* src, track_node* dst );

/* find next sensor on the track the train is going to hit */
int train_next_sensor( Train_data* train, int* switch_table );
/* Find next/previous node from a known node */
track_node* track_next_node( track_node* node, int* switch_table );
track_node* track_previous_node( track_node* node, int* switch_table );
/* find next sensor from a known sensor */
track_node* track_next_sensor( track_node* sensor, int* switch_table );

/* parse sensor data, find the triggered sensor after the given one */
track_node* parse_sensor( Sensor_data* sensor_data, track_node* current_sensor, track_node* track_graph, int node_map[ GROUP_COUNT ][ TRACK_GRAPH_NODES_PER_GROUP ] );
/* clear a specific sensor report in sensor_data */
int clear_sensor_data( Sensor_data* sensor_data, track_node* current_sensor );
/* Test if the requested sensor is tripped */
int train_loc_is_sensor_tripped( Sensor_data* sensor_data, track_node* sensor );

/* find shortest route, and return the distance between 2 nodes on the track */
int track_route( track_node* src, track_node* dst, track_node* track_graph, Map_route* route );



/* find the expected target to hit by a train */
int train_detective( Train_data* train, track_node** next_sensor_ahead, track_node** next_sensor_skipped, track_node** next_sensor_curved );

#endif

