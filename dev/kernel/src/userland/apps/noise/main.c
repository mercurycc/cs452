#include <user/syscall.h>
#include <user/apps_entry.h>
#include <user/name_server.h>
#include <bwio.h>
#include <user/apps_entry.h>
#include <user/servers_entry.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <user/drivers_entry.h>
#include <user/devices/clock.h>
#include <user/time.h>
#include <bwio.h>
#include <err.h>

void noise()
{
	int status;

	int tid = WhoIs("time");

	status = time_delay( tid, 500 );
	assert( status == 0 );

	status = time_ask( tid );
	bwprintf( COM2, "current time is 0x%x\n", status );


	Exit();
}
