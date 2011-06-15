#ifndef _USER_TRAIN_H_
#define _USER_TRAIN_H_

/* task priorities */
#define TRAIN_MODULE_PRIORITY 5
#define TRAIN_CLOCK_PRIORITY 5
#define TRAIN_SENSOR_PRIORITY 5

/* constant */
#define TRAIN_MODULE_NAME "train_module"
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

#endif
