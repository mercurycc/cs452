#include <user/syscall.h>
#include <user/apps_entry.h>
#include <user/servers_entry.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <user/drivers_entry.h>
#include <bwio.h>
#include <err.h>

static inline void K2()
{
	int tid;
	unsigned int buf;
	int status;

	/* For Kernel 2, launch the RPS game */
	tid = Create( 2, rps_game );
	assert( tid >= 0 );

	/* Synchronize with rps_game */
	status = Receive( &tid, ( char* )&buf, sizeof( buf ) );
	assert( status == sizeof( buf ) );
	status = Reply( tid, ( char* )&buf, sizeof( buf ) );
	assert( status == SYSCALL_SUCCESS );
}

void user_init()
{
	int tid;
	int status;
	
	tid = Create( 0, name_server_start );
	assert( tid > 0 );

	/* Create device drivers */
	tid = Create( 0, clock_main );
	assert( tid > 0 );

	K2();

	DEBUG_NOTICE( DBG_USER, "killing name server\n" );
	name_server_stop();
	DEBUG_NOTICE( DBG_USER, "name server killed\n" );

	Exit();
}
