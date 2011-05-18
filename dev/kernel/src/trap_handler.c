#include <types.h>
#include <err.h>
#include <config.h>
#include <trap.h>
#include <lib/str.h>
#include <task.h>
#include <syscall.h>
#include <trap_reason.h>
#include <kernel.h>
#include <mem.h>
#include <sched.h>

int trap_init( Context* ctx )
{
	uint* trap_vector_base = (uint*)0x28;
	
	/* All interrupt are handled by trap_handler_begin.  For K1, only swi is setup */
	*trap_vector_base = (uint)trap_handler_begin;

	return 0;
}

void trap_handler( Syscall* reason, uint sp_caller, uint mode, ptr kernelsp )
{
	Context* ctx = (Context*)(*(uint*)kernelsp);
	int status = 0;
	Task* temp;
	DEBUG_PRINT( DBG_TRAP, "Obtained context 0x%x\n", ctx );
	DEBUG_PRINT( DBG_TRAP, "Trap handler called with reason 0x%x, sp = 0x%x\n", reason->code, sp_caller );

	ctx->current_task->stack = sp_caller;
	ctx->current_task->reason = reason;

	// TODO: change err codes

	switch( reason->code ){
	case TRAP_CREATE:
		task_setup( ctx, &temp, reason->data, ctx->current_task, reason->datalen );
		if ( status ) {
			reason->result = status;
			break;
		} else {
			reason->result = task_tid( temp );
		}
		DEBUG_PRINT( DBG_TRAP, "new task created at addr 0x%x, list ptr 0x%x\n", temp, &(temp->queue));
		break;
	case TRAP_MY_TID:
		reason->result = task_tid( ctx->current_task );
		break;
	case TRAP_MY_PARENT_TID:
		reason->result = task_parent_tid( ctx->current_task );
		break;
	case TRAP_PASS:
		DEBUG_NOTICE( DBG_TRAP, "sched passing...\n" );
		DEBUG_PRINT( DBG_TRAP, "current task addr = %x\n", ctx->current_task );

		status = sched_pass( ctx, ctx->current_task );
		DEBUG_PRINT( DBG_TRAP, "status = %d\n", status );

		ASSERT( status == ERR_NONE );

		break;
	case TRAP_EXIT:

		DEBUG_NOTICE( DBG_TRAP, "sched killing...\n" );

		status = sched_kill( ctx, ctx->current_task );
		ASSERT( status == ERR_NONE );

		/* Release resources */
		status = task_zombiefy( ctx, ctx->current_task );
		ASSERT( status == ERR_NONE );

		break;
	default:
		DEBUG_PRINT( DBG_TRAP, "%u not implemented\n", reason->code );
	}

	DEBUG_NOTICE( DBG_TRAP, "sched scheduling...\n" );
	status = sched_schedule( ctx, &(ctx->current_task) );
	ASSERT( status == ERR_NONE );
	DEBUG_PRINT( DBG_TRAP, "new task addr = %x\n", ctx->current_task );

	/* Shutdown kernel if no ready task can be scheduled */
	if (!(ctx->current_task)){
		DEBUG_NOTICE( DBG_TRAP, "shutting down...\n" );
		kernel_shutdown( kernelsp );
	}

	trap_handler_end( reason, ctx->current_task->stack, mode, kernelsp );
}
