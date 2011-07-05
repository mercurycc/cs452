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

int sensor_trustable( track_node* sensor ){
	return ( sensor->broken < SENSOR_NOT_TRUSTABLE );
}

int sensor_error( track_node* sensor ){
	sensor->broken += 1;
	return 0;
}

int sensor_trust( track_node* sensor ){
	if ( sensor->broken )
		sensor->broken -= 1;
	return 0;
}

int train_next_possible( Train_data* train, int* switch_table ){
	assert( train->next_sensor );
	assert( train->last_sensor );

	track_node* primary = train->next_sensor;
	track_node* secondary = track_next_sensor( primary, switch_table );
	track_node* tertiary = 0;
	track_node* temp = train->last_sensor;

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
				tertiary = track_next_sensor( tertiary, switch_table );
			}
			break;
		}
	} while ( temp != primary );

	train->secondary_sensor = secondary;
	train->tertiary_sensor = tertiary;

	dprintf( "primary %c%d secondary %c%d tertiary %c%d\n", primary->group+'A', primary->id+1, secondary->group+'A', secondary->id+1, tertiary->group+'A', tertiary->id+1 );
	return 0;
}

int train_hit_untrustable( Train_data* train ){
	track_node* next_sensor = train->next_sensor;
	int eta = train_tracking_eta( train );

	return ( !sensor_trustable( next_sensor ) && (eta <= 0) );
}

