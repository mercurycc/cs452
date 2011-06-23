#include <types.h>
#include <err.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/clock_server.h>
#include <user/devices/clock.h>
#include <user/uart.h>
#include <user/display.h>
#include <user/lib/sync.h>
#include <user/name_server.h>
#include "../inc/sensor_data.h"
#include "../inc/config.h"
#include "../inc/train.h"
#include "../inc/warning.h"

#define SENSOR_LIST_SIZE 5

#define SENSOR_PRINT_STATE( buffer, group, id )				\
	( ( buffer.sensor_raw[ group ] & ( ( 1 << 7 ) >> id ) ) != 0 ? '#' : ' ' )

#define SENSOR_PRINT_GROUP( buffer, group )		\
	SENSOR_PRINT_STATE( buffer, group, 0 ),		\
		SENSOR_PRINT_STATE( buffer, group, 1 ),	\
		SENSOR_PRINT_STATE( buffer, group, 2 ),	\
		SENSOR_PRINT_STATE( buffer, group, 3 ),	\
		SENSOR_PRINT_STATE( buffer, group, 4 ),	\
		SENSOR_PRINT_STATE( buffer, group, 5 ),	\
		SENSOR_PRINT_STATE( buffer, group, 6 ),	\
		SENSOR_PRINT_STATE( buffer, group, 7 )


#define SENSOR_PRINTS( buffer )					\
	SENSOR_PRINT_GROUP( buffer, 0 ),			\
		SENSOR_PRINT_GROUP( buffer, 1 ),		\
		SENSOR_PRINT_GROUP( buffer, 2 ),		\
		SENSOR_PRINT_GROUP( buffer, 3 ),		\
		SENSOR_PRINT_GROUP( buffer, 4 ),		\
		SENSOR_PRINT_GROUP( buffer, 5 ),		\
		SENSOR_PRINT_GROUP( buffer, 6 ),		\
		SENSOR_PRINT_GROUP( buffer, 7 ),		\
		SENSOR_PRINT_GROUP( buffer, 8 ),		\
		SENSOR_PRINT_GROUP( buffer, 9 )
		
static int parse_sensor( char data, int group, char name[][ SENSOR_NAME_LENGTH + 1 ], int head )
{
	int i;
	int status;

	if ( data ) {
		for ( i = 0; i < BITS_IN_BYTE; i++ ) {
			if ( data & ( ( 0x1 << 7 ) >> i ) ){
				head = ( head + SENSOR_LIST_SIZE - 1 ) % SENSOR_LIST_SIZE;
				status = sensor_id2name( name[ head ], group, i );
				assert( status == ERR_NONE );
			}
		}
	}
	
	return head;
}

void sensor_ui()
{
	int quit = 0;
	int status;
	int i;
	int temp;
	int tid;
	Sensor_data sensor_buffer;
	char sensor_list[ SENSOR_LIST_SIZE ][ SENSOR_NAME_LENGTH + 1 ] = {{0}};
	int head = 0;
	Region sensor_title = { 2, 3, 1, 18 - 2, 1, 0 };
	Region sensor_reg = { 2, 4, 12 - 4, 27 - 2, 1, 1 };
	Region sensor_list_reg = { 22, 6, 1, 3, 0, 0 };
	Region warning_reg = WARNING_REGION;

	status = RegisterAs( SENSOR_UI_NAME );
	assert( status == REGISTER_AS_SUCCESS );
	
	status = region_init( &sensor_reg );
	assert( status == ERR_NONE );
	status = region_init( &sensor_title );
	assert( status == ERR_NONE );
	status = region_init( &sensor_list_reg );
	assert( status == ERR_NONE );

	/* Initialize region */
	status = region_printf( &sensor_title, "Sensor Monitor" );
	assert( status == ERR_NONE );
	status = region_printf( &sensor_reg,
				" 123456789ABCDEFG Rec"
				"A\n"
				"B\n"
				"C\n"
				"D\n"
				"E\n" );

	/* Reset the region for sensor display */
	sensor_reg.col = 5;
	sensor_reg.row = 6;
	sensor_reg.width = 21 - 5;
	sensor_reg.height = 11 - 6;
	sensor_reg.margin = 0;
	sensor_reg.boundary = 0;

	while ( !quit ) {
		status = Receive( &tid, ( char* )&sensor_buffer, sizeof( Sensor_data ) );
		assert( status == sizeof( Sensor_data ) );
		status = Reply( tid, ( char* )&head, sizeof( head ) );
		assert( status == SYSCALL_SUCCESS );

		for( i = 0; i < SENSOR_BYTE_COUNT; i += 1 ){
			head = parse_sensor( sensor_buffer.sensor_raw[ i ], i, sensor_list, head );
		}

		/* Update real time sensor display */
		region_printf( &sensor_reg,
			       "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
			       "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
			       "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
			       "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
			       "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
			       SENSOR_PRINTS( sensor_buffer ) );
		
		/* Update recent triggered sensor list */
		temp = sensor_list_reg.row;
		for( i = 0; i < SENSOR_LIST_SIZE; i += 1 ){
			status = region_printf( &sensor_list_reg, "%s\n", sensor_list[ ( head + i ) % SENSOR_LIST_SIZE ] );
			sensor_list_reg.row += 1;
		}
		sensor_list_reg.row = temp;

		assert( status == 0 );
	}

	Exit();
}

int sensor_ui_update( int tid, Sensor_data* data )
{
	int status;
	int result;

	status = Send( tid, ( char* )data, sizeof( Sensor_data ), ( char* )&result, sizeof( result ) );
	assert( status == sizeof( result ) );
	
	return ERR_NONE;
}
