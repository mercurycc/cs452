#include <user/syscall.h>
#include <user/apps_entry.h>
#include <bwio.h>

void noise()
{

	bwprintf( COM2, "New task entered, tid = 0x%x, parent id = 0x%x\n", MyTid(), MyParentTid() );

	Pass();

	bwprintf( COM2, "New task entered, tid = 0x%x, parent id = 0x%x\n", MyTid(), MyParentTid() );

	Exit();

}
