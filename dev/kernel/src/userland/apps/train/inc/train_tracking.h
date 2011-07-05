#ifndef _TRAIN_TRACKING_H_
#define _TRAIN_TRACKING_H_

#include "train_types.h"

/* Init */
int train_tracking_init( Train* train );
int train_tracking_init_calib( Train* train );

/* Instruction */
/* Train tracking does not have permission for charging a sensor as a
   land mark.  All sensor hits must be reported by train_auto */
int train_tracking_update( Train* train, int curtime );

/* Informing */
int train_tracking_new_sensor( Train* train, int sensor_time, int curtime );
int train_tracking_reverse( Train* train );
int train_tracking_speed_change( Train* train, int new_speed_level, int curtime );

/* Conversion */
int train_tracking_trav_dist( const Train* train, int ticks );
int train_tracking_trav_time( const Train* train, int dist );

/* Reporting */
int train_tracking_speed_in_sec( const Train* train );
int train_tracking_position( const Train* train );
int train_tracking_remaining_distance( const Train* train );
int train_tracking_eta( const Train* train );
int train_tracking_stop_distance( const Train* train );

#endif /* _TRAIN_TRACKING_H_ */
