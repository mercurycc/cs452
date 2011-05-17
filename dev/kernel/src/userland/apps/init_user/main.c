#include <user/syscall.h>
#include <user/apps_entry.h>
#include <bwio.h>

void init_user()
{
	bwprintf( COM2, "%s entered, tid 0x%x\n", __func__, MyTid() );

	Create( 0, noise );

	while( 1 ){
		Pass();
		bwprintf( COM2, "%s reentered, tid 0x%x, parent tid 0x%x\n", __func__, MyTid(), MyParentTid() );
	}
}
