#include <types.h>
#include <err.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/clock_server.h>
#include <user/devices/clock.h>
#include <user/train.h>
#include <user/uart.h>
#include <user/display.h>

void train_sensor() {
	int quit = 0;
	int status;
	int i;

	char sensor_table[10];

	Region sensor_rect = { 1, 10, 4, 60, 0, 0 };
	Region *sensor_region = &sensor_rect;
	status = region_init( sensor_region );
	assert( status == ERR_NONE );

	while ( !quit ) {

		status = Putc( COM_1, (char)133 );
		assert( status == ERR_NONE );
		for ( i = 0; i < 10; i++ ) {
			sensor_table[i] = Getc( COM_1 );
		}

		status = region_printf( sensor_region, "\n|%d|%d|%d|%d|%d|\n|%d|%d|%d|%d|%d|\n", sensor_table[0], sensor_table[1], sensor_table[2], sensor_table[3], sensor_table[4], sensor_table[5], sensor_table[6], sensor_table[7], sensor_table[8], sensor_table[9] );
		assert( status == 0 );


		status = Delay(1);
		assert( status == 0 );
	}

	Exit();
}



