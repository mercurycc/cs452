#include <types.h>
#include <err.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/clock_server.h>
#include <user/devices/clock.h>
#include <user/train.h>
#include <user/uart.h>
#include <user/display.h>


void train_clock() {
	int quit = 0;
	int status;
	uint ticks = Time();

	char* time_str = "00:00:00.0";

	Region clock_rect = { 59, 1, 3, 16, 0, 1 };
	Region *clock_region = &clock_rect;
	status = region_init( clock_region );
	assert( status == ERR_NONE );

	while ( !quit ) {
		ticks += 100 / CLOCK_COUNT_DOWN_MS_PER_TICK;
		status = DelayUntil(ticks);
		assert( status == 0 );

		time_str[9]++;
		if ( time_str[9] > '9' ) {
			time_str[9] -= 10;
			time_str[7] += 1;
		}
		if ( time_str[7] > '9' ) {
			time_str[7] -= 10;
			time_str[6] += 1;
		}
		if ( time_str[6] > '5' ) {
			time_str[6] -= 6;
			time_str[4] += 1;
		}
		if ( time_str[4] > '9' ) {
			time_str[3] -= 10;
			time_str[3] += 1;
		}
		if ( time_str[3] > '5' ) {
			time_str[3] -= 6;
			time_str[1] += 1;
		}
		if ( time_str[1] > '9' ) {
			time_str[0] -= 10;
			time_str[0] += 1;
		}

		status = region_printf( clock_region, "  %s", time_str );
		assert( status == 0 );

	}

	Exit();
}



