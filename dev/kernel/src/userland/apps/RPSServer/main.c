#include <user/syscall.h>
#include <user/apps_entry.h>
#include <user/name_server.h>
#include <user/RPS_game.h>
#include <user/assert.h>
#include <err.h>
#include <bwio.h>

struct Group {
	int occupied;
	int p;
	int c;
};

void RPSServer() {
	int quit = 0;
	struct Group group[32] = {{0}};
	int index = 0;
	int signup_waiter = -1;
	int winner = 0;
	int status;
	int i = 0;
	
	bwprintf( COM2, "RPS Server start.\n" );

	for ( i = 0; i < 32; i++ ){
		group[i].occupied = 0;
	}

	/* Register */
	status = RegisterAs( "RPSServer" );
	assert( status == 0 );

	while ( !quit ) {
		int tid;
		struct RPSmsg msg;
		struct RPSreply reply;
		int status = 0;

		status = Receive( &tid, (char*)&msg, sizeof( msg ) );
		assert( status == sizeof( msg ) );

		// parse command
		switch ( msg.command ) {
		case SUICIDE:
			if ( tid != MyParentTid() ){
				bwprintf( COM2, "Receive fake suicide command from task 0x%x\n", tid );
			} else {
				quit = 1;
				/* At this point all clients should have already quited */
				reply.result = RESULT_QUIT;
				status = Reply( tid, ( char* )&reply, sizeof( reply ) );
				assert( status == SYSCALL_SUCCESS );
			}
			break;
		case SIGN_UP:
			if ( signup_waiter == -1 ) {
				signup_waiter = tid;
			}
			else {
				index = 0;
				while ( ( index < 32 )&& group[index].occupied ){
					index++;
				}
				if ( index >= 32 ) {
					bwprintf( COM2, "server full.\n" );
				}

				group[index].p = 0;
				group[index].occupied = 1;
				group[index].c = 0;

				reply.result = index;
				
				status = Reply( signup_waiter, (char*)&reply, sizeof( reply ) );
				DEBUG_PRINT( DBG_USER, "return status = %d\n", status );
				assert( status == SYSCALL_SUCCESS );
				status = Reply( tid, (char*)&reply, sizeof( reply ) );
				assert( status == SYSCALL_SUCCESS );

				/* Reset signup_waiter */
				signup_waiter = -1;
			}
			break;
		case QUIT:
		case ROCK:
		case PAPER:
		case SCISSORS:
			index = msg.group_num;
			if ( group[index].c ) {
				winner = ( ( group[index].c + 1 ) - ( msg.command - 2 ) ) % 3;
				if ( ( group[index].c == QUIT ) || ( msg.command == QUIT ) ) {
					// both quit
					reply.result = RESULT_QUIT;
					status = Reply( group[index].p, (char*)&reply, sizeof( reply ) );
					assert( status == SYSCALL_SUCCESS );
					status = Reply( tid, (char*)&reply, sizeof( reply ) );
					assert( status == SYSCALL_SUCCESS );

				} else if ( winner == 1 ) {
					reply.result = RESULT_WIN;
					status = Reply( group[index].p, (char*)&reply, sizeof( reply ) );
					assert( status == SYSCALL_SUCCESS );
					reply.result = RESULT_LOSE;
					status = Reply( tid, (char*)&reply, sizeof( reply ) );
					assert( status == SYSCALL_SUCCESS );
				} else if ( winner == 2 ) {
					reply.result = RESULT_LOSE;
					status = Reply( group[index].p, (char*)&reply, sizeof( reply ) );
					assert( status == SYSCALL_SUCCESS );
					reply.result = RESULT_WIN;
					status = Reply( tid, (char*)&reply, sizeof( reply ) );
					assert( status == SYSCALL_SUCCESS );
				} else {
					// draw
					reply.result = RESULT_DRAW;
					status = Reply( group[index].p, (char*)&reply, sizeof( reply ) );
					assert( status == SYSCALL_SUCCESS );
					status = Reply( tid, (char*)&reply, sizeof( reply ) );
					assert( status == SYSCALL_SUCCESS );
				}
				group[index].p = 0;
				group[index].c = 0;
			} else {
				group[index].p = tid;
				group[index].c = msg.command;
			}
			break;
		default:
			bwprintf( COM2, "Invalid command: 0x%x\n", msg.command );
			break;
		}
	}

	bwprintf( COM2, "RPS Server exit.\n" );
	Exit();
}

