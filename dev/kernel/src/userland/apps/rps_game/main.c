#include <user/syscall.h>
#include <user/apps_entry.h>
#include <user/servers_entry.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <user/RPS_game.h>
#include <bwio.h>

void rps_game()
{
	unsigned int i = 0;
	unsigned int status = 0;
	unsigned int server_tid;
	int tid;
	unsigned int buf;
	RPSmsg msg;
	RPSreply reply;

	/* Launch rps_client */
	for( i = 0; i < ( CLIENT_GROUP_COUNT * 2 ); i += 1 ){
		status = Create( 4 + i - i % 2, rps_client );
		assert( status >= 0 );
	}

	/* Launch rps_server */
	server_tid = Create( 3, RPSServer );
	assert( server_tid >= 0 );

	/* Synchronize */
	for( i = 0; ( i < CLIENT_GROUP_COUNT * 2 ); i += 1 ){
		status = Receive( &tid, ( char* )&buf, sizeof( buf ) );
		assert( status == sizeof( buf ) );
		status = Reply( tid, ( char* )&buf, sizeof( buf ) );
		assert( status == SYSCALL_SUCCESS );
	}

	/* Kill rps_server */
	msg.command = SUICIDE;
	status = Send( server_tid, ( char* )&msg, sizeof( msg ), ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );

	/* Signal parent */
	status = Send( MyParentTid(), ( char* )&buf, sizeof( buf ), ( char* )&buf, sizeof( buf ) );
	assert( status == sizeof( buf ) );

	Exit();
}
