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
	Task* sender_task;
	Task* receiver_task;
	List* elem;
	DEBUG_PRINT( DBG_TRAP, "Obtained context 0x%x\n", ctx );
	DEBUG_PRINT( DBG_TRAP, "Trap handler called with reason 0x%x, sp = 0x%x\n", reason->code, sp_caller );

	ctx->current_task->stack = sp_caller;
	ctx->current_task->reason = reason;

	// TODO: change err codes

	switch( reason->code ){
		/* Task management */
	case TRAP_CREATE:
		task_setup( ctx, &temp, reason->data, ctx->current_task, reason->datalen );
		if ( status == ERR_INVALID_PRIORITY ) {
			reason->result = CREATE_INVALID_PRIORITY;
		} else if ( status == ERR_OUT_OF_TASK_DESCRIPTOR ){
			reason->result = CREATE_OUT_OF_TASK_DESCRIPTOR;
		} else {
			reason->result = task_tid( temp );
			DEBUG_PRINT( DBG_TRAP, "new task created at addr 0x%x, list ptr 0x%x\n", temp, &(temp->queue));
		}
		break;
	case TRAP_MY_TID:
		reason->result = task_tid( ctx->current_task );
		if( reason->result == ERR_PARENT_EXIT ){
			reason->result = MY_PARENT_TID_BURIED;
		}
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
		/* Message passing */
	case TRAP_SEND:
		receiver_task = &( ctx->task_array[ task_array_index_tid( reason->target_tid ) ] );
		sender_task = ctx->current_task;
		if ( receiver_task->state == TASK_SEND_BLK ) {
			//copy message
			DEBUG_NOTICE( DBG_TRAP, "test only, message not copied" );
			//reply block sender
			status = sched_block( ctx );
			ASSERT( status == ERR_NONE );
			sender_task->state = TASK_RPL_BLK;
			//signal receiver
			status = sched_signal( ctx, sender_task );
			ASSERT( status == ERR_NONE );
		}
		else {
			//receive block sender
			sched_block( ctx );
			ASSERT( status == ERR_NONE );
			sender_task->state = TASK_RCV_BLK;
			//add to send queue
			status = list_add_tail( &(receiver_task->send_queue), &(sender_task->queue) );
			ASSERT( status == ERR_NONE );
		}

		DEBUG_PRINT( DBG_TMP, "%u not implemented\n", reason->code );
		break;
	case TRAP_RECEIVE:
		receiver_task = ctx->current_task;
		if ( receiver_task->send_queue ) {
			//get sender;
			status = list_remove_head( &(receiver_task->send_queue), &elem );
			ASSERT( status == ERR_NONE );
			sender_task = list_entry( Task, elem, queue );
			//copy message
			DEBUG_NOTICE( DBG_TRAP, "test only, message not copied" );
			//change sender to reply block
			sender_task->state = TASK_RPL_BLK;
		}
		else {
			//send block receiver
			status = sched_block( ctx );
			ASSERT( status == ERR_NONE );
			receiver_task->state = TASK_SEND_BLK;
		}

		DEBUG_PRINT( DBG_TMP, "%u not implemented\n", reason->code );
		break;
	case TRAP_REPLY:
		receiver_task = ctx->current_task;
		sender_task = &( ctx->task_array[ task_array_index_tid( reason->target_tid ) ] );
		//copy message
		DEBUG_NOTICE( DBG_TRAP, "test only, message not copied" );
		//signal sender
		status = sched_signal( ctx, sender_task );
		ASSERT( status == ERR_NONE );

		DEBUG_PRINT( DBG_TMP, "%u not implemented\n", reason->code );
		break;
	case TRAP_REGISTER_AS:
		DEBUG_PRINT( DBG_TMP, "%u not implemented\n", reason->code );
		break;
	case TRAP_WHO_IS:
		DEBUG_PRINT( DBG_TMP, "%u not implemented\n", reason->code );
		break;
	default:
		DEBUG_PRINT( DBG_TMP, "%u not implemented\n", reason->code );
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
