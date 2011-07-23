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
	assert( sensor );
	if ( sensor_trustable( sensor ) ) {
		sensor->broken += 1;
		dprintf( "sensor %c%d is less trustable\n", sensor->group+'A', sensor->id+1 );
	}
	else {
		//dprintf( "sensor %c%d is considered not trustable\n", sensor->group+'A', sensor->id+1 );
	}
	return 0;
}

int sensor_trust( track_node* sensor ){
	if ( sensor->broken && sensor_trustable( sensor ) ) sensor->broken -= 1;
	return 0;
}

int train_next_possible( Train_data* train, int* switch_table )
{
	assert( train->last_sensor );

	/* If train stops heading an exit then no sensor available */
	if( ! train->next_sensor ){
		return 0;
	}

	track_node* primary = train->next_sensor;
	track_node* secondary = 0;
	track_node* tertiary = 0;
	track_node* last = train->last_sensor;
	track_node* temp = train->last_sensor;
	int dist = 0;
	assert( last );

	/* find a trustable primary */
	while ( primary && !sensor_trustable( primary ) ) {
		primary = track_next_sensor( primary, switch_table );
	}

	/* find trustable secodnary */
	secondary = primary;
	do {
		/* secondary has a chance to be null */
		if ( !secondary ) {
			break;
		}
		secondary = track_next_sensor( secondary, switch_table );
	} while ( secondary && !sensor_trustable( secondary ) );

	/* find tertiary if possible */
	do {
		dist += node_distance( temp, switch_table );
		temp = track_next_node( temp, switch_table );
		if ( temp && temp->type == NODE_BRANCH ) {
			int id = SWID_TO_ARRAYID( temp->id + 1 );
			if ( switch_table[id] == 'C' ) {
				tertiary = temp->edge[DIR_STRAIGHT].dest;
				dist += temp->edge[DIR_STRAIGHT].dist;
			} else {
				tertiary = temp->edge[DIR_CURVED].dest;
				dist += temp->edge[DIR_CURVED].dist;
			}
			while ( tertiary && ( tertiary->type != NODE_SENSOR || !sensor_trustable( tertiary ) ) ) {
				dist += node_distance( tertiary, switch_table );
				tertiary = track_next_sensor( tertiary, switch_table );
			} 
			break;
		}
	} while ( primary && temp != primary );

	if( temp == primary ){
		tertiary = 0;
	}

	train->next_sensor = primary;
	train->secondary_sensor = secondary;
	train->tertiary_sensor = tertiary;
	if ( tertiary ) {
		train->tertiary_distance = dist;
	}
	else {
		train->tertiary_distance = 0;
	}
	
	/* update pred time */
	// train_update_time_pred( train, switch_table );
	
	uint range = train->next_time_range;
	uint a = train->next_time_pred - range;
	uint b = train->next_time_pred + range;
	
	
	// dprintf( "last %c%d @ %d primary %c%d @ [ %d , %d ] range %d\n", last->group+'A', last->id+1, train->tracking.speed_change_last_integration, primary->group+'A', primary->id+1, a, b, range );
	/*
	int group[4] = { 13, 13, 13, 13 };
	int id[4] = { 0, 0, 0, 0 };
	if ( last ) {
		group[0] = last->group+'A';
		id[0] = last->id+1;
	}
	if ( primary ) {
		group[1] = primary->group+'A';
		id[1] = primary->id+1;
	}
	if ( secondary ) {
		group[2] = secondary->group+'A';
		id[2] = secondary->id+1;
	}
	if ( tertiary ) {
		group[3] = tertiary->group+'A';
		id[3] = tertiary->id+1;
	}
	
	dprintf( "last %c%d primary %c%d secondary %c%d tertiary %c%d\n", group[0], id[0], group[1], id[1], group[2], id[2], group[3], id[3] );
	*/
	return 0;
}

int train_update_time_pred( Train_data* train, int* switch_table ){
	track_node* primary = train->next_sensor;
	track_node* secondary = train->secondary_sensor;
	track_node* tertiary = train->tertiary_sensor;
	track_node* temp = 0;
	int dist = 0;

	if ( primary ) {
		temp = train->check_point;
		dist = track_next_sensor_distance( temp, switch_table );
		
		switch ( train->state ){
		case TRAIN_STATE_SPEED_CHANGE:
			train->next_time_pred = train_time_to_distance( train, ( dist - train->tracking.distance ) ) + train->tracking.speed_change_last_integration;
			break;
		case TRAIN_STATE_TRACKING:
			train->next_time_pred = ( dist - train->tracking.distance ) / train->tracking.speed + train->tracking.speed_change_last_integration;
			break;
		default:
			train->next_time_pred = 0;
		}
		// dprintf( "range is 1/3 of %d - %d\n", train->next_time_pred, train->tracking.speed_change_last_integration );
		train->next_time_range = ( train->next_time_pred - train->tracking.speed_change_last_integration ) * TIME_WINDOW_RANGE_FACTOR;
	}
	else {
		train->next_time_pred = 0;
	}
	
	return 0;
}

int train_arrive_on_time( Train_data* train, uint sensor_time  ){
	uint range = train->next_time_range;
	if ( !range ) {
		return 1;
	}
	uint low = train->next_time_pred - range;
	uint high = train->next_time_pred + range;
	
	return ( sensor_time <= high && sensor_time >= low );
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

