#ifndef _TRAIN_SENSOR_DATA_H_
#define _TRAIN_SENSOR_DATA_H_
#include <types.h>
#include "config.h"

typedef struct Sensor_data_s Sensor_data;
struct Sensor_data_s {
	uchar sensor_raw[ SENSOR_BYTE_COUNT ];
	char last_sensor_group;
	char last_sensor_id;
	uint last_sensor_time;
};

#endif /* _TRAIN_SENSOR_DATA_H_ */
