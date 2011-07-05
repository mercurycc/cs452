#ifndef _TRAIN_ERROR_TOLERANCE_H_
#define _TRAIN_ERROR_TOLERANCE_H_

#include "train_location.h"
#include "train_types.h"


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

#endif

