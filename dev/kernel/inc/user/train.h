#ifndef _USER_TRAIN_H_
#define _USER_TRAIN_H_

/* task priorities : high - low */
#define TRAIN_SENSOR_PRIORITY 5
#define TRAIN_CONTROL_PRIORITY 5
#define TRAIN_MODULE_PRIORITY 5
#define TRAIN_CLOCK_PRIORITY 6
#define TRAIN_SWITCH_PRIORITY 6

/* constant */
#define TRAIN_MODULE_NAME "train_module"
#define SWITCH_XMIT_DELAY 13
#define SWITCH_OFF_DELAY 20
#define TRAIN_XMIT_DELAY 20


/* train event api */
int train_update_time( int ticks );
int train_set_speed( int train, int speed );
int train_reverse( int train );
int train_switch( int switch_id, int direction );
int train_check_switch( int switch_id );
int train_last_sensor();
int train_all_sensor();
int train_module_suicide();
int train_pressure_test();
int train_switch_all( int d );

/* switch event api */
int switch_update_id( int id, char direction );
int switch_update_all( char direction );
int switch_check_id( int id );

/* sensor api */
int sensor_last( int tid );

#endif
