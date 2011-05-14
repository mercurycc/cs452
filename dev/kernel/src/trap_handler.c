#include <types.h>
#include <err.h>
#include <config.h>
#include <trap.h>
#include <lib/str.h>

int trap_init( Context* ctx )
{
	void (**trap_vector_base)( uint, uint ) = (void (**)( uint, uint))0x28;
	
	/* All interrupt are handled by trap_handler_begin.  For K1, only swi is setup */
	*trap_vector_base = trap_handler_begin;

	return 0;
}

void trap_handler( uint reason, uint sp_caller, uint mode )
{
	DEBUG_PRINT( DBG_TRAP, "Trap handler called with reason 0x%x, sp = 0x%x\n", reason, sp_caller );

	/* Exit trap handler */
	trap_handler_end( reason, sp_caller );
}

