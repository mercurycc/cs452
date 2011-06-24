#include <types.h>
#include <err.h>
#include <lib/str.h>
#include <user/syscall.h>
#include <user/lib/assert.h>
#include <user/time.h>
#include <user/name_server.h>
#include "inc/train.h"
#include "inc/config.h"
#include "inc/sensor_data.h"
#include "inc/track_data.h"
#include "inc/track_node.h"
#include "inc/train_location.h"


int update_train_location( Train_data *train ) {

	// get current time
	uint cur_time = Time();
	
	// calculate distance
	uint time = cur_time - train->last_sensor_time;
	uint distance = time * train->speed;
	
	// update train's distance
	train->distance = distance;
	
	return ERR_NONE;
}

int update_train_speed( Train_data* train, track_node* new_sensor, track_node* track ) {

	// TODO: check?

	// get current time
	uint cur_time = Time();
	
	// get distance and time
	track_edge* last_path = find_edge( train->last_sensor, new_sensor );
	uint distance = last_path->dist;
	uint time = cur_time - train->last_sensor_time;

	// calculate new speed
	train->speed_n = distance;
	train->speed_d = time;

	// update location
	assert( status == 0 );
	
	train->last_sensor = sensor_node;
	train->distance = 0;
	
	return ERR_NONE;
}

