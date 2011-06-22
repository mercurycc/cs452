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
#include <user/name_server.h>
#include "inc/config.h"
#include "inc/sensor_data.h"
#include "inc/train.h"

static int sensor_new_data( int tid, int data );

int sensor_id2name( char* str, uchar group, uchar id )
{
	int status;
	
	status = sprintf( str, "%c%2u", group / 2 + 'A', ( group % 2 ) * BITS_IN_BYTE + id + 1 );
	assert( status == SENSOR_NAME_LENGTH );

	return ERR_NONE;
}

void train_sensor()
{
	Sensor_data data_buffer[ 2 ];
	Sensor_data* sensor_data_old = data_buffer;
	Sensor_data* sensor_data = data_buffer + 1;
	Sensor_data* temp = 0;
	int sensor_notifiee[ 4 ];
	int* sensor_ui_tid = sensor_notifiee;
	int* train_mod_tid = sensor_notifiee + 1;
	int* train_auto_tid = sensor_notifiee + 2;
	int* notified = sensor_notifiee + 3;
	int quit = 0;
	int group, id;
	int need_update;
	int courier_tid = Create( TRAIN_SENSOR_PRIORITY, courier );
	int status;
	
	*sensor_ui_tid = WhoIs( SENSOR_UI_NAME );
	*train_mod_tid = WhoIs( TRAIN_MODULE_NAME );
	*train_auto_tid = WhoIs( TRAIN_AUTO_NAME );
	*notified = 0;

	courier_init( courier_tid, sensor_new_data );
	
	while ( !quit ) {
		need_update = 0;
		
		status = train_all_sensor( *train_mod_tid );
		assert( status == 0 );

		/* Obtain sensor data */
		for( group = 0; group < SENSOR_BYTE_COUNT; group += 1 ){
			sensor_data->sensor_raw[ group ] = Getc( COM_1 );
		}

		for( group = 0; group < SENSOR_BYTE_COUNT; group += 1 ){
			for( id = 0; id < 8; id += 1 ) {
				if ( sensor_data->sensor_raw[ group ] & ( ( 0x1 << 7 ) >> id ) ){
					need_update = 1;
					sensor_data->last_sensor_group = group;
					sensor_data->last_sensor_id = id;
				}
			}
		}

		/* Notify all other tasks about the update */
		if( *notified ){
			temp = sensor_data_old;
			sensor_data_old = sensor_data;
			sensor_data = temp;
			courier_go( courier_tid, ( int )sensor_notifiee, ( int )sensor_data );
			*notified = 0;
		}
	}

	Exit();
}

static int sensor_new_data( int tid, int data )
{
	int* sensor_notifiee = ( int* )tid;
	int* sensor_ui_tid = sensor_notifiee;
	int* train_mod_tid = sensor_notifiee + 1;
	int* train_auto_tid = sensor_notifiee + 2;
	int* notified = sensor_notifiee + 3;
	Sensor_data* sensor_data = ( Sensor_data* )data;
	int status;

	status = sensor_ui_update( *sensor_ui_tid, sensor_data );
	assert( status == ERR_NONE );

	*notified = 1;

	return ERR_NONE;
}

