#ifndef _SENSORCTRL_H_
#define _SENSORCTRL_H_

#include <types.h>
#include <err.h>
#include <config.h>
#include <train.h>

#define HIGH_BIT_MASK  1 << 7

enum SensorGroup {
	SENSOR_GROUP_A = 0,
	SENSOR_GROUP_B = 1,
	SENSOR_GROUP_C = 2,
	SENSOR_GROUP_D = 3,
	SENSOR_GROUP_E = 4
};

enum SensorState {
	SENSOR_TRIGGERED,
	SENSOR_UNTRIGGERED
};

int sensorctrl_init();
int sensorctrl_update();
int sensorctrl_most_recent( uint* group, uint* id );

#endif /* _SENSORCTRL_H_ */
