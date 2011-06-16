#include <types.h>
#include <err.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/clock_server.h>
#include <user/devices/clock.h>
#include <user/train.h>

void train_sensor() {
	int quit = 0;
	int status;

	while ( !quit ) {
		status = train_all_sensor();
		if ( status == -1 ) {
			quit = 1;
		}
		else {
			assert( status == 0 );
		}

		status = Delay(10);
		assert( status == 0 );
	}
	
	Exit();
}



