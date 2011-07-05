#ifndef _TRAIN_ERROR_TOLERANCE_H_
#define _TRAIN_ERROR_TOLERANCE_H_

#define SENSOR_NOT_TRUSTABLE 3

/* check if a sensor is trustable
 * trustable returns true if sensor is trustable
 * sensor_error makes a sensor less trustable
 * sensor_trust makes a sensor more trustable
 */
int sensor_trustable( track_node* sensor );
int sensor_error( track_node* sensor );
int sensor_trust( track_node* sensor );

/* find possible sensors
 * primary is next expected sensor
 * secondary is the expected sensor if primary is malfunctioned
 * tertiary is the expected sensor if the switch is malfunctioned
 */
int sensor_find_possible( track_node* sensor, int* switch_table, track_node** primary, track_node** secondary, track_node** tertiary );

#endif

