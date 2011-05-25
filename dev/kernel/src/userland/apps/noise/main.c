#include <user/syscall.h>
#include <user/apps_entry.h>
#include <bwio.h>

void noise()
{

	bwprintf( COM2, "New task entered, tid = 0x%x, parent id = 0x%x\n", MyTid(), MyParentTid() );

	char msg[256];
	char* reply = "WELCOME!";
	int tid = 55555;

	// receive
	Receive( &tid, msg, 256 );

	bwprintf( COM2, "task 0x%x: message received from 0x%x: %s\n", MyTid(), tid, msg );

	Reply( tid, reply, 9 );

	bwprintf( COM2, "task 0x%x:exiting...\n", MyTid());
	Exit();

}
