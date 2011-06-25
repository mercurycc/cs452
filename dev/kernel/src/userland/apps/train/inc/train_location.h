#ifndef _TRAIN_LOCATION_H_
#define _TRAIN_LOCATION_H_

#include <types.h>
#include "train.h"
#include "config.h"
#include "sensor_data.h"
#include "track_data.h"
#include "track_node.h"
#include "train_types.h"

/* update train's location at current time */
int update_train_location( Train_data* train );
/* update train's speed (also location) when hit new sensor */
int update_train_speed( Train_data* train, track_node* next_sensor, uint time_stamp ); 

/* returns distance between 2 nodes on the track */
int track_distance( track_node* src, track_node* dst );

/* find the expected target to hit by a train */
int train_detective( Train_data* train, track_node** next_sensor_ahead, track_node** next_sensor_skipped, track_node** next_sensor_curved );

#endif

