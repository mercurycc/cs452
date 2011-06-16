#include <types.h>
#include <err.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/clock_server.h>
#include <user/devices/clock.h>
#include <user/train.h>


void train_clock() {
	int quit = 0;
	int status;
	uint ticks = 0;

	while ( !quit ) {
		ticks += 100 / CLOCK_COUNT_DOWN_MS_PER_TICK;
		status = DelayUntil(ticks);
		assert( status == 0 );

		status = train_update_time( ticks );
		if ( status == -1 ) {
			quit = 1;
		}
		else {
			assert( status == 0 );
		}
	}
	
	Exit();
}



