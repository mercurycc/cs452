#ifndef _USER_TRAIN_H_
#define _USER_TRAIN_H_

#include "sensor_data.h"

/* task priorities : high - low */
#define TRAIN_SENSOR_PRIORITY   6
#define TRAIN_MODULE_PRIORITY   6
#define TRAIN_AUTO_PRIROTY      6
#define TRAIN_UI_PRIORITY       7

/* train event api */
/* Server */
void train_module();
int train_set_speed( int tid, int train, int speed );
int train_reverse( int tid, int train );
int train_switch( int tid, int switch_id, int direction );
int train_check_switch( int tid, int switch_id );
int train_last_sensor( int tid );
int train_all_sensor( int tid );
int train_module_suicide( int tid );
int train_pressure_test( int tid );
int train_switch_all( int tid, int d );

/* switch event api */
int switch_update_id( int id, char direction );
int switch_update_all( char direction );
int switch_check_id( int id );

/* sensor api */
/* Server */
void train_sensor();

/* ui components */
void sensor_ui();
int sensor_ui_update( int tid, Sensor_data* data );
void clock_ui();

#endif
