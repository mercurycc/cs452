#include <user/syscall.h>
#include <user/apps_entry.h>
#include <bwio.h>

void init_user()
{
	// create 4 tasks
	int tid = 0;
	tid = Create( 30, noise );
	bwprintf( COM2, "Created: 0x%x\n",tid );

	tid = Create( 30, noise );
	bwprintf( COM2, "Created: 0x%x\n",tid );

	tid = Create( 0, noise );
	bwprintf( COM2, "Created: 0x%x\n",tid );

	tid = Create( 0, noise );
	bwprintf( COM2, "Created: 0x%x\n",tid );

	bwprintf( COM2, "First: exiting\n" );
	Exit();

}
