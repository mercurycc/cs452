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
int update_train_speed( Train_data* train, Sensor_data* sensor_data );


#endif

