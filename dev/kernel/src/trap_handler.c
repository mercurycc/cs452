#include <types.h>
#include <err.h>
#include <config.h>
#include <trap.h>
#include <lib/str.h>
#include <task.h>
#include <syscall.h>
#include <trap_reason.h>

int trap_init( Context* ctx )
{
	uint* trap_vector_base = (uint*)0x28;
	
	/* All interrupt are handled by trap_handler_begin.  For K1, only swi is setup */
	*trap_vector_base = (uint)trap_handler_begin;

	return 0;
}

void trap_handler( Syscall* reason, uint sp_caller, uint mode, uint* kernelsp )
{
	Context* ctx = (Context*)(*kernelsp);
	Task* temp;
	DEBUG_PRINT( DBG_TRAP, "Obtained context 0x%x\n", ctx );
	DEBUG_PRINT( DBG_TRAP, "Trap handler called with reason 0x%x, sp = 0x%x, position = 0x%x\n", reason, sp_caller, &ctx );

	ctx->current_task->stack = sp_caller;
	ctx->current_task->reason = reason;

	DEBUG_PRINT( DBG_TRAP, "reason %u called\n", reason->code );

	switch( reason->code ){
	case TRAP_PASS:
		temp = ctx->current_task;
		ctx->current_task = ctx->last_task;
		ctx->last_task = temp;
		break;
	default:
		DEBUG_PRINT( DBG_TRAP, "%u not implemented\n", reason->code );
	}

	trap_handler_end( reason, ctx->current_task->stack, mode, kernelsp );
}
