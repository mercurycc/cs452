#ifndef _TRAIN_H_
#define _TRAIN_H_

#include <types.h>
#include <err.h>

enum TrainSpeed { TRAIN_SPEED_MIN = 0,
		  TRAIN_SPEED_MAX = 14,
		  TRAIN_SPEED_REVERSE = 15 };

/* Init and deinit*/
int train_init();
int train_deinit();

/* Command control */
int train_command_flushed();
int train_command_flush();

/* Train control */
int train_set_speed( uint train, uint speed );
int train_reverse_direction( uint train );
int train_stop( uint train );

/* Switch control */
uint train_switch_id_to_swi( uint id );
uint train_switch_swi_to_id( uint swi );
int train_switch_straight( uint swi );
int train_switch_curve( uint swi );
int train_switch_recent_switch( uint* swi );

/* Sensor control */
/* TODO */
int train_sensor_query_one( uint sensor, uchar* data );
/* return ERR_NONE when data is ready, ERR_NOT_READY o/w */
int train_sensor_query_all( uchar* data );

#endif
