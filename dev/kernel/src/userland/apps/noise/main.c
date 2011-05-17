#include <user/syscall.h>
#include <user/apps_entry.h>
#include <bwio.h>

void noise()
{
	bwprintf( COM2, "%s entered, tid 0x%x\n", __func__, MyTid() );

	while( 1 ){
		Pass();
		bwprintf( COM2, "%s reentered, tid 0x%x, parent tid 0x%x\n", __func__, MyTid(), MyParentTid() );
	}
}
