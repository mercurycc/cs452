#include <types.h>
#include <err.h>
#include <config.h>
#include <trap.h>
#include <lib/str.h>
#include <task.h>
#include <syscall.h>
#include <trap_reason.h>
#include <mem.h>
#include <sched.h>

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
	int status = 0;
	Task* temp;
	DEBUG_PRINT( DBG_TRAP, "Obtained context 0x%x\n", ctx );
	DEBUG_PRINT( DBG_TRAP, "Trap handler called with reason 0x%x, sp = 0x%x\n", reason->code, sp_caller );

	ctx->current_task->stack = sp_caller;
	ctx->current_task->reason = reason;

	switch( reason->code ){
	case TRAP_CREATE:
		status = mem_alloc( ctx, MEM_TASK, ( void** )&ctx->last_task, 1 );
		if( status == ERR_OUT_OF_MEMORY ){
			reason->result = -1;
		} else {
			task_setup( ctx, ctx->last_task, reason->data, ctx->current_task, reason->datalen );
			reason->result = task_tid( ctx->last_task );
		}
		break;
	case TRAP_MY_TID:
		reason->result = task_tid( ctx->current_task );
		break;
	case TRAP_MY_PARENT_TID:
		reason->result = task_tid( ctx->current_task->parent );
		break;
	case TRAP_PASS:
		temp = ctx->current_task;
		ctx->current_task = ctx->last_task;
		ctx->last_task = temp;

		status = sched_pass( ctx, ctx->current_task );
		if ( status ) {
			reason->result = status;
			break;
		}

		status = sched_schedule( ctx, ctx->current_task, &(ctx->current_task) );
		if ( status ) {
			reason->result = status;
			break;
		}
		// TODO: change err codes
		break;
	case TRAP_EXIT:
		
		break;
	default:
		DEBUG_PRINT( DBG_TRAP, "%u not implemented\n", reason->code );
	}

	trap_handler_end( reason, ctx->current_task->stack, mode, kernelsp );
}
