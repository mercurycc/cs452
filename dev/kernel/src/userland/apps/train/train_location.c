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

int update_train_speed( Train_data* train, track_node* next_sensor, uint time_stamp ) {
	
	int status;

	// get distance and time
	track_node* last_sensor = train->last_sensor;
	uint distance = track_distance( last_sensor, next_sensor );
	
	// store old speed
	// TODO

	// calculate new speed with avg
	// TODO
	uint time = time_stamp - train->last_sensor_time;
	train->speed_numerator = distance;
	train->speed_denominator = time;

	// update location
	// assert( status == 0 );
	
	train->last_sensor = next_sensor;
	train->last_sensor_time = time_stamp;
	train->distance = 0;
	
	return 0;
}


int track_distance( track_node* src, track_node* dst ){
	
	return 0;
}

/* find the expected target to hit by a train */
int train_detective( Train_data* train, track_node** next_sensor_ahead, track_node** next_sensor_skipped, track_node** next_sensor_curved ){
	// TODO
	return 0;
}

