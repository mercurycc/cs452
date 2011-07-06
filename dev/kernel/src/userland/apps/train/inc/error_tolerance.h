#ifndef _TRAIN_ERROR_TOLERANCE_H_
#define _TRAIN_ERROR_TOLERANCE_H_

#include "train_location.h"
#include "train_types.h"

/* constants */
#define SENSOR_NOT_TRUSTABLE 3
#define ETA_WINDOW_SIZE 100


/* check if a sensor is trustable
 * trustable returns true if sensor is trustable
 * sensor_error makes a sensor less trustable
 * sensor_trust makes a sensor more trustable
 */
int sensor_trustable( track_node* sensor );
int sensor_error( track_node* sensor );
int sensor_trust( track_node* sensor );

/* check if train get to an untrustable sensor */
int train_hit_untrustable( Train_data* train );

/* find possible sensors for train */
int train_next_possible( Train_data* train, int* switch_table );

/* mark if a sensor is being expected by a train
 * mark single ones or
 * mark several of a train
 */
int expect_sensor( int sensor_expect[ SENSOR_GROUP_COUNT ][ SENSOR_COUNT_PER_GROUP ], track_node* current_sensor );
int forget_sensor( int sensor_expect[ SENSOR_GROUP_COUNT ][ SENSOR_COUNT_PER_GROUP ], track_node* current_sensor );
int train_expect_sensors( Train_data* train, int sensor_expect[ SENSOR_GROUP_COUNT ][ SENSOR_COUNT_PER_GROUP ] );
int train_forget_sensors( Train_data* train, int sensor_expect[ SENSOR_GROUP_COUNT ][ SENSOR_COUNT_PER_GROUP ] );


#endif

