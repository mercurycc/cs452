#ifndef _TRAIN_TYPES_H_
#define _TRAIN_TYPES_H_


#include <lib/rbuf.h>
#include "track_node.h"
#include "config.h"
#include "speed.h"

/* number of speed levels of train */
#define NUM_SPEED_LEVEL 30

enum Train_planner_direction_type {
	PLANNER_FORWARD,
	PLANNER_BACKWARD,
};

enum Train_state {
	TRAIN_STATE_INIT,             /* Init */
	TRAIN_STATE_STOP,             /* Stopped */
	TRAIN_STATE_TRACKING,         /* Normal state */
	TRAIN_STATE_REVERSE,          /* Just reversed direction */
	TRAIN_STATE_SPEED_CHANGE,     /* Just changed speed */
	TRAIN_STATE_SPEED_ERROR,      /* Speed prediction out of bound error */
	TRAIN_STATE_SWITCH_ERROR,     /* Switch prediction error */
	TRAIN_STATE_UNKNOW            /* Unknown state */
};

typedef struct Train_stat_s {
	uint total_dist;
	uint total_time;
	uint avg_speed;
} Train_stat;

typedef struct Train_path_s {
	track_node* node;
	int direction;
} Train_path;

typedef struct Train_data_s {
	uint id;
	uint state;
	uint pickup;                     /* Traveling with pick up at the front is forward, backward o/w */
	uint distance;                   /* Distance from last check point, in mm */
	uint remaining_distance;         /* Distance towards next check point, in mm */
	//uint speed_numerator;
	//uint speed_denominator;
	Speed speed;                     /* Speed currently at, in mm/10ms */
	uint speed_val;
	uint speed_level;
	uint old_speed_level;
	uint last_sensor_time;
	track_node* last_sensor;
	uint last_check_point_time;
	track_node* check_point;
	track_node* next_check_point;
	track_node* next_sensor;
	uint next_sensor_eta;            /* Estimated time of arrival to next sensor */
	uint last_eta_time_stamp;
	Speed speed_table[ NUM_SPEED_LEVEL ];
	uint speed_count[ NUM_SPEED_LEVEL ];
	uint speed_change_time_stamp;
	uint speed_distance;
	track_node* speed_mark;
	uint stop_distance;
	track_node* stop_sensor;
	track_node* stop_node;
	uint stop_length;

	/* Planner */
	uint auto_command;               /* Flag indicating if auto is controlling the train for, for instance, trip planning */
	int planner_tid;
	int planner_ready;               /* If not ready, don't wake up to avoid deadlock */
	int planner_stop;
	const track_node* planner_stop_node;
	int planner_stop_dist;
	const track_node* track_graph;
	const int* switch_table;
	const int* node_map;
} Train_data;
#endif
