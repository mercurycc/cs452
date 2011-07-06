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

int expect_sensor( uchar* sensor_expect, track_node* current_sensor ){
	int group = current_sensor->group * 2 + current_sensor->id / 8;
	int id = current_sensor->id % 8;

	sensor_expect[group] = sensor_expect[group] | (0x80 >> id);
	return 0;
}

int forget_sensor( uchar* sensor_expect, track_node* current_sensor ){
	int group = current_sensor->group * 2 + current_sensor->id / 8;
	int id = current_sensor->id % 8;

	sensor_expect[group] = sensor_expect[group] & ~(0x80 >> id);
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
		dprintf( "sensor %c%d is considered not trustable\n", sensor->group+'A', sensor->id+1 );
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
	track_node* temp = train->last_sensor;

	while ( primary && !sensor_trustable( primary ) ) {
		primary = track_next_sensor( primary, switch_table );
	}

	secondary = primary;
	do {
		secondary = track_next_sensor( secondary, switch_table );
	} while ( secondary && !sensor_trustable( secondary ) );

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
/*
	// need to do this if we need to calculate the time window for secondary sensor
	if ( secondary ) {
		int primary_eta = train_tracking_eta( train );
		int primary_distance = train_tracking_remaining_distance( train );
		int secondary_distance = track_next_sensor_distance( primary, switch_table );
		int secondary_eta = ( int )( (secondary_distance + primary_distance) / train->tracking.speed );
		assert(secondary_eta >= 0);
		train->secondary_eta = secondary_eta;
	}
*/

	// dprintf( "primary %c%d secondary %c%d tertiary %c%d secondary eta %d\n", primary->group+'A', primary->id+1, secondary->group+'A', secondary->id+1, tertiary->group+'A', tertiary->id+1, train->secondary_eta );
	return 0;
}

int train_expect_sensors( Train_data* train, uchar* sensor_expect ) {
	track_node* primary = train->next_sensor;
	track_node* secondary = train->secondary_sensor;
	track_node* tertiary = train->tertiary_sensor;

	if ( primary ) expect_sensor( sensor_expect, primary );
	if ( secondary ) expect_sensor( sensor_expect, secondary );
	if ( tertiary ) expect_sensor( sensor_expect, tertiary );
	return 0;
}

int train_forget_sensors( Train_data* train, uchar* sensor_expect ) {
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

