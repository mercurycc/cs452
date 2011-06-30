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
#include "inc/train.h"
#include "inc/config.h"
#include "inc/sensor_data.h"
#include "inc/track_data.h"
#include "inc/track_node.h"
#include "inc/warning.h"
#include "inc/train_location.h"
#include "inc/train_types.h"


enum Train_state {
	TRAIN_STATE_INIT,             /* Init */
	TRAIN_STATE_STOP,             /* Stopped */
	TRAIN_STATE_TRACKING,         /* Normal state */
	TRAIN_STATE_REVERSE,          /* Just reversed direction */
	TRAIN_STATE_SPEED_CHANGE,     /* Just changed speed */
	TRAIN_STATE_SPEED_ERROR,      /* Speed prediction out of bound error */
	TRAIN_STATE_SWITCH_ERROR,     /* Switch prediction error */
	TRAIN_STATE_UNKNOW            /* Unknown state */
};

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
	TRAIN_AUTO_PLAN
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
			uint pickup;
			uint next_group;  /* The sensor the pickup is heading towards */
			uint next_id;
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
		Delay( 2 );

		train_auto_wakeup( ptid );
	}
}

void train_auto()
{
	Train_auto_request request;
	Train_auto_reply reply;
	Sensor_data sensor_data;
	int last_sensor_group = -1;
	int last_sensor_id = -1;
	int tracking_ui_tid;
	int alarm_tid;
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
	int switch_table[ NUM_SWITCHES ] = { 0 };
	int module_tid;
	uint current_time;
	uint current_distance;
	int status;

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
	
	/* Start alarm */
	alarm_tid = Create( TRAIN_AUTO_PRIROTY, train_auto_alarm );
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
		case TRAIN_AUTO_PLAN:
			status = Reply( tid, ( char* )&reply, sizeof( reply ) );
			assert( status == SYSCALL_SUCCESS );
		default:
			break;
		}

		/* Process request */
		switch( request.type ){
		case TRAIN_AUTO_NEW_SENSOR_DATA:
			/* Unfortuate memcpy */
			memcpy( ( uchar* )&sensor_data, ( uchar* )&request.data.sensor_data, sizeof( sensor_data ) );

			//test_sensor = 0;

			/* Update last triggered sensor */
			for( temp = 0; temp < SENSOR_BYTE_COUNT; temp += 1 ){
				if( sensor_data.sensor_raw[ temp ] ){
					last_sensor_group = temp;
					for( i = 0; i < BITS_IN_BYTE; i += 1 ){
						if( sensor_data.sensor_raw[ temp ] & ( ( 1 << 7 ) >> i ) ){
							last_sensor_group = temp / 2;
							last_sensor_id = i + ( temp % 2 ) * 8;
						}
					}
				}
			}
			break;
		case TRAIN_AUTO_NEW_TRAIN:
			current_train = trains + available_train;
			train_map[ request.data.new_train.train_id ] = available_train;
			available_train += 1;
			current_train->id = request.data.new_train.train_id;
			current_train->state = TRAIN_STATE_INIT;
			current_train->pickup = request.data.new_train.pickup;
			current_train->distance = 0;
			current_train->speed.numerator = 0;
			current_train->speed.denominator = 1;
			current_train->speed_level = 0;
			current_train->old_speed_level = 0;
			current_train->last_sensor_time = current_time;
			for ( i = 0; i < NUM_SPEED_LEVEL; i++ ) {
				current_train->speed_table[i].numerator = TRAIN_SPEED_PRED_FACTOR( i )*i;
				current_train->speed_table[i].denominator = 2;
				current_train->speed_count[i] = 0;
			}

			current_train->last_sensor = track_graph + node_map[ request.data.new_train.next_group ][ request.data.new_train.next_id ];
			current_train->check_point = track_graph + node_map[ request.data.new_train.next_group ][ request.data.new_train.next_id ];
			current_train->next_check_point = 0;
			current_train->next_sensor = track_next_sensor( current_train->last_sensor->reverse, switch_table );
			current_train->stop_sensor = 0;
			current_train->auto_command = 0;
			current_train->track_graph = track_graph;
			current_train->switch_table = switch_table;

			current_train->planner_tid = Create( TRAIN_PLAN_PRIORITY, train_planner );
			assert( current_train->planner_tid > 0 );

			status = train_planner_init( current_train->planner_tid, current_train );
			assert( status == ERR_NONE );

			train_set_speed( module_tid, request.data.new_train.train_id, TRAIN_AUTO_REGISTER_SPEED );

			tracking_ui_new_train( tracking_ui_tid, current_train->id );
			
			break;
		case TRAIN_AUTO_SET_TRAIN_SPEED:
			current_train = trains + train_map[ request.data.set_speed.train_id ];
			if ( current_train->speed_level / 2 == request.data.set_speed.speed_level ) {
				break;
			}
			current_train->old_speed_level = current_train->speed_level;
			if ( current_train->old_speed_level / 2 < request.data.set_speed.speed_level ) {
				current_train->speed_level = request.data.set_speed.speed_level * 2;
			}
			else if ( current_train->old_speed_level > request.data.set_speed.speed_level ) {
				current_train->speed_level = request.data.set_speed.speed_level * 2+1;
			}
			current_train->state = TRAIN_STATE_SPEED_CHANGE;
			if ( current_train->speed_count[ current_train->speed_level ] ) {
				current_train->speed.numerator = current_train->speed_table[ current_train->speed_level ].numerator;
				current_train->speed.denominator = current_train->speed_table[ current_train->speed_level ].denominator;
			}
			else {
				// TODO: prediction
				current_train->speed.numerator = current_train->speed_table[ current_train->speed_level ].numerator;
				current_train->speed.denominator = current_train->speed_table[ current_train->speed_level ].denominator;
			}
			current_train->speed_change_time_stamp = current_time;
			WAR_PRINT( "speed change from %d to %d\n", current_train->old_speed_level, current_train->speed_level );
			break;
		case TRAIN_AUTO_SET_TRAIN_REVERSE:
			current_train = trains + train_map[ request.data.set_speed.train_id ];
			if( current_train->pickup == TRAIN_PICKUP_FRONT ){
				current_train->pickup = TRAIN_PICKUP_BACK;
			} else if( current_train->pickup == TRAIN_PICKUP_BACK ){
				current_train->pickup = TRAIN_PICKUP_FRONT;
			}
			/* TODO: This has not update the distance yet */
			current_train->check_point = track_previous_node( current_train->check_point, switch_table );
			current_train->next_check_point = track_next_node( current_train->check_point, switch_table );
			current_sensor = current_train->last_sensor;
			current_train->last_sensor = current_train->next_sensor->reverse;
			current_train->next_sensor = track_next_sensor( current_train->last_sensor, switch_table );
			current_train->state = TRAIN_STATE_REVERSE;
			break;
		case TRAIN_AUTO_SET_SWITCH_DIR:
			switch_table[ SWID_TO_ARRAYID( request.data.set_switch.switch_id ) ] = request.data.set_switch.direction;
			break;
		case TRAIN_AUTO_SET_ALL_SWITCH:
			for( i = 0; i < NUM_SWITCHES; i += 1 ){
				switch_table[ i ] = request.data.set_switch_all.direction;
			}
		case TRAIN_AUTO_QUERY_SWITCH_DIR:
			reply.group = switch_table[ SWID_TO_ARRAYID( request.data.set_switch.switch_id ) ];
			break;
		case TRAIN_AUTO_QUERY_LAST_SENSOR:
			reply.group = last_sensor_group;
			reply.id = last_sensor_id;
			break;
		case TRAIN_AUTO_HIT_AND_STOP:
			current_train = trains + train_map[ request.data.hit_and_stop.train_id ];
			current_train->stop_sensor = track_graph + node_map[ request.data.hit_and_stop.group ][ request.data.hit_and_stop.id ];
			break;
		case TRAIN_AUTO_PLAN:
			current_train = trains + train_map[ request.data.hit_and_stop.train_id ];
			train_planner_path_plan( current_train->planner_tid,
						 track_graph + node_map[ request.data.hit_and_stop.group ][ request.data.hit_and_stop.id ],
						 request.data.plan.dist_pass );
			break;
		}

		/* Late reply */
		switch( request.type ){
		case TRAIN_AUTO_QUERY_SWITCH_DIR:
		case TRAIN_AUTO_QUERY_LAST_SENSOR:
			status = Reply( tid, ( char* )&reply, sizeof( reply ) );
			assert( status == SYSCALL_SUCCESS );
		default:
			break;
		}

		/* Process train states */
		if( request.type == TRAIN_AUTO_NEW_SENSOR_DATA ){
			for( temp = 1; temp < available_train; temp += 1 ){
				current_train = trains + temp;
				switch( current_train->state ){
				case TRAIN_STATE_INIT:
					/* For init only one sensor can be triggered by the init train.  Safe and easy assumption, not an ideal one though. */
					if( train_loc_is_sensor_tripped( &sensor_data, current_train->last_sensor ) ){
						current_train->pickup = TRAIN_PICKUP_FRONT;
						clear_sensor_data( &sensor_data, current_train->last_sensor );
					} else if( train_loc_is_sensor_tripped( &sensor_data, current_train->next_sensor ) ){
						current_train->pickup = TRAIN_PICKUP_BACK;
						current_train->last_sensor = current_train->next_sensor;
						clear_sensor_data( &sensor_data, current_train->next_sensor );
					} else {
						break;
					}

					if ( current_time - current_train->last_sensor_time > SPEED_CHANGE_TIME ) {
						current_train->old_speed_level = TRAIN_AUTO_REGISTER_SPEED;
					}
					else {
						current_train->old_speed_level = TRAIN_AUTO_REGISTER_SPEED * 2 * (current_time - current_train->last_sensor_time + SENSOR_AVERAGE_DELAY) / SPEED_CHANGE_TIME;
					}
					current_train->speed_level = 0;

					current_train->last_sensor_time = sensor_data.last_sensor_time;
					current_train->check_point = current_train->last_sensor;
					current_train->next_check_point = track_next_node( current_train->check_point, switch_table );
					current_train->last_check_point_time = sensor_data.last_sensor_time;
					current_train->next_sensor = track_next_sensor( current_train->last_sensor, switch_table );
					current_train->last_eta_time_stamp = 0;
					
					train_set_speed( module_tid, current_train->id, 0 );
					current_train->state = TRAIN_STATE_SPEED_CHANGE;


					WAR_PRINT( "reg train %d pickup %x hit sensor %c%d next sensor %c%d\n", current_train->id, current_train->pickup, current_train->last_sensor->group+'A', current_train->last_sensor->id+1, current_train->next_sensor->group+'A', current_train->next_sensor->id+1 );
					break;
				case TRAIN_STATE_SPEED_CHANGE:
				case TRAIN_STATE_TRACKING:
					/* FIXME: remove me */
					if( current_train->stop_sensor && train_loc_is_sensor_tripped( &sensor_data, current_train->stop_sensor ) ){
						train_set_speed( module_tid, current_train->id, 0 );
					}
					if( train_loc_is_sensor_tripped( &sensor_data, current_train->next_sensor ) ){
						int changing = 0;
						
						if( current_train->state == TRAIN_STATE_TRACKING ){
							/* Speed update for speed_change is handled by periodic update, sensor only helps with location */
							status = update_train_speed( current_train, current_train->next_sensor, sensor_data.last_sensor_time );
							assert( status == 0 );
						}
						else { // speed change

							if ( current_time - current_train->speed_change_time_stamp >= SPEED_CHANGE_TIME ) {
								if ( current_train->speed_level > 1 ) {
									current_train->state = TRAIN_STATE_TRACKING;
								}
								else {
									current_train->state = TRAIN_STATE_STOP;
								}
							} else {
								changing = 1;
							}
						}

						if ( current_train->last_eta_time_stamp ) {
							current_train->next_sensor_eta -= current_time - current_train->last_eta_time_stamp;
							current_train->last_eta_time_stamp = current_time;
						}


						WAR_PRINT( "train %d in speed level %d speed %d / %d, eta %d\n", current_train->id, current_train->speed_level, current_train->speed.numerator, current_train->speed.denominator, current_train->next_sensor_eta );

						current_train->distance = 0;
						current_train->remaining_distance = 0;
						current_train->last_sensor = current_train->next_sensor;
						current_train->next_sensor = track_next_sensor( current_train->next_sensor, switch_table );
						/* TODO: deal with when next sensor cannot find, meaning heading to an exit */
						current_train->check_point = current_train->last_sensor;
						current_train->next_check_point = track_next_node( current_train->check_point, switch_table );

						/* Update Time Diff */
						tracking_ui_chkpnt( tracking_ui_tid, current_train->id,
								    current_train->last_sensor->group,
								    current_train->last_sensor->id,
								    sensor_data.last_sensor_time + current_train->next_sensor_eta,
								    current_train->next_sensor_eta );
						
						/* Update eta */
						if ( changing ) {
							int dist = sensor_distance( current_train->last_sensor, current_train->next_sensor );
							int modifier = 0;
							int time_left = current_train->speed_change_time_stamp + SPEED_CHANGE_TIME - current_time;

							int c = current_train->speed_table[ current_train->speed_level ].numerator;
							int d = current_train->speed_table[ current_train->speed_level ].denominator;
							int e = current_train->speed.numerator;
							int f = current_train->speed.denominator;

							unsigned long long int finish_top = ( e * d + c * f ) * time_left;
							unsigned long long int finish_bot = 2 * d * f;
							int finish = finish_top / finish_bot;
							
							if ( finish > dist ) {
								// approximated
								modifier = time_left * finish / dist;
							}
							else {
								modifier = ( dist - finish ) * current_train->speed.denominator / current_train->speed.numerator;
							}
							current_train->next_sensor_eta = time_left + modifier;
						}
						else {
							current_train->next_sensor_eta = sensor_distance( current_train->last_sensor, current_train->next_sensor ) * current_train->speed.denominator / current_train->speed.numerator ;
						}
						current_train->last_eta_time_stamp = current_time;

						/* Update UI */
						tracking_ui_landmrk( tracking_ui_tid, current_train->id,
								     current_train->check_point->group, current_train->check_point->id );

						tracking_ui_nextmrk( tracking_ui_tid, current_train->id,
								     current_train->next_sensor->group, current_train->next_sensor->id,
								     //current_train->check_point->edge[DIR_AHEAD].dist * current_train->speed.denominator / current_train->speed.numerator );
								     current_train->next_sensor_eta );

						tracking_ui_speed( tracking_ui_tid, current_train->id,
								   current_train->speed.numerator * 100 / current_train->speed.denominator );
						tracking_ui_dist( tracking_ui_tid, current_train->id, current_train->distance );
						//WAR_PRINT( "update train %d on sensor %c%d\n", current_train->id, current_train->last_sensor->group+'A', current_train->last_sensor->id+1 );
					}
					break;
				default:
					break;
				}
			}
		}
		
		if( request.type == TRAIN_AUTO_WAKEUP ){// || request.type == TRAIN_AUTO_NEW_SENSOR_DATA ){
			for( temp = 1; temp < available_train; temp += 1 ){
				current_train = trains + temp;
				int a = current_train->speed_table[ current_train->old_speed_level ].numerator;
				int b = current_train->speed_table[ current_train->old_speed_level ].denominator;
				int c = current_train->speed_table[ current_train->speed_level ].numerator;
				int d = current_train->speed_table[ current_train->speed_level ].denominator;
				int diff = current_time - current_train->speed_change_time_stamp;
				unsigned long long int top;
				unsigned long long int bottom;
				int t;
				Speed old_speed;

				switch( current_train->state ){
				case TRAIN_STATE_SPEED_CHANGE:
					if ( current_time > current_train->speed_change_time_stamp + SPEED_CHANGE_TIME ) {
						if ( current_train->speed_level <= 1 ) {
							current_train->state = TRAIN_STATE_STOP;
						}
						current_train->speed.numerator = current_train->speed_table[ current_train->speed_level ].numerator;
						current_train->speed.denominator = current_train->speed_table[ current_train->speed_level ].denominator;
						t = current_time - current_train->last_eta_time_stamp;
						current_train->distance += t * current_train->speed.numerator / current_train->speed.denominator;
					}
					else {
						old_speed.numerator = current_train->speed.numerator;
						old_speed.denominator = current_train->speed.denominator;

						top = b * diff * c + a * SPEED_CHANGE_TIME * d - a * diff * d;
						bottom = b * SPEED_CHANGE_TIME * d;

						while ( top > 10000 && bottom > 10000 ) {
							top = top / 10;
							bottom = bottom / 10;
						}
						current_train->speed.numerator = top;
						current_train->speed.denominator = bottom;

						t = current_time - current_train->last_eta_time_stamp;
						current_train->distance += (old_speed.numerator * t * current_train->speed.denominator + current_train->speed.numerator * t * old_speed.denominator ) / 2 * old_speed.denominator * current_train->speed.denominator;
					}
					break;
				case TRAIN_STATE_TRACKING:
					current_train->distance = current_train->speed.numerator *
						( current_time - current_train->last_check_point_time ) / current_train->speed.denominator;
					break;
				case TRAIN_STATE_REVERSE:
					/* TODO: Either update the distance here or up above */
					break;
				case TRAIN_STATE_SPEED_ERROR:
					break;
				case TRAIN_STATE_SWITCH_ERROR:
					break;
				case TRAIN_STATE_UNKNOW:
					break;
				}

				switch( current_train->state ){
				case TRAIN_STATE_SPEED_CHANGE:
				case TRAIN_STATE_TRACKING:
					current_distance = node_distance( current_train->check_point, switch_table );
					if( current_train->distance > current_distance ){
						current_train->check_point = current_train->next_check_point;
						current_train->next_check_point = track_next_node( current_train->check_point, switch_table );
						current_train->last_check_point_time = current_time;
						current_train->distance -= current_distance;
					}
					// update eta
					if ( current_train->last_eta_time_stamp ) {
						current_train->next_sensor_eta -= current_time - current_train->last_eta_time_stamp;
						current_train->last_eta_time_stamp = current_time;
					}

					/* Update UI */	
					tracking_ui_landmrk( tracking_ui_tid, current_train->id,
							     current_train->check_point->group, current_train->check_point->id );
					tracking_ui_dist( tracking_ui_tid, current_train->id, current_train->distance );

					break;
				case TRAIN_STATE_STOP:
					current_train->next_sensor_eta = 0;
					/* Update UI */	
					tracking_ui_landmrk( tracking_ui_tid, current_train->id,
							     current_train->check_point->group, current_train->check_point->id );
					tracking_ui_dist( tracking_ui_tid, current_train->id, current_train->distance );

					break;
				}
			}
		}
		
		/* Really late reply */
		switch( request.type ){
		case TRAIN_AUTO_WAKEUP:
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

int train_auto_new_train( int tid, uint id, uint pre_grp, uint pre_id )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_NEW_TRAIN;
	request.data.new_train.train_id = id;
	request.data.new_train.pickup = TRAIN_PICKUP_FRONT;
	request.data.new_train.next_group = pre_grp;
	request.data.new_train.next_id = pre_id;

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

	request.type = TRAIN_AUTO_QUERY_SWITCH_DIR;

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

int train_auto_hit_and_stop( int tid, int train_id, int group, int id )
{
	Train_auto_request request;

	request.type = TRAIN_AUTO_HIT_AND_STOP;
	request.data.hit_and_stop.train_id = train_id;
	request.data.hit_and_stop.group = group;
	request.data.hit_and_stop.id = id;

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
