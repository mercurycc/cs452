#include <types.h>
#include <err.h>
#include <lib/str.h>
#include <user/syscall.h>
#include <user/assert.h>
#include <user/time.h>
#include <user/name_server.h>
#include "inc/train.h"
#include "inc/config.h"
#include "inc/sensor_data.h"
#include "inc/track_data.h"
#include "inc/track_node.h"
#include "inc/train_location.h"
#include "inc/warning.h"
#include "inc/train_types.h"


int update_train_location( Train_data* train ) {

	// get current time
	uint cur_time = Time();
	
	// calculate distance
	uint time = cur_time - (train->last_sensor_time);
	uint distance = time * (train->speed_numerator) / (train->speed_denominator);
	
	// update train's distance
	train->distance = distance;
	
	return ERR_NONE;
}

int update_train_speed( Train_data* train, Sensor_data* sensor_data ) {
	
	int status;

	// TODO: check?
	//WAR_PRINT( "sensor: %c %c; train: %s", sensor_data->last_sensor_group, sensor_data->last_sensor_id, train->last_sensor->edge[DIR_AHEAD].dest->name );
	WAR_PRINT( "sensor: %d %d, train: %s", sensor_data->last_sensor_group, sensor_data->last_sensor_id, train->last_sensor->edge[DIR_AHEAD].dest->name );
	
	if ( train == 0 ) {
		return 0;
	}

	// get current time
	uint time_stamp = sensor_data->last_sensor_time;
	
	// get distance and time
	track_node* last_node = train->last_sensor;
	track_node* next_node = last_node;
	uint distance = 0;
	do {
		distance += next_node->edge[DIR_AHEAD].dist;
		next_node = last_node->edge[DIR_AHEAD].dest;
	} while ( next_node->type == NODE_SENSOR );
	
	uint time = time_stamp - train->last_sensor_time;

	// store old speed
	// TODO

	// calculate new speed with avg
	// TODO
	train->speed_numerator = distance;
	train->speed_denominator = time;

	// update location
	assert( status == 0 );
	
	train->last_sensor = next_node;
	train->last_sensor_time = time_stamp;
	train->distance = 0;
	
	return ERR_NONE;
}



