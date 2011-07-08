#include <types.h>
#include <err.h>
#include <lib/str.h>
#include <user/assert.h>
#include "inc/train_location.h"
#include "inc/train_types.h"
#include "inc/train_tracking.h"
#include "inc/error_tolerance.h"


#define LOCAL_DEBUG
#include <user/dprint.h>



// local

// end local

int expect_sensor( int sensor_expect[ SENSOR_GROUP_COUNT ][ SENSOR_COUNT_PER_GROUP ], track_node* current_sensor ){
	int group = current_sensor->group;
	int id = current_sensor->id;

	sensor_expect[group][id] += 1;
	return 0;
}

int forget_sensor( int sensor_expect[ SENSOR_GROUP_COUNT ][ SENSOR_COUNT_PER_GROUP ], track_node* current_sensor ){
	int group = current_sensor->group;
	int id = current_sensor->id;

	if ( sensor_expect[group][id] > 0 ) sensor_expect[group][id] -= 1;
	return 0;
}

int sensor_trustable( track_node* sensor ){
	return ( sensor->broken < SENSOR_NOT_TRUSTABLE );
}

int sensor_error( track_node* sensor ){
	if ( sensor_trustable( sensor ) ) {
		sensor->broken += 1;
		dprintf( "sensor %c%d is less trustable\n", sensor->group+'A', sensor->id+1 );
	}
	else {
		// dprintf( "sensor %c%d is considered not trustable\n", sensor->group+'A', sensor->id+1 );
	}
	return 0;
}

int sensor_trust( track_node* sensor ){
	if ( sensor->broken && sensor_trustable( sensor ) ) sensor->broken -= 1;
	return 0;
}

int train_next_possible( Train_data* train, int* switch_table ){
	assert( train->next_sensor );
	assert( train->last_sensor );

	track_node* primary = train->next_sensor;
	track_node* secondary = 0;
	track_node* tertiary = 0;
	track_node* last = train->last_sensor;
	track_node* temp = train->last_sensor;
	assert( last );
	
	int dist;

	/* find a trustable primary */
	while ( primary && !sensor_trustable( primary ) ) {
		primary = track_next_sensor( primary, switch_table );
	}

	/* find trustable secodnary */
	secondary = primary;
	do {
		secondary = track_next_sensor( secondary, switch_table );
	} while ( secondary && !sensor_trustable( secondary ) );

	/* find tertiary if possible */
	do {
		temp = temp->edge[DIR_AHEAD].dest;
		if ( temp->type == NODE_BRANCH ) {
			int id = SWID_TO_ARRAYID( temp->id + 1 );
			if ( switch_table[id] == 'C' ) {
				tertiary = temp->edge[DIR_STRAIGHT].dest;
			} else {
				tertiary = temp->edge[DIR_CURVED].dest;
			}
			if ( tertiary->type != NODE_SENSOR ) {
				do {
					tertiary = track_next_sensor( tertiary, switch_table );
				} while ( tertiary && !sensor_trustable( tertiary ) );
			}
			break;
		}
	} while ( temp != primary );

	train->next_sensor = primary;
	train->secondary_sensor = secondary;
	train->tertiary_sensor = tertiary;
	
	/* update pred time */
	if ( primary ) {
		temp = last;
		dist = track_next_sensor_distance( temp, switch_table );
		
		switch ( train->state ){
		case TRAIN_STATE_SPEED_CHANGE:
			break;
		case TRAIN_STATE_TRACKING:
			train->next_time_pred = ( dist - train->tracking.distance ) / train->tracking.speed + train->tracking.speed_change_start_time;
			break;
		default:
			assert(0);
		}
	}
	

	if ( secondary ) {
		temp = primary;
		dist += track_next_sensor_distance( temp, switch_table );
		
		switch ( train->state ){
		case TRAIN_STATE_SPEED_CHANGE:
			break;
		case TRAIN_STATE_TRACKING:
			train->secondary_time_pred = ( dist - train->tracking.distance ) / train->tracking.speed + train->tracking.speed_change_start_time;
			break;
		default:
			assert(0);
		}
	}

	dprintf( "last %c%d @ %d primary %c%d @ %d secondary %c%d @ %d\n", last->group+'A', last->id+1, train->tracking.speed_change_start_time, primary->group+'A', primary->id+1, train->next_time_pred, secondary->group+'A', secondary->id+1, train->secondary_time_pred );
	return 0;
}

int train_expect_sensors( Train_data* train, int sensor_expect[ SENSOR_GROUP_COUNT ][ SENSOR_COUNT_PER_GROUP ] ) {
	track_node* primary = train->next_sensor;
	track_node* secondary = train->secondary_sensor;
	track_node* tertiary = train->tertiary_sensor;

	if ( primary ) expect_sensor( sensor_expect, primary );
	if ( secondary ) expect_sensor( sensor_expect, secondary );
	if ( tertiary ) expect_sensor( sensor_expect, tertiary );
	return 0;
}

int train_forget_sensors( Train_data* train, int sensor_expect[ SENSOR_GROUP_COUNT ][ SENSOR_COUNT_PER_GROUP ] ) {
	track_node* primary = train->next_sensor;
	track_node* secondary = train->secondary_sensor;
	track_node* tertiary = train->tertiary_sensor;

	if ( primary ) forget_sensor( sensor_expect, primary );
	if ( secondary ) forget_sensor( sensor_expect, secondary );
	if ( tertiary ) forget_sensor( sensor_expect, tertiary );
	return 0;
}



int train_hit_untrustable( Train_data* train ){
	track_node* next_sensor = train->next_sensor;
	int eta = train_tracking_eta( train );

	return ( !sensor_trustable( next_sensor ) && (eta <= 0) );
}

