#ifndef _USER_TRAIN_H_
#define _USER_TRAIN_H_

#include "sensor_data.h"

/* task priorities : high - low */
#define TRAIN_SENSOR_PRIORITY   6
#define TRAIN_MODULE_PRIORITY   7
#define TRAIN_AUTO_PRIROTY      7
#define TRAIN_UI_PRIORITY       8

/* train event api */
/* Server */
void train_module();
int train_set_speed( int tid, int train, int speed );
int train_reverse( int tid, int train );
int train_check_switch( int tid, int switch_id );
int train_switch( int tid, int switch_id, int direction );
int train_all_sensor( int tid );
int train_module_suicide( int tid );
int train_pressure_test( int tid );
int train_switch_all( int tid, int d );

/* Automation */
/* Server */
void train_auto();
/* API */
enum Track_id {
	TRACK_A,
	TRACK_B
};

int train_auto_new_sensor_data( int tid, Sensor_data* data );
int train_auto_init( int tid, uint track );
int train_auto_new_train( int tid, uint id, uint pre_grp, uint pre_id );
int train_auto_set_speed( int tid, uint id, uint speed_level );
int train_auto_set_reverse( int tid, uint id );
int train_auto_set_switch( int tid, uint id, char direction );
int train_auto_set_switch_all( int tid, char direction );
int train_auto_query_switch( int tid, uint id, int* direction );
/* Return group < 0 if no sensor is triggered */
int train_auto_query_sensor( int tid, int* group, int* id );


/* switch event api */
int switch_ui_update_id( int tid, int id, char direction );
int switch_ui_update_all( int tid, char direction );
int switch_ui_check_id( int tid, int id );

/* sensor api */
/* Server */
void train_sensor();
/* group < 0 if no sensor has been triggered yet */
int sensor_query_recent( int tid, int* group, int* id );
int sensor_id2name( char* str, uchar group, uchar id );
int sensor_name2id( char* str, int* group, int* id );

/* ui components */
void sensor_ui();
int sensor_ui_update( int tid, Sensor_data* data );
void clock_ui();
void switches_ui();

#endif
