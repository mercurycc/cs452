#include <user/syscall.h>
#include <user/apps_entry.h>
#include <bwio.h>

void init_user()
{
	// create a task with lower priority
	int tid = 0;
	tid = Create( 30, noise );
	bwprintf( COM2, "Created: 0x%x\n",tid );

	// send message
	char* msg = "HELLO!";
	char reply[256];
	Send( tid, msg, 6, reply, 256 );
	bwprintf( COM2, "task 0x%x: message sent to: 0x%x\n", MyTid(), tid );

	Exit();
}
