#include <types.h>
#include <err.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/clock_server.h>
#include <user/devices/clock.h>
#include <user/uart.h>
#include <user/lib/sync.h>
#include <user/courier.h>
#include <lib/str.h>
#include <user/display.h>
#include <user/name_server.h>
#include "inc/config.h"
#include "inc/sensor_data.h"
#include "inc/train.h"
#include "inc/warning.h"

enum Sensor_query_type {
	SENSOR_QUERY_RENEW,
	SENSOR_QUERY_RECENT
};

typedef struct Sensor_query_request_s {
	uint type;
	Sensor_data** sensor_data;
} Sensor_query;

typedef struct Sensor_query_reply_s {
	int group;
	int id;
} Sensor_reply;

static int sensor_new_data( int tid, int data );

int sensor_id2name( char* str, uchar group, uchar id )
{
	int status;
	
	status = sprintf( str, "%c%02u", group / 2 + 'A', ( group % 2 ) * BITS_IN_BYTE + id + 1 );
	assert( status > 0 );

	return ERR_NONE;
}

int sensor_name2id( char* str, int* group, int* id )
{
	*group = ( str[ 0 ] - 'A' ) * 2;
	*id = stou( str + 1 ) - 1;

	if( *id > 15 || *id < 0 ){
		return ERR_INVALID_ARGUMENT;
	}

	if( *id > 7 ){
		*group += 1;
		*id %= 8;
	}

	if( *group >= SENSOR_BYTE_COUNT ){
		return ERR_INVALID_ARGUMENT;
	}

	return ERR_NONE;
}

static void sensor_query_server()
{
	Sensor_query query = {0};
	Sensor_reply reply = {0};
	volatile Sensor_data volatile ** sensor_data = 0;
	int tid;
	int status;

	status = RegisterAs( SENSOR_QUERY_NAME );
	assert( status == REGISTER_AS_SUCCESS );

	while( 1 ){
		status = Receive( &tid, ( char* )&query, sizeof( query ) );
		switch( query.type ){
		case SENSOR_QUERY_RENEW:
			assert( status == sizeof( query ) );
			sensor_data = query.sensor_data;
			break;
		case SENSOR_QUERY_RECENT:
			assert( status == sizeof( query.type ) );
			if( sensor_data ){
				reply.group = (*sensor_data)->last_sensor_group;
				reply.id = (*sensor_data)->last_sensor_id;
			} else {
				reply.group = -1;
				reply.id = -1;
			}
			break;
		default:
			assert( 0 );
		}

		status = Reply( tid, ( char* )&reply, sizeof( reply ) );
		assert( status == SYSCALL_SUCCESS );
	}
	
	Exit();
}

static int sensor_query_request( int tid, uint type, Sensor_data** data, int* id )
{
	Sensor_query query;
	Sensor_reply reply;
	int size;
	int status;

	if( type == SENSOR_QUERY_RENEW ){
		size = sizeof( query );
	} else if( type == SENSOR_QUERY_RECENT ){
		size = sizeof( query.type );
	}

	query.type = type;
	query.sensor_data = data;

	status = Send( tid, ( char* )&query, size, ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );

	if( id ){
		*( ( int* )data ) = reply.group;
		*id = reply.id;
	}

	return ERR_NONE;
}

static int sensor_query_renew( int tid, Sensor_data** data )
{
	return sensor_query_request( tid, SENSOR_QUERY_RENEW, data, 0 );
}

int sensor_query_recent( int tid, int* group, int* id )
{
	return sensor_query_request( tid, SENSOR_QUERY_RECENT, ( Sensor_data* )group, id );
}

void train_sensor()
{
	Sensor_data data_buffer[ 2 ] = {0};
	Sensor_data* sensor_data_old = data_buffer;
	Sensor_data* sensor_data = data_buffer + 1;
	Sensor_data* temp = 0;
	int sensor_notifiee[ 5 ];
	int* sensor_ui_tid = sensor_notifiee;
	int* train_mod_tid = sensor_notifiee + 1;
	int* train_auto_tid = sensor_notifiee + 2;
	int* notified = sensor_notifiee + 3;
	int* query_tid = sensor_notifiee + 4;
	int group, id;
	int need_update = 0;
	int courier_tid = Create( TRAIN_SENSOR_PRIORITY, courier );
	int status;
		
	*sensor_ui_tid = WhoIs( SENSOR_UI_NAME );
	*train_mod_tid = WhoIs( TRAIN_MODULE_NAME );
	// *train_auto_tid = WhoIs( TRAIN_AUTO_NAME );
	*notified = 1;
	*query_tid = Create( TRAIN_SENSOR_PRIORITY, sensor_query_server );

	courier_init( courier_tid, sensor_new_data );

	status = sensor_query_renew( *query_tid, &sensor_data_old );
	assert( status == ERR_NONE );
	
	while( 1 ){
		status = train_all_sensor( *train_mod_tid );
		assert( status == 0 );

		/* Obtain sensor data */
		for( group = 0; group < SENSOR_BYTE_COUNT; group += 1 ){
			sensor_data->sensor_raw[ group ] = Getc( COM_1 );
		}
		
		for( group = 0; group < SENSOR_BYTE_COUNT; group += 1 ){
			if( sensor_data->sensor_raw[ group ] != sensor_data_old->sensor_raw[ group ] ){
				need_update = 1;
			}
			for( id = 0; id < 8; id += 1 ) {
				if ( sensor_data->sensor_raw[ group ] & ( ( 0x1 << 7 ) >> id ) ){
					sensor_data->last_sensor_group = group;
					sensor_data->last_sensor_id = id;
				}
			}
		}

		/* Notify all other tasks about the update */
		if( *notified && need_update ){
			temp = sensor_data_old;
			sensor_data_old = sensor_data;
			sensor_data = temp;
			courier_go( courier_tid, ( int )sensor_notifiee, ( int )sensor_data_old );
			*notified = 0;
			need_update = 0;
		}
	}

	Kill( courier_tid );
	Kill( *query_tid );

	Exit();
}

static int sensor_new_data( int tid, int data )
{
	int* sensor_notifiee = ( int* )tid;
	int* sensor_ui_tid = sensor_notifiee;
	int* train_mod_tid = sensor_notifiee + 1;
	int* train_auto_tid = sensor_notifiee + 2;
	int* notified = sensor_notifiee + 3;
	int* query_tid = sensor_notifiee + 4;
	Sensor_data* sensor_data = ( Sensor_data* )data;
	int status;

	status = sensor_ui_update( *sensor_ui_tid, sensor_data );
	assert( status == ERR_NONE );

	*notified = 1;

	return ERR_NONE;
}
