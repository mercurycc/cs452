/* Train_auto receives sensor and control data to make judgements */
/* It is not implemented strictly as a administrator because incoming
   requests should be rare relative to its speed (sensor every 60 ms,
   control command every second, such and such) */

#include <types.h>
#include <err.h>
#include <lib/str.h>
#include <user/syscall.h>
#include <user/assert.h>
#include <user/time.h>
#include <user/name_server.h>
#include <config.h>
#include <user/display.h>
#include <user/semaphore.h>
#include "inc/train.h"
#include "inc/config.h"
#include "inc/sensor_data.h"
#include "inc/track_data.h"
#include "inc/track_node.h"
#include "inc/warning.h"
#include "inc/train_location.h"
#include "inc/train_types.h"
#include "inc/train_tracking.h"
#include "inc/error_tolerance.h"

// #define LOCAL_DEBUG
#include <user/dprint.h>

enum Train_pickup {
	TRAIN_PICKUP_FRONT,
	TRAIN_PICKUP_BACK,
	TRAIN_PICKUP_UNKONWN
};

enum Train_auto_request_type {
	TRAIN_AUTO_INIT,
	TRAIN_AUTO_WAKEUP,
	TRAIN_AUTO_NEW_SENSOR_DATA,
	TRAIN_AUTO_NEW_TRAIN,
	TRAIN_AUTO_SET_TRAIN_SPEED,
	TRAIN_AUTO_SET_TRAIN_REVERSE,
	TRAIN_AUTO_SET_SWITCH_DIR,
	TRAIN_AUTO_SET_ALL_SWITCH,
	TRAIN_AUTO_QUERY_SWITCH_DIR,
	TRAIN_AUTO_QUERY_LAST_SENSOR,
	TRAIN_AUTO_HIT_AND_STOP,
	TRAIN_AUTO_PLAN,
	TRAIN_AUTO_NONE
};

typedef struct Train_auto_request_s {
	uint type;
	union {
		struct {
			uint track_id;
		} init;
		Sensor_data sensor_data;
		struct {
			uint train_id;
		} new_train;
		struct {
			uint train_id;
			uint speed_level;
		} set_speed;
		struct {
			uint train_id;
		} set_reverse;
		struct {
			uint switch_id;
			char direction;
		} set_switch;
		struct {
			char direction;
		} set_switch_all;
		struct {
			uint switch_id;
		} query_switch;
		struct {
			uint dummy;
		} query_sensor;
		struct {
			uint train_id;
			uint group;
			uint id;
			uint distance;
		} hit_and_stop;
		struct {
			uint train_id;
			uint group;
			uint id;
			uint dist_pass;
		} plan;
	} data;
} Train_auto_request;

typedef struct Train_auto_reply_s {
	int group;
	int id;
} Train_auto_reply;

static int train_auto_wakeup( int tid );

static void train_auto_alarm()
{
	int ptid = MyParentTid();

	while( 1 ){
		Delay( 1 );

		train_auto_wakeup( ptid );
	}
}

static inline void train_auto_recompose_set_speed( Train_auto_request* request, int train_id, int speed_level )
{
	request->type = TRAIN_AUTO_SET_TRAIN_SPEED;
	request->data.set_speed.train_id = train_id;
	request->data.set_speed.speed_level = speed_level;
}

void train_auto()
{
	Train_auto_request request;
	Train_auto_reply reply;
	Sensor_data sensor_data;
	// uchar sensor_expect[ SENSOR_BYTE_COUNT ] = { 0 };
	int sensor_expect[ SENSOR_GROUP_COUNT ][ SENSOR_COUNT_PER_GROUP ] = { { 0 } };
	int last_sensor_group = -1;
	int last_sensor_id = -1;
	int tracking_ui_tid;
	int alarm_tid;
	int control_tid;
	int tid;
	int i;
	int temp;
	track_node track_graph[ TRACK_NUM_NODES ];
	int node_map[ GROUP_COUNT ][ TRACK_GRAPH_NODES_PER_GROUP ];
	Train_data trains[ MAX_NUM_TRAINS ] = { { 0 } };
	int train_map[ MAX_TRAIN_ID ] = { 0 };
	int available_train = 1;     /* Record 0 is not used, to distinguish non-registered car */
	Train_data* current_train;
	track_node* current_sensor;
	track_node* next_sensor;
	track_node* current_node;
	int switch_table[ NUM_SWITCHES ] = { 0 };
	int module_tid;
	uint current_time;
	uint current_distance;
	uint reprocess = 0;
	int status;

	/*
	for ( i = 0; i < SENSOR_BYTE_COUNT; i++ ){
		sensor_expect[i] = 0;
	}
	*/

	/* Receive initialization data */
	status = Receive( &tid, ( char* )&request, sizeof( request ) );
	assert( status == sizeof( int ) + sizeof( request.data.init ) );
	status = Reply( tid, ( char* )&reply, sizeof( reply ) );
	assert( status == SYSCALL_SUCCESS );

	switch( request.data.init.track_id ){
	case TRACK_A:
		init_tracka( track_graph, node_map );
		break;
	case TRACK_B:
		init_trackb( track_graph, node_map );
		break;
	default:
		assert( 0 );
	}

	status = RegisterAs( TRAIN_AUTO_NAME );
	assert( status == REGISTER_AS_SUCCESS );

	module_tid = WhoIs( TRAIN_MODULE_NAME );
	assert( module_tid > 0 );

	tracking_ui_tid = WhoIs( TRACKING_UI_NAME );
	assert( tracking_ui_tid > 0 );

	control_tid = WhoIs( CONTROL_NAME );
	assert( control_tid > 0 );
	
	/* Start alarm */
	alarm_tid = Create( TRAIN_AUTO_PRIROTY + 1, train_auto_alarm );
	assert( alarm_tid > 0 );
	
	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );

		current_time = Time();

		status -= sizeof( uint );
		/* Size check */
		switch( request.type ){
		case TRAIN_AUTO_INIT:
			/* No more init should be received */
			assert( 0 );
			break;
		case TRAIN_AUTO_WAKEUP:
			assert( status == 0 );
			break;
		case TRAIN_AUTO_NEW_SENSOR_DATA:
			assert( status == sizeof( request.data.sensor_data ) );
			break;
		case TRAIN_AUTO_NEW_TRAIN:
			assert( status == sizeof( request.data.new_train ) );
			break;
		case TRAIN_AUTO_SET_TRAIN_SPEED:
			assert( status == sizeof( request.data.set_speed ) );
			break;
		case TRAIN_AUTO_SET_TRAIN_REVERSE:
			assert( status == sizeof( request.data.set_reverse ) );
			break;
		case TRAIN_AUTO_SET_SWITCH_DIR:
			assert( status == sizeof( request.data.set_switch ) );
			break;
		case TRAIN_AUTO_SET_ALL_SWITCH:
			assert( status == sizeof( request.data.set_switch_all ) );
			break;
		case TRAIN_AUTO_QUERY_SWITCH_DIR:
			assert( status == sizeof( request.data.query_switch ) );
			break;
		case TRAIN_AUTO_QUERY_LAST_SENSOR:
			assert( status == sizeof( request.data.query_sensor ) );
			break;
		case TRAIN_AUTO_HIT_AND_STOP:
			assert( status == sizeof( request.data.hit_and_stop ) );
			break;
		case TRAIN_AUTO_PLAN:
			assert( status == sizeof( request.data.plan ) );
			break;
		default:
			assert( 0 );
			break;
		}

		/* Quick reply */
		switch( request.type ){
		case TRAIN_AUTO_NEW_SENSOR_DATA:
		case TRAIN_AUTO_NEW_TRAIN:
		case TRAIN_AUTO_SET_TRAIN_SPEED:
		case TRAIN_AUTO_SET_TRAIN_REVERSE:
		case TRAIN_AUTO_SET_SWITCH_DIR:
		case TRAIN_AUTO_SET_ALL_SWITCH:
		case TRAIN_AUTO_HIT_AND_STOP:
		case TRAIN_AUTO_WAKEUP:
		case TRAIN_AUTO_PLAN:
			status = Reply( tid, ( char* )&reply, sizeof( reply ) );
			assert( status == SYSCALL_SUCCESS );
		default:
			break;
		}

		/* Skip if train is not registered */
		switch( request.type ){
		case TRAIN_AUTO_SET_TRAIN_SPEED:
			temp = train_map[ request.data.set_speed.train_id ];
			break;
		case TRAIN_AUTO_SET_TRAIN_REVERSE:
			temp = train_map[ request.data.set_reverse.train_id ];
			break;
		case TRAIN_AUTO_PLAN:
			temp = train_map[ request.data.plan.train_id ];
			break;
		}
		
		switch( request.type ){
		case TRAIN_AUTO_SET_TRAIN_SPEED:
		case TRAIN_AUTO_SET_TRAIN_REVERSE:
		case TRAIN_AUTO_PLAN:
			if( ! temp ){
				continue;
			}
			/* Bring control back to manual if source is train_train */
			/* Clear planner control if the request is from user */
			if( tid == control_tid ){
				current_train = trains + train_map[ request.data.new_train.train_id ];
				current_train->planner_control = 0;
			}
		}


		do {
			if( reprocess ){
				dnotice( "Reprocessing...\n" );
			}
			
			reprocess = 0;
			/* Process request */
			switch( request.type ){
			case TRAIN_AUTO_NEW_SENSOR_DATA:
				/* Unfortuate memcpy */
				memcpy( ( uchar* )&sensor_data, ( uchar* )&request.data.sensor_data, sizeof( sensor_data ) );

				/* Update last triggered sensor, for wh */
				for( temp = 0; temp < SENSOR_BYTE_COUNT; temp += 1 ){
					if( ! sensor_data.sensor_raw[ temp ] ){
						continue;
					}
					last_sensor_group = temp;
					for( i = 0; i < BITS_IN_BYTE; i += 1 ){
						if( ! ( sensor_data.sensor_raw[ temp ] & ( ( 1 << 7 ) >> i ) ) ){
							continue;
						}
						int group = temp / 2;
						int id = i + ( temp % 2 ) * 8;
						current_sensor = track_graph + node_map[ group ][ id ];

						if ( !sensor_trustable( current_sensor ) ) {
							clear_sensor_data( &sensor_data, current_sensor );
						}
						else {
							last_sensor_group = group;
							last_sensor_id = id;
							if ( !( sensor_expect[ group ][ id ] ) ){
								sensor_error( current_sensor );
							}
						}
					}
				}
				
				break;
			case TRAIN_AUTO_NEW_TRAIN:
				if( ! train_map[ request.data.new_train.train_id ] ){
					/* Map train */
					current_train = trains + available_train;
					train_map[ request.data.new_train.train_id ] = available_train;
					available_train += 1;

					/* Create planner */
					current_train->planner_tid = Create( TRAIN_PLAN_PRIORITY, train_planner );
					assert( current_train->planner_tid > 0 );

					current_train->id = request.data.new_train.train_id;

					/* Initialize semaphore */
					current_train->sem = &current_train->sem_body;

					status = sem_init( current_train->sem, 1 );
					assert( status == ERR_NONE );

					/* Initialize update notifier */
					current_train->update = &current_train->update_body;

					status = sem_init( current_train->update, 0 );
					assert( status == ERR_NONE );

					current_train->track_graph = track_graph;
					current_train->switch_table = switch_table;
					current_train->node_map = ( int* )node_map;

					current_train->tracking.speed_change_time = SPEED_CHANGE_TIME;

					status = train_planner_init( current_train->planner_tid, current_train );
					assert( status == ERR_NONE );

					/* Update UI */
					tracking_ui_new_train( tracking_ui_tid, request.data.new_train.train_id );
				} else {
					current_train = trains + train_map[ request.data.new_train.train_id ];
				}

				/* Set initial states */
				current_train->state = TRAIN_STATE_INIT_1;
				current_train->init_state = TRAIN_STATE_INIT_1;
				current_train->pickup = TRAIN_PICKUP_FRONT;

				train_set_speed( module_tid, current_train->id, TRAIN_AUTO_REG_SPEED_1 );

				/* Compose speed change request */
				train_auto_recompose_set_speed( &request, current_train->id, TRAIN_AUTO_REG_SPEED_1 );

				/* Process the composed request */
				reprocess = 1;

				dprintf( "Train %d register\n", current_train->id );
				
				break;
			case TRAIN_AUTO_SET_TRAIN_SPEED:
				current_train = trains + train_map[ request.data.set_speed.train_id ];
				if( request.data.set_speed.speed_level != 15 ){
					current_train->state = TRAIN_STATE_SPEED_CHANGE;
					sem_acquire_all( current_train->sem );
					train_tracking_speed_change( current_train, request.data.set_speed.speed_level, current_time );
					sem_release( current_train->sem );
					train_update_time_pred( current_train, switch_table );
					break;
				}
			case TRAIN_AUTO_SET_TRAIN_REVERSE:
				current_train = trains + train_map[ request.data.set_reverse.train_id ];
				if( current_train->pickup == TRAIN_PICKUP_FRONT ){
					current_train->pickup = TRAIN_PICKUP_BACK;
				} else if( current_train->pickup == TRAIN_PICKUP_BACK ){
					current_train->pickup = TRAIN_PICKUP_FRONT;
				}
				sem_acquire_all( current_train->sem );
				current_train->check_point = current_train->next_check_point->reverse;
				current_train->next_check_point = track_next_node( current_train->check_point, switch_table );
				current_sensor = current_train->last_sensor;
				
				/* The last sensor does not matter that much */
				current_train->last_sensor = current_train->next_sensor->reverse;
				
				/* The next sensor needs to be found from the check point in case of multiple branches between sensors */
				current_train->next_sensor = track_next_sensor( current_train->check_point, switch_table );
				
				current_train->state = TRAIN_STATE_STOP;
				train_tracking_reverse( current_train );
				train_next_possible( current_train, switch_table );
				train_expect_sensors( current_train, sensor_expect );
				sem_release( current_train->sem );
				break;
			case TRAIN_AUTO_SET_SWITCH_DIR:
				switch_table[ SWID_TO_ARRAYID( request.data.set_switch.switch_id ) ] = request.data.set_switch.direction;
				break;
			case TRAIN_AUTO_SET_ALL_SWITCH:
				for( i = 0; i < NUM_SWITCHES; i += 1 ){
					switch_table[ i ] = request.data.set_switch_all.direction;
				}
				break;
			case TRAIN_AUTO_QUERY_SWITCH_DIR:
				reply.group = switch_table[ SWID_TO_ARRAYID( request.data.set_switch.switch_id ) ];
				break;
			case TRAIN_AUTO_QUERY_LAST_SENSOR:
				reply.group = last_sensor_group;
				reply.id = last_sensor_id;
				break;
			case TRAIN_AUTO_PLAN:
				current_train = trains + train_map[ request.data.plan.train_id ];
				current_sensor = track_graph + node_map[ request.data.plan.group ][ request.data.plan.id ];
				if( ! current_train->planner_control ){
					dprintf( "Train %d received planning request successfully\n", current_train->id );
					train_planner_path_plan( current_train->planner_tid, current_sensor, request.data.plan.dist_pass );
				} else {
					dprintf( "Train %d received planning request failed\n", current_train->id );
				}
				break;
			}

			if( request.type == TRAIN_AUTO_SET_SWITCH_DIR || request.type == TRAIN_AUTO_SET_ALL_SWITCH ||
			    request.type == TRAIN_AUTO_WAKEUP || request.type == TRAIN_AUTO_NEW_SENSOR_DATA ){
				for( temp = 1; temp < available_train; temp += 1 ){
					current_train = trains + temp;
					/* Critical section */
					sem_acquire_all( current_train->sem );

					/* If the switch table is updated, the sensor prediction has to be updated as well */
					if( request.type == TRAIN_AUTO_SET_SWITCH_DIR || request.type == TRAIN_AUTO_SET_ALL_SWITCH ){
						/* The reason that the next check point does not need to be updated is because,
						   if the next check point is before the switch then the next check point should
						   be the switch, and if the train is on the switch the switch should not be
						   switched, and if the train is off the switch then it does not matter */
						train_forget_sensors( current_train, sensor_expect );
						current_train->next_sensor = track_next_sensor( current_train->last_sensor, switch_table );
						current_train->tracking.trav_distance = track_next_sensor_distance( current_train->last_sensor, switch_table );
						train_next_possible( current_train, switch_table );
						train_expect_sensors( current_train, sensor_expect );
						
					}

					/* Process train states */
					if( request.type == TRAIN_AUTO_NEW_SENSOR_DATA ){
						switch( current_train->init_state ){
						case TRAIN_STATE_INIT_1:
							current_train->last_sensor = track_graph + node_map[ last_sensor_group ][ last_sensor_id ];
							current_train->next_sensor = track_next_sensor( current_train->last_sensor, switch_table );
							current_train->init_retry = 0;
							current_train->init_state = TRAIN_STATE_INIT_2;
							dprintf( "Train %d init_1 pass\n", current_train->id );
							break;
						case TRAIN_STATE_INIT_2:
							current_sensor = track_graph + node_map[ last_sensor_group ][ last_sensor_id ];
							if( current_sensor != current_train->next_sensor ){
								current_train->init_retry += 1;
								dprintf( "Train %d init_2 fail %d\n", current_train->id, current_train->init_retry );
							} else {
								current_train->last_sensor = current_sensor;
								current_train->next_sensor = track_next_sensor( current_train->last_sensor, switch_table );
								current_train->init_retry = 0;
								current_train->init_state = TRAIN_STATE_INIT_3;
								dprintf( "Train %d init_2 pass\n", current_train->id );
							}
							if( !( current_train->init_retry < TRAIN_AUTO_REG_RETRY ) ){
								current_train->init_state = TRAIN_STATE_INIT_1;
								dprintf( "Train %d init_2 revert to init_1\n", current_train->id );
							}
							break;
						case TRAIN_STATE_INIT_3:
							current_sensor = track_graph + node_map[ last_sensor_group ][ last_sensor_id ];
							if( current_sensor != current_train->next_sensor ){
								current_train->init_retry += 1;
								dprintf( "Train %d init_3 fail %d\n", current_train->id, current_train->init_retry );
							} else {
								current_train->last_sensor = current_sensor;
								current_train->next_sensor = track_next_sensor( current_train->last_sensor, switch_table );
								train_next_possible( current_train, switch_table );
								train_expect_sensors( current_train, sensor_expect );

								current_train->init_retry = 0;

								/* Let tracking update the train states corresponding to the first sensor */
								train_tracking_new_sensor( current_train, sensor_data.last_sensor_time, current_time );

								/* Force tracking */
								current_train->state = TRAIN_STATE_TRACKING;

								/* Allow tracking to track the train */
								current_train->init_state = TRAIN_STATE_INIT_4;
								current_train->init_speed_timeout = current_time + TRAIN_AUTO_REG_SPEED_CALIB_TIME;

								dprintf( "Train %d init_3 pass\n", current_train->id );
							}
							if( !( current_train->init_retry < TRAIN_AUTO_REG_RETRY ) ){
								current_train->init_state = TRAIN_STATE_INIT_1;
								dprintf( "Train %d init_3 revert to init_1\n", current_train->id );
							}
							break;
						case TRAIN_STATE_STOP:
							// this train should not move
							break;
						default:
							if( train_loc_is_sensor_tripped( &sensor_data, current_train->next_sensor ) ){
								sensor_trust( current_train->next_sensor );
								train_forget_sensors( current_train, sensor_expect );
								current_train->last_sensor = current_train->next_sensor;
								dprintf( "train hit primary %c%d\n", current_train->last_sensor->group+'A', current_train->last_sensor->id+1);
							} else if ( train_loc_is_sensor_tripped( &sensor_data, current_train->secondary_sensor ) ) {
								sensor_trust( current_train->secondary_sensor );
								sensor_error( current_train->next_sensor );
								train_forget_sensors( current_train, sensor_expect );
								current_train->last_sensor = current_train->secondary_sensor;
								current_train->tracking.trav_distance += track_next_sensor_distance( current_train->last_sensor, switch_table );
								dprintf( "train hit secondary %c%d\n", current_train->last_sensor->group+'A', current_train->last_sensor->id+1);
							} else if ( train_loc_is_sensor_tripped( &sensor_data, current_train->tertiary_sensor ) ) {
								sensor_trust( current_train->tertiary_sensor );
								// TODO switch error
								train_forget_sensors( current_train, sensor_expect );
								current_train->last_sensor = current_train->tertiary_sensor;
								current_train->tracking.trav_distance = current_train->tertiary_distance;
								dprintf( "train hit tertiary %c%d\n", current_train->last_sensor->group+'A', current_train->last_sensor->id+1);
							}

							if( train_loc_is_sensor_tripped( &sensor_data, current_train->next_sensor ) ||
							    train_loc_is_sensor_tripped( &sensor_data, current_train->secondary_sensor ) ||
							    train_loc_is_sensor_tripped( &sensor_data, current_train->tertiary_sensor ) ){
								current_train->next_sensor = track_next_sensor( current_train->last_sensor, switch_table );

								/* UI update have to come before train update */
								tracking_ui_chkpnt( tracking_ui_tid, current_train->id,
										    current_train->last_sensor->group, current_train->last_sensor->id,
										    current_time + train_tracking_eta( current_train ),
										    train_tracking_eta( current_train ) );

								dprintf( "train traveled %d\n", current_train->tracking.trav_distance );

								train_tracking_new_sensor( current_train, sensor_data.last_sensor_time, current_time );

								train_next_possible( current_train, switch_table );
								train_expect_sensors( current_train, sensor_expect );
							}

							break;
						}
					}

					if( request.type == TRAIN_AUTO_WAKEUP ){
						switch( current_train->init_state ){
						default:
							train_tracking_update( current_train, current_time );
							// TODO: if sensor has not hit any sensor for a long time => lost the train

						case TRAIN_STATE_INIT_1:
						case TRAIN_STATE_INIT_2:
						case TRAIN_STATE_INIT_3:
							break;
						}

						switch( current_train->init_state ){
						case TRAIN_STATE_INIT_4:
							if( current_time >= current_train->init_speed_timeout ){
								current_train->init_state = TRAIN_STATE_INIT_5;
								current_train->init_speed_timeout = current_time + TRAIN_AUTO_REG_SPEED_CALIB_TIME + SPEED_CHANGE_TIME;
							
								train_set_speed( module_tid, current_train->id, TRAIN_AUTO_REG_SPEED_2 );
							
								/* Compose speed change request */
								train_auto_recompose_set_speed( &request, current_train->id, TRAIN_AUTO_REG_SPEED_2 );

								/* Process the composed request */
								reprocess = 1;

								/* Break the loop */
								temp = available_train;

								dprintf( "Train %d init_4 pass\n", current_train->id );
							}
							break;
						case TRAIN_STATE_INIT_5:
							if( current_time >= current_train->init_speed_timeout ){
								current_train->init_state = TRAIN_STATE_INIT_DONE;

								train_tracking_init_calib( current_train );
								train_set_speed( module_tid, current_train->id, 0 );

								/* Compose speed change request */
								train_auto_recompose_set_speed( &request, current_train->id, 0 );

								/* Process the composed request */
								reprocess = 1;

								/* Break the loop */
								temp = available_train;

								dprintf( "Train %d init_5 pass\n", current_train->id );
							}
							break;
						}
					}

					switch( current_train->init_state ){
					default:
						tracking_ui_landmrk( tracking_ui_tid, current_train->id,
								     current_train->check_point->group, current_train->check_point->id );
						tracking_ui_dist( tracking_ui_tid, current_train->id, train_tracking_position( current_train ) );
						tracking_ui_speed( tracking_ui_tid, current_train->id, train_tracking_speed_in_sec( current_train ) );
						tracking_ui_nextmrk( tracking_ui_tid, current_train->id,
								     current_train->next_check_point->group, current_train->next_check_point->id,
								     train_tracking_eta( current_train ) );
					case TRAIN_STATE_INIT_1:
					case TRAIN_STATE_INIT_2:
					case TRAIN_STATE_INIT_3:
						break;
					}

					sem_release( current_train->sem );
				}
			}
		} while( reprocess );

		/* Late reply */
		switch( request.type ){
		case TRAIN_AUTO_QUERY_SWITCH_DIR:
		case TRAIN_AUTO_QUERY_LAST_SENSOR:
			status = Reply( tid, ( char* )&reply, sizeof( reply ) );
			assert( status == SYSCALL_SUCCESS );
		default:
			break;
		}
	}
}

static int train_auto_request( int tid, Train_auto_request* data, uint size, int* group, int* id )
{
	int status;
	Train_auto_reply reply;

	status = Send( tid, ( char* )data, size, ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );

	if( group ){
		*group = reply.group;
	}
	if( id ){
		*id = reply.id;
	}

	return ERR_NONE;
}

int train_auto_new_sensor_data( int tid, Sensor_data* data )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_NEW_SENSOR_DATA;

	/* Very unfortunate */
	memcpy( ( uchar* )&request.data.sensor_data, ( uchar* )data, sizeof( Sensor_data ) );

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.sensor_data ), 0, 0 );
}

static int train_auto_wakeup( int tid )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_WAKEUP;

	return train_auto_request( tid, &request, sizeof( uint ), 0, 0 );
}

int train_auto_init( int tid, uint track )
{
	Train_auto_request request;
	
	request.type = TRAIN_AUTO_INIT;
	request.data.init.track_id = track;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.init ), 0, 0 );
}

int train_auto_new_train( int tid, uint id )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_NEW_TRAIN;
	request.data.new_train.train_id = id;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.new_train ), 0, 0 );
}

int train_auto_set_speed( int tid, uint id, uint speed_level )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_SET_TRAIN_SPEED;
	request.data.set_speed.train_id = id;
	request.data.set_speed.speed_level = speed_level;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.set_speed ), 0, 0 );
}

int train_auto_set_reverse( int tid, uint id )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_SET_TRAIN_REVERSE;

	request.data.set_reverse.train_id = id;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.set_reverse ), 0, 0 );
}

int train_auto_set_switch( int tid, uint id, char direction )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_SET_SWITCH_DIR;
	request.data.set_switch.switch_id = id;
	request.data.set_switch.direction = direction;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.set_switch ), 0, 0 );
}

int train_auto_set_switch_all( int tid, char direction )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_SET_ALL_SWITCH;
	request.data.set_switch_all.direction = direction;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.set_switch_all ), 0, 0 );
}

int train_auto_query_switch( int tid, uint id, int* direction )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_QUERY_SWITCH_DIR;
	request.data.query_switch.switch_id = id;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.query_switch ), direction, 0 );
}

int train_auto_query_sensor( int tid, int* group, int* id )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_QUERY_LAST_SENSOR;
	
	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.query_sensor ), group, id );
}

int train_auto_hit_and_stop( int tid, int train_id, int group, int id, int distance )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_HIT_AND_STOP;
	request.data.hit_and_stop.train_id = train_id;
	request.data.hit_and_stop.group = group;
	request.data.hit_and_stop.id = id;
	request.data.hit_and_stop.distance = distance;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.hit_and_stop ), 0, 0 );
}

int train_auto_plan( int tid, int train_id, int group, int id, int dist_pass )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_PLAN;
	request.data.plan.train_id = train_id;
	request.data.plan.group = group;
	request.data.plan.id = id;
	request.data.plan.dist_pass = dist_pass;

	return train_auto_request( tid, &request, sizeof( uint ) + sizeof( request.data.plan ), 0, 0 );
}
