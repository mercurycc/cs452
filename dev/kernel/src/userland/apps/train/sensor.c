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
#include <user/time.h>
#include <perf.h>
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

void train_sensor()
{
	Sensor_data data_buffer[ 2 ] = {{{0}}};
	Sensor_data* sensor_data_old = data_buffer;
	Sensor_data* sensor_data = data_buffer + 1;
	Sensor_data* temp = 0;
	int sensor_notifiee[ 5 ];
	int* sensor_ui_tid = sensor_notifiee;
	int* train_mod_tid = sensor_notifiee + 1;
	int* train_auto_tid = sensor_notifiee + 2;
	int* notified = sensor_notifiee + 3;
	int* query_tid = sensor_notifiee + 4;
	int group;
	int need_update = 0;
	int courier_tid = Create( TRAIN_SENSOR_PRIORITY, courier );
	int status;

	*sensor_ui_tid = WhoIs( SENSOR_UI_NAME );
	*train_mod_tid = WhoIs( TRAIN_MODULE_NAME );
	*train_auto_tid = WhoIs( TRAIN_AUTO_NAME );
	*notified = 1;

	courier_init( courier_tid, sensor_new_data );

	while( 1 ){
		status = train_all_sensor( *train_mod_tid );
		assert( status == 0 );

		/* Obtain sensor data */
		for( group = 0; group < SENSOR_BYTE_COUNT; group += 1 ){
			sensor_data->sensor_raw[ group ] = Getc( COM_1 );
		}

		/* sensor data incoming time */
		sensor_data->last_sensor_time = Time();
		
		for( group = 0; group < SENSOR_BYTE_COUNT; group += 1 ){
			if( sensor_data->sensor_raw[ group ] != sensor_data_old->sensor_raw[ group ] ){
				need_update = 1;
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
	int* train_auto_tid = sensor_notifiee + 2;
	int* notified = sensor_notifiee + 3;
	Sensor_data* sensor_data = ( Sensor_data* )data;
	int status;

	status = train_auto_new_sensor_data( *train_auto_tid, sensor_data );
	assert( status == ERR_NONE );

	status = sensor_ui_update( *sensor_ui_tid, sensor_data );
	assert( status == ERR_NONE );

	*notified = 1;

	return ERR_NONE;
}

