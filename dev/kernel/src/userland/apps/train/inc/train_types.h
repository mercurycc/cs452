#ifndef _TRAIN_TYPES_H_
#define _TRAIN_TYPES_H_


#include <lib/rbuf.h>
#include <user/semaphore.h>
#include "track_node.h"
#include "config.h"

/* number of speed levels of train */
#define NUM_SPEED_LEVEL 30

enum Train_planner_direction_type {
	PLANNER_FORWARD,
	PLANNER_BACKWARD,
};

enum Train_state {
	TRAIN_STATE_INIT_1,           /* Init for first sensor hit */
	TRAIN_STATE_INIT_2,           /* Init for second sensor hit */
	TRAIN_STATE_INIT_3,           /* Init for third sensor hit */
	TRAIN_STATE_INIT_4,           /* Init for first speed end, second speed start */
	TRAIN_STATE_INIT_5,           /* Init for second speed end, tracking start */
	TRAIN_STATE_INIT_DONE,
	TRAIN_STATE_STOP,             /* Stopped */
	TRAIN_STATE_TRACKING,         /* Normal state */
	TRAIN_STATE_REVERSE,          /* Just reversed direction */
	TRAIN_STATE_SPEED_CHANGE,     /* Just changed speed */
	TRAIN_STATE_SPEED_ERROR,      /* Speed prediction out of bound error */
	TRAIN_STATE_SWITCH_ERROR,     /* Switch prediction error */
	TRAIN_STATE_UNKNOW            /* Unknown state */
};

typedef struct Train_speed_s {
	int stat_dist;
	uint stat_time;
} Train_speed;

typedef struct Train_stat_s {
	uint total_dist;
	uint total_time;
	uint avg_speed;
} Train_stat;

typedef struct Train_path_s {
	const track_node* node;
	int direction;
} Train_path;

typedef struct Train_tracking_s {
	float speed;                     /* Speed currently at */
	float old_speed;                 /* Last speed */
	int old_speed_level;            /* For acc/deceleration */
	int speed_level;                /* Current speed level */
	int distance;                    /* Distance from last check point, in mm */
	int remaining_distance;          /* Distance towards next check point, in mm */
	int trav_distance;              /* Distance between current and next sensor */
	int trav_time_stamp;
	float speed_stat_table[ NUM_SPEED_LEVEL ];
	int speed_stat_count[ NUM_SPEED_LEVEL ];
	int speed_change_start_time;
	int speed_change_last_integration;
	int speed_change_end_time;
	int check_point_time;
	int eta;                        /* Estimated time of arrival to next check point */
} Train_tracking;

typedef struct Train_planner_s {
	uint auto_command;               /* Flag indicating if auto is controlling the train for, for instance, trip planning */
	int planner_stop_dist;
} Train_planner;

typedef struct Train_data_s {
	uint id;
	uint state;

	/* Init */
	uint init_state;
	uint init_retry;
	uint init_speed_timeout;

	uint pickup;                     /* Traveling with pick up at the front is forward, backward o/w */
	track_node* last_sensor;         /* Sensor */
	track_node* next_sensor;
	track_node* check_point;         /* Shared.  Check point */
	track_node* next_check_point;    /* Shared.  Next check point */
	
	/* error tolerance variables */
	track_node* secondary_sensor;    /* Sensor for error tolerance */
	track_node* tertiary_sensor;
	uint next_time_pred;             /* expected arrive time for sensors */
	uint secondary_time_pred;
	uint tertiary_time_pred;
	uint next_time_range;
	uint secondary_time_range;
	uint tertiary_time_range;
	uint tertiary_distance;			 /* so we can update speed when tertiary is hit */
	int going_to_secondary;

	/* Shared data lock */
	Semaphore sem_body;
	Semaphore* sem;

	/* Update event notifier, only one task can be waiting on this */
	Semaphore update_body;
	Semaphore* update;
	
	const track_node* track_graph;
	const int* switch_table;
	const int* node_map;

	volatile Train_tracking tracking; /* Shared. */

	/* Planner */
	int planner_tid;                 /* Planner */
	volatile int planner_control;
	volatile int mark_dist;
} Train_data;

typedef Train_data Train;
#endif
