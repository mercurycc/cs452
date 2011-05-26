#include <user/syscall.h>
#include <user/apps_entry.h>
#include <user/servers_entry.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <bwio.h>

void user_init()
{
	unsigned int tid;
	unsigned int query;
	
	tid = Create( 0, name_server_start );
	assert( tid == NAME_SERVER_TID );

	tid = Create( 30, init_user );

	query = WhoIs( "Goodman" );

	assert( tid == query - 1 );

	name_server_stop();
}
