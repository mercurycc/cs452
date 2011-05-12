#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <types.h>
#include <err.h>
#include <config.h>
#include <console.h>
#include <ui/ui.h>
#include <sensorctrl.h>

int ui_sensor_init( Console* cons );
int ui_sensor_update( uint group, uint id, uint state );
/* Sensorctrl can pass its data to sensor ui through this interface */
int ui_sensor_sensorctrl_update( uchar* data );
int ui_sensor_flush();

#endif
