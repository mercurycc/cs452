#include <types.h>
#include <err.h>
#include <config.h>
#include <trap.h>
#include <lib/str.h>

int userland1();
int userland2();

int trap_init( Context* ctx )
{
	uint* trap_vector_base = (uint*)0x28;
	
	/* All interrupt are handled by trap_handler_begin.  For K1, only swi is setup */
	*trap_vector_base = (uint)trap_handler_begin;

	return 0;
}

void trap_handler( uint reason, uint sp_caller, uint mode, uint kernelsp )
{
	static uint lastcaller = 0;
	static uint lastlastcaller = 0;
	uint position;
	DEBUG_PRINT( DBG_TRAP, "Trap handler called with reason 0x%x, sp = 0x%x, position = 0x%x\n", reason, sp_caller, &position );

	lastlastcaller = lastcaller;
	lastcaller = sp_caller;
	
	if( reason == 0xdeadbeef ){
		/* Exit trap handler */
		uint* ptr = sp_caller - 4100;
		*ptr = (uint)userland2;
		trap_handler_end( reason, ptr, mode, kernelsp );
	} else if( reason == 0xcafebabe ){
		trap_handler_end( reason, lastlastcaller, mode, kernelsp );
	}
}
