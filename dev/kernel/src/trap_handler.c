#include <types.h>
#include <err.h>
#include <config.h>
#include <trap.h>
#include <lib/str.h>
#include <task.h>

int trap_init( Context* ctx )
{
	uint* trap_vector_base = (uint*)0x28;
	
	/* All interrupt are handled by trap_handler_begin.  For K1, only swi is setup */
	*trap_vector_base = (uint)trap_handler_begin;

	return 0;
}

void trap_handler( uint reason, uint sp_caller, uint mode, uint* kernelsp )
{
	Context* ctx = (Context*)(*kernelsp);
	Task* temp;
	DEBUG_PRINT( DBG_TRAP, "Obtained context 0x%x\n", ctx );
	DEBUG_PRINT( DBG_TRAP, "Trap handler called with reason 0x%x, sp = 0x%x, position = 0x%x\n", reason, sp_caller, &ctx );

	ctx->current_task->stack = sp_caller;
	temp = ctx->current_task;
	ctx->current_task = ctx->last_task;
	ctx->last_task = temp;

	trap_handler_end( reason, ctx->current_task->stack, mode, kernelsp );
}
