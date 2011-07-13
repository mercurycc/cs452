#include <types.h>
#include <err.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/time.h>
#include <user/devices/clock.h>     /* For CLOCK_COUNT_DOWN_MS_PER_TICK */
#include <lib/str.h>
#include "inc/train_types.h"
#include "inc/train.h"
#include "inc/train_tracking.h"
#include "inc/track_node.h"
#include "inc/train_location.h"
#include "inc/error_tolerance.h"

#define LOCAL_DEBUG
#include <user/dprint.h>

static void train_tracking_sc_current_speed( Train* train, int curtime )
{
	if( curtime < train->tracking.speed_change_end_time ){
		train->tracking.speed = ( ( train->tracking.speed_stat_table[ train->tracking.speed_level ] - train->tracking.old_speed ) *
					  ( curtime - train->tracking.speed_change_start_time )
					  / ( train->tracking.speed_change_end_time - train->tracking.speed_change_start_time ) + train->tracking.old_speed );
	} else {
		train->tracking.speed = train->tracking.speed_stat_table[ train->tracking.speed_level ];
	}
}

static int train_tracking_sc_dist_diff( Train* train, int curtime )
{
	int dist_diff = 0;

	if( curtime < train->tracking.speed_change_end_time ){
		if( curtime <= train->tracking.speed_change_start_time ){
			/* Hack.  This should not happen */
			dist_diff = 0;
		} else {
			dist_diff = ( int )( ( curtime - train->tracking.speed_change_start_time ) * ( train->tracking.old_speed + train->tracking.speed ) / 2 );
		}
	} else {
		dist_diff += ( int )( ( curtime - train->tracking.speed_change_start_time ) * train->tracking.speed );
	}

	assert( dist_diff >= 0 );
	return dist_diff;
}

static void train_tracking_sc_update_time_point( Train* train, int curtime )
{
	train->tracking.speed_change_start_time = curtime;
	train->tracking.old_speed = train->tracking.speed;
}

static inline int train_tracking_sc_time( Train* train )
{
	int diff;
	
	if( train->tracking.old_speed_level ){
		diff = train->tracking.old_speed_level - train->tracking.speed_level;
		if( diff < 0 ){
			diff = -diff;
		}
		if( diff < CLOSE_SPEED_CHANGE_GAP ){
			return CLOSE_SPEED_CHANGE_TIME;
		}
		return SPEED_CHANGE_TIME;
	} else {
		/* If the train was picking up speed from 0 then we use this number */
		return START_CHANGE_TIME;
	}
}

static void train_tracking_update_check_point( Train* train, int check_point_time, int curtime )
{
	int old_remaining;
	
	old_remaining = train->tracking.remaining_distance;
	
	/* Update check point */
	train->check_point = train->next_check_point;
	train->next_check_point = track_next_node( train->check_point, train->switch_table );
	train->tracking.check_point_time = check_point_time;

	/* Initialize distance */
	train->tracking.remaining_distance = node_distance( train->check_point, train->switch_table );
	switch( train->state ){
	case TRAIN_STATE_TRACKING:
		train->tracking.distance = ( int )( ( curtime - check_point_time ) * train->tracking.speed );
		break;
	default:
		train->tracking.distance = ( old_remaining ) >= 0 ? 0 : -old_remaining;
		break;
	}
	train->tracking.remaining_distance -= train->tracking.distance;

	/* Notify whoever is waiting for an update */
	sem_release( train->update );
}

int train_tracking_init( Train* train )
{
	int i;
	
	memset( ( uchar* )&train->tracking, 0, sizeof( train->tracking ) );

	for( i = 2; i < NUM_SPEED_LEVEL; i += 1 ){
		train->tracking.speed_stat_table[ i ] = 100;
	}

	return ERR_NONE;
}

int train_tracking_init_calib( Train* train )
{
	float factor_1;
	float factor_2;
	float factor;
	int i;

	factor_1 = train->tracking.speed_stat_table[ TRAIN_AUTO_REG_SPEED_1 * 2 ] / ( 2 * TRAIN_AUTO_REG_SPEED_1 );
	factor_2 = train->tracking.speed_stat_table[ TRAIN_AUTO_REG_SPEED_2 * 2 ] / ( 2 * TRAIN_AUTO_REG_SPEED_2 );

	dprintf( "Train %d factor 1 = %d, factor 2 = %d\n", ( int )( factor_1 * 1 ), ( int )( factor_2 * 1 ) );
	
	factor = ( factor_1 + factor_2 ) / 2;

	train->tracking.speed_stat_table[ 0 ] = 0;
	train->tracking.speed_stat_count[ 0 ] = 1;
	train->tracking.speed_stat_table[ 1 ] = 0;
	train->tracking.speed_stat_count[ 1 ] = 1;

	for( i = 2; i < NUM_SPEED_LEVEL; i += 1 ){
		train->tracking.speed_stat_table[ i ] = ( float )i * factor;
		train->tracking.speed_stat_count[ i ] = 1;
	}

	return 0;
}

int train_tracking_new_sensor( Train* train, int sensor_time, int curtime )
{
	int status;

	/* Force next check point to be the current sensor triggered */
	train->next_check_point = train->last_sensor;
	
	train_tracking_update_check_point( train, sensor_time, curtime );
	
	status = train_tracking_update( train, curtime );
	assert( status == ERR_NONE );
	
	switch( train->state ){
	case TRAIN_STATE_TRACKING:
		/* Update speed */
		train->tracking.speed = ( ( train->tracking.speed_stat_table[ train->tracking.speed_level ] *
					    train->tracking.speed_stat_count[ train->tracking.speed_level ] +
					    ( float )train->tracking.trav_distance / ( float )( sensor_time - train->tracking.trav_time_stamp ) ) /
					  ( train->tracking.speed_stat_count[ train->tracking.speed_level ] + 1 ) );
		train->tracking.speed_stat_table[ train->tracking.speed_level ] = train->tracking.speed;

		if( train->tracking.speed_stat_count[ train->tracking.speed_level ] < NUM_SPEED_HISTORY ){
			train->tracking.speed_stat_count[ train->tracking.speed_level ] += 1;
		}
		break;
	case TRAIN_STATE_SPEED_CHANGE:
		if( curtime >= train->tracking.speed_change_end_time ){
			if( train->tracking.speed_level / 2 ){
				train->state = TRAIN_STATE_TRACKING;
			} else {
				train->tracking.speed = 0;
				train->state = TRAIN_STATE_STOP;
			}
		} else {
			train->tracking.speed_change_start_time = curtime;
			train->tracking.old_speed = train->tracking.speed;
		}
		break;
	default:
		break;
	}

	train->tracking.trav_distance = track_next_sensor_distance( train->last_sensor, train->switch_table );
	train->tracking.trav_time_stamp = sensor_time;

	return 0;
}

int train_tracking_update_speed( Train* train, int curtime )
{
	switch( train->state ){
	case TRAIN_STATE_SPEED_CHANGE:
		train_tracking_sc_current_speed( train, curtime );
		break;
	case TRAIN_STATE_TRACKING:
	default:
		break;
	}

	return ERR_NONE;
}

int train_tracking_update_position( Train* train, int curtime )
{
	int dist_diff;

	switch( train->state ){
	case TRAIN_STATE_SPEED_CHANGE:
		dist_diff = train_tracking_sc_dist_diff( train, curtime );
		break;
	case TRAIN_STATE_TRACKING:
		dist_diff = ( int )( ( curtime - train->tracking.check_point_time ) * train->tracking.speed ) - train->tracking.distance;
		break;
	default:
		break;
	}

	switch( train->state ){
	case TRAIN_STATE_SPEED_CHANGE:
	case TRAIN_STATE_TRACKING:
		if( dist_diff >= 0 ){
			train->tracking.distance += dist_diff;
			train->tracking.remaining_distance -= dist_diff;

			if( train->tracking.remaining_distance >= 0 ){
				train->mark_dist -= dist_diff;
			}
		
			while( train->tracking.remaining_distance <= 0 && ( train->next_check_point->type != NODE_SENSOR || !sensor_trustable( train->next_check_point ) ) ){
				train_tracking_update_check_point( train, curtime, curtime );
			}
		}
		break;
	default:
		break;
	}

	return ERR_NONE;
}

int train_tracking_update_eta( Train* train, int curtime )
{
	if( train->tracking.speed ){
		if ( train->state == TRAIN_STATE_TRACKING ) {
			train->tracking.eta = ( int )( train->tracking.remaining_distance / train->tracking.speed );
		}
		else if ( train->state == TRAIN_STATE_SPEED_CHANGE ){
			train->tracking.eta = train_time_to_distance( train, train->tracking.remaining_distance );
			//dprintf( "eta %d\n", train->tracking.eta );
		}
	} else {
		train->tracking.eta = 0;
	}

	return ERR_NONE;
}

int train_tracking_update( Train* train, int curtime )
{
	int status;

	status = train_tracking_update_speed( train, curtime );
	assert( status == ERR_NONE );

	status = train_tracking_update_position( train, curtime );
	assert( status == ERR_NONE );

	status = train_tracking_update_eta( train, curtime );
	assert( status == ERR_NONE );

	train_tracking_sc_update_time_point( train, curtime );

	/* Set train to stop if speed level is 0 and speed change has completed */
	if( train->tracking.speed_level / 2 == 0 && curtime >= train->tracking.speed_change_end_time ){
		train->tracking.speed = 0;
		train->state = TRAIN_STATE_STOP;
	}

	return ERR_NONE;
}

int train_tracking_reverse( Train* train )
{
	int temp;
	
	temp = train->tracking.remaining_distance;
	train->tracking.remaining_distance = train->tracking.distance;
	train->tracking.distance = temp;
	train->tracking.speed = 0;
	train->tracking.speed_level = 0;

	return ERR_NONE;
}

int train_tracking_speed_change( Train* train, int new_speed_level, int curtime )
{
	uint old_speed_level;

	/* For acc/deceleration */
	new_speed_level *= 2;

	old_speed_level = train->tracking.speed_level;
	train->state = TRAIN_STATE_SPEED_CHANGE;
	if( old_speed_level > new_speed_level ){
		new_speed_level += 1;
	}
	train->tracking.old_speed_level = train->tracking.speed_level;
	train->tracking.speed_level = new_speed_level;
	train->tracking.old_speed = train->tracking.speed;
	train->tracking.speed_change_start_time = curtime;
	train->tracking.speed_change_end_time = train_tracking_sc_time( train ) + curtime;

	return ERR_NONE;
}

int train_tracking_trav_dist( const Train* train, int ticks )
{
	int dist;
	float end_speed;
	int curtime;
	
	switch( train->state ){
	case TRAIN_STATE_SPEED_CHANGE:
		curtime = Time();
		if( curtime < train->tracking.speed_change_end_time ){
			end_speed = ( ( train->tracking.speed_stat_table[ train->tracking.speed_level ] - train->tracking.old_speed ) *
				      ( curtime + ticks - train->tracking.speed_change_start_time )
				      / ( train->tracking.speed_change_end_time - train->tracking.speed_change_start_time ) + train->tracking.old_speed );

			dist = ( int )( ( end_speed + train->tracking.speed ) * ticks / 2 );
			break;
		}
	case TRAIN_STATE_TRACKING:
		dist = ( int )( train->tracking.speed * ticks );
		break;
	}
	return dist;
}

int train_tracking_trav_time( const Train* train, int dist )
{
	return ( int )( dist / train->tracking.speed );
}

int train_tracking_speed_in_sec( const Train* train )
{
	return train->tracking.speed * ( 1000 / CLOCK_COUNT_DOWN_MS_PER_TICK );
}

int train_tracking_position( const Train* train )
{
	return train->tracking.distance;
}

int train_tracking_remaining_distance( const Train* train )
{
	return train->tracking.remaining_distance;
}

int train_tracking_eta( const Train* train )
{
	return train->tracking.eta;
}

int train_tracking_stop_distance( const Train* train )
{
	return ( int )( train->tracking.speed * ( float )( train_tracking_sc_time( train ) ) / 2 );
}
