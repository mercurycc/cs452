#include <user/syscall.h>
#include <user/apps_entry.h>
#include <user/servers_entry.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <user/drivers_entry.h>
#include <user/devices/clock.h>
#include <user/time.h>
#include <user/lib/sync.h>
#include <bwio.h>
#include <err.h>

void user_init()
{
	int tid;	
	tid = Create( 0, name_server_start );
	assert( tid > 0 );
	DEBUG_NOTICE( DBG_USER, "name server created\n" );

	tid = Create( 0, time_main );
	assert( tid > 0 );
	DEBUG_NOTICE( DBG_USER, "name server created\n" );


	tid = Create( 1, lazy_dog );
	assert( tid > 0 );
	DEBUG_NOTICE( DBG_USER, "test case created\n" );
	sync_wait();
	DEBUG_NOTICE( DBG_USER, "test case complete\n" );	

	
	DEBUG_NOTICE( DBG_USER, "Finishing...\n" );
	time_suicide( WhoIs("time") );
	DEBUG_NOTICE( DBG_USER, "clock server killed\n" );
	
	name_server_stop();
	DEBUG_NOTICE( DBG_USER, "name server killed\n" );

	Exit();
}
