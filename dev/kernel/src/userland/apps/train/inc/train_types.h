#ifndef _TRAIN_TYPES_H_
#define _TRAIN_TYPES_H_

#include "track_node.h"

typedef struct Train_stat_s {
	uint total_dist;
	uint total_time;
	uint avg_speed;
} Train_stat;

typedef struct Speed_s {
	uint numerator;
	uint denominator;
} Speed;

typedef struct Train_data_s {
	uint id;
	uint state;
	uint pickup;                     /* Traveling with pick up at the front is forward, backward o/w */
	uint distance;                   /* Distance from last check point, in mm */
	uint remaining_distance;         /* Distance towards next check point, in mm */
	//uint speed_numerator;
	//uint speed_denominator;
	Speed speed;                     /* Speed currently at, in mm/10ms */
	uint speed_level;
	uint old_speed_level;
	uint last_sensor_time;
	track_node* last_sensor;
	uint last_check_point_time;
	track_node* check_point;
	track_node* next_sensor;
	uint next_sensor_eta;            /* Estimated time of arrival to next sensor */
	Speed speed_table[14];
	uint speed_count[14];
	
} Train_data;



#endif

