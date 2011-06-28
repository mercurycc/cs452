#include <types.h>
#include <err.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/clock_server.h>
#include <user/time.h>
#include <user/devices/clock.h>
#include <user/uart.h>
#include <user/display.h>
#include "../inc/train.h"

void clock_ui()
{
	int quit = 0;
	int status;
	uint ticks = Time();
	Timespec spec;

	Region clock_rect = { 63, 1, 1, 78 - 63, 1, 0 };
	Region *clock_region = &clock_rect;
	status = region_init( clock_region );
	assert( status == ERR_NONE );

	while ( !quit ) {
		status = Delay( 10 );
		ticks = Time();
		assert( status == 0 );

		time_tick_to_spec( &spec, ticks );

		status = region_printf( clock_region, "%02u:%02u:%02u:%02u:%u", spec.day, spec.hour, spec.minute, spec.second, spec.fraction );
		assert( status == 0 );

	}

	Exit();
}



