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

	/* Fixed launch order in order to obtain fixed tid for servers */
	/*
	  1      name server
	  2      time server
	  3 - 7  console servers  (servers for each UART are created by the main console server)
	*/
	tid = Create( 0, name_server_start );
	assert( tid > 0 );
	DEBUG_NOTICE( DBG_USER, "name server created\n" );

	tid = Create( 0, time_main );
	assert( tid > 0 );
	DEBUG_NOTICE( DBG_USER, "time server created\n" );

	/* tid = Create( 0, console_main ); */
	/* assert( tid > 0 ); */
	/* DEBUG_NOTICE( DBG_USER, "console server created\n" ); */


	tid = Create( 1, lazy_dog );
	assert( tid > 0 );
	DEBUG_NOTICE( DBG_USER, "test case created\n" );

	
	sync_wait();
	
	DEBUG_NOTICE( DBG_USER, "Finishing...\n" );
	time_suicide( WhoIs("time") );
	DEBUG_NOTICE( DBG_USER, "clock server killed\n" );
	
	name_server_stop();
	DEBUG_NOTICE( DBG_USER, "name server killed\n" );

	Exit();
}
