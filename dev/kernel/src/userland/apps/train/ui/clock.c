#include <types.h>
#include <err.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/clock_server.h>
#include <user/devices/clock.h>
#include <user/uart.h>
#include <user/display.h>
#include "../inc/train.h"

void clock_ui()
{
	int quit = 0;
	int status;
	uint ticks = Time();
	uint day;
	uint hour;
	uint minu;
	uint sec;
	uint msec;
	uint tmsec;

	Region clock_rect = { 63, 1, 1, 78 - 63, 1, 0 };
	Region *clock_region = &clock_rect;
	status = region_init( clock_region );
	assert( status == ERR_NONE );

	while ( !quit ) {
		status = Delay( 10 );
		ticks = Time();
		assert( status == 0 );

		tmsec = ticks / 10;
		msec = tmsec % 10;
		tmsec /= 10;
		sec = tmsec % 60;
		tmsec /= 60;
		minu = tmsec % 60;
		tmsec /= 60;
		hour = tmsec % 24;
		tmsec /= 24;
		day = tmsec;

		status = region_printf( clock_region, "%02u:%02u:%02u:%02u:%u", day, hour, minu, sec, msec );
		assert( status == 0 );

	}

	Exit();
}



