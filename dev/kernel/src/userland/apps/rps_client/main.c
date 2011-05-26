#include <user/syscall.h>
#include <user/apps_entry.h>
#include <user/name_server.h>
#include <user/RPS_game.h>
#include <user/assert.h>
#include <user/lib/prng.h>
#include <bwio.h>

#define CLIENT_PRINT( fmt, args... )					\
	do {								\
		if( group_id < 0 ){					\
			bwprintf( COM2, "Ungrouped RPSClient %d: " fmt, tid, args ); \
		} else {						\
			bwprintf( COM2, "Group %d RPSClient %d: " fmt, group_id, tid, args ); \
		}							\
	} while( 0 )

void rps_client()
{
	unsigned int server_tid = 0;
	unsigned int tid = MyTid();
	unsigned int seed = tid;
	int group_id = -1;
	RPSmsg msg;
	RPSreply reply;
	unsigned int losings = 0;
	int status = 0;

	/* Query server */
	server_tid = WhoIs( "RPSServer" );
	CLIENT_PRINT( "RSPServer tid found: 0x%d\n", server_tid );
	
	/* Signup */
	msg.command = SIGN_UP;
	status = Send( server_tid, ( char* )&msg, sizeof( msg ), ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );

	group_id = reply.result;

	CLIENT_PRINT( "signup'ed, group assigned: %d\n", group_id );

	/* Play the game, after 2 consecutive loses, each lose will make the player tose a coin to decide if he will quit the game */
	while( 1 ){
		msg.group_num = group_id;
		if( losings >= 2 && ( random( &seed ) % 2 ) ){
			CLIENT_PRINT( "lost too much, will quit now\n", 0 );
			msg.command = QUIT;
		} else {
			msg.command = ( random( &seed ) % 3 ) + ROCK;
		}

		{
			char* decision;
			switch( msg.command ){
			case QUIT:
				decision = "QUIT";
				break;
			case ROCK:
				decision = "ROCK";
				break;
			case PAPER:
				decision = "PAPER";
				break;
			case SCISSORS:
				decision = "SCISSORS";
				break;
			default:
				assert( 0 );
			}
			CLIENT_PRINT( "requesting server for %s\n", decision );
		}
		
		status = Send( server_tid, ( char* )&msg, sizeof( msg ), ( char* )&reply, sizeof( reply ) );
		assert( status == sizeof( reply ) );

		if( reply.result == RESULT_QUIT ){
			CLIENT_PRINT( "server says I should quit\n", 0 );
			break;
		} else if( reply.result == RESULT_WIN ){
			CLIENT_PRINT( "server says I win\n", 0 );
			losings = 0;
		} else if( reply.result == RESULT_LOSE ){
			CLIENT_PRINT( "server says I lose\n", 0 );
			losings += 1;
		} else if( reply.result == RESULT_DRAW ){
			CLIENT_PRINT( "server says we draw\n", 0 );
		} else {
			assert( 0 );
		}

		/* Pause */
		bwgetc( COM2 );
	}

	/* Signal parent */
	{
		unsigned int buf;
		status = Send( MyParentTid(), ( char* )&buf, sizeof( buf ), ( char* )&buf, sizeof( buf ) );
		assert( status == sizeof( buf ) );
	}

	Exit();
}
