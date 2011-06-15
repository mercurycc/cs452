#include <types.h>
#include <err.h>
#include <config.h>
#include <trap.h>
#include <lib/str.h>
#include <task.h>
#include <syscall.h>
#include <trap_reason.h>
#include <kernel.h>
#include <ts7200.h>
#include <ep9302.h>
#include <interrupt.h>
#include <mem.h>
#include <sched.h>
#include <regopts.h>
#include <devices/clock.h>
#include <watchdog.h>

static inline int msg_copy( Task* sender, Task* receiver ){
	// TODO: implement with assemble?
	char* data = sender->reason->data;
	uint datalen = sender->reason->datalen;
	char* buffer = receiver->reason->buffer;
	uint bufferlen = receiver->reason->bufferlen;

	uint size = 0;
	if ( datalen > bufferlen ){
		size = bufferlen;
	}
	else {
		size = datalen;
	}

	memcpy( (uchar*)buffer, (uchar*)data, size );
	return size;
}

int trap_init( Context* ctx )
{
	uint* trap_addr = (uint*)0x28;
	
	/* All interrupt are handled by trap_handler_begin.  For K1, only swi is setup */
	*trap_addr = (uint)trap_handler_begin;

	return 0;
}

static void syscall_handler( Context* ctx, Syscall* reason )
{
	Task* temp;
	Task* sender_task;
	Task* receiver_task;
	List* elem;
	int status = 0;
	int interrupt = 1;

	reason->result = SYSCALL_SUCCESS;

	switch( reason->code ){
		/* Task management */
	case TRAP_CREATE_DRV:
		interrupt = 0;
	case TRAP_CREATE:
		task_setup( ctx, &temp, reason->data, ctx->current_task, reason->datalen, interrupt );
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
		break;
	case TRAP_MY_PARENT_TID:
		status = task_parent_tid( ctx->current_task, &reason->result );
		if( status == ERR_PARENT_EXIT ){
			reason->result = MY_PARENT_TID_BURIED;
		}
		break;
	case TRAP_PASS:
		status = sched_pass( ctx, ctx->current_task );
		ASSERT( status == ERR_NONE );

		break;
	case TRAP_KILL:
		receiver_task = task_get_by_tid( ctx, reason->target_tid );
		if( receiver_task ){
			if( receiver_task->state != TASK_READY ){
				sched_signal( ctx, receiver_task );
			}
			task_kill( receiver_task->stack );
		}
		break;
	case TRAP_EXIT:
		status = sched_kill( ctx, ctx->current_task );
		ASSERT( status == ERR_NONE );

		/* Release resources */
		status = task_zombiefy( ctx, ctx->current_task );
		ASSERT( status == ERR_NONE );

		break;
		/* Message passing */
	case TRAP_SEND:
		ASSERT( reason->target_tid != ctx->current_task->tid );
		receiver_task = task_get_by_tid( ctx, reason->target_tid );
		sender_task = ctx->current_task;
		if ( ! receiver_task ) {
			if ( reason->target_tid > KERNEL_MAX_NUM_TASKS ) {
				reason->result = SEND_INVALID_TASK_ID;
			} else {
				reason->result = SEND_TASK_DOES_NOT_EXIST;
			}
			break;
		} else if ( receiver_task->state == TASK_SEND_BLK ) {
			// pass sender tid to receiver
			*(uint*)(receiver_task->reason->data) = sender_task->tid;
			// reply block sender
			status = sched_block( ctx );
			ASSERT( status == ERR_NONE );
			sender_task->state = TASK_RPL_BLK;
			// signal receiver
			status = sched_signal( ctx, receiver_task );
			ASSERT( status == ERR_NONE );
			// copy message
			status = msg_copy( sender_task, receiver_task );
			receiver_task->reason->result = status;
		} else {
			// receive block sender
			sched_block( ctx );
			ASSERT( status == ERR_NONE );
			sender_task->state = TASK_RCV_BLK;
			// add to send queue
			status = list_add_tail( &(receiver_task->send_queue), &(sender_task->queue) );
			ASSERT( status == ERR_NONE );
		}
		break;
	case TRAP_RECEIVE:
		receiver_task = ctx->current_task;
		if( receiver_task->send_queue ) {
			// get sender;
			status = list_remove_head( &(receiver_task->send_queue), &elem );
			ASSERT( status == ERR_NONE );
			sender_task = list_entry( Task, elem, queue );
			// change sender to reply block
			sender_task->state = TASK_RPL_BLK;
			// pass sender tid to receiver
			*(uint*)(receiver_task->reason->data) = sender_task->tid;
			// copy message
			status = msg_copy( sender_task, receiver_task );
			receiver_task->reason->result = status;
		}
		else {
			// send block receiver
			status = sched_block( ctx );
			ASSERT( status == ERR_NONE );
			receiver_task->state = TASK_SEND_BLK;
		}
		break;
	case TRAP_REPLY:
		sender_task = task_get_by_tid( ctx, reason->target_tid );
		receiver_task = ctx->current_task;

		DEBUG_PRINT( DBG_TRAP, "Target_tid = %d\n", reason->target_tid );

		if( ! sender_task ){
			if ( reason->target_tid >= KERNEL_MAX_NUM_TASKS ) {
				reason->result = REPLY_INVALID_TASK_ID;
			} else {
				reason->result = REPLY_TASK_DOES_NOT_EXIST;
			}
			break;
		}

		if ( sender_task->state != TASK_RPL_BLK ) {
			receiver_task->reason->result = REPLY_TASK_IN_WRONG_STATE;
			break;
		}
		
		// copy message
		status = msg_copy( receiver_task, sender_task );
		receiver_task->reason->result = ERR_NONE;
		sender_task->reason->result = status;

		// signal sender
		status = sched_signal( ctx, sender_task );
		ASSERT( status == ERR_NONE );
		
		break;
	case TRAP_EXIST:
		receiver_task = task_get_by_tid( ctx, reason->target_tid );
		reason->result = receiver_task != 0;
		break;
	case TRAP_KERNEL_CONTEXT:
		*((Context**)(reason->buffer)) = ctx;
		break;
	case TRAP_AWAIT_EVENT:
		status = interrupt_register( ctx, ctx->current_task, reason->target_tid );
		if( status == ERR_INTERRUPT_ALREADY_REGISTERED ){
			reason->result = AWAIT_EVENT_ALREADY_REGISTERED;
		} else if( status == ERR_INTERRUPT_INVALID_INTERRUPT ){
			reason->result = AWAIT_EVENT_INVALID_EVENT;
		} else {
			ASSERT( status == ERR_NONE );
			sched_block( ctx );
			reason->result = SYSCALL_SUCCESS;
		}
		break;
	case TRAP_PRESHUTDOWN:
		status = interrupt_deinit( ctx );
		ASSERT( status == ERR_NONE );
		break;
	default:
		DEBUG_PRINT( DBG_TMP, "%u not implemented\n", reason->code );
	}
}

void trap_handler( Syscall* reason, uint sp_caller, uint mode, ptr kernelsp )
{
	Context* ctx = (Context*)(*(uint*)kernelsp);
	int status = 0;
	DEBUG_PRINT( DBG_TRAP, "Obtained context 0x%x\n", ctx );

	// DEBUG_PRINT( DBG_TEMP, "Trap handler called by tid: %d, sp = 0x%x, mode = 0x%x\n", ctx->current_task->tid, sp_caller, mode );

	ctx->current_task->stack = sp_caller;
	ctx->current_task->reason = reason;

#ifdef KERNEL_ENABLE_WATCHDOG
	/* Feed watchdog */
	watchdog_refresh();
#endif

	// TODO: change err codes

	if( ( mode & CPSR_MODE_MASK ) == CPSR_MODE_IRQ ){
		DEBUG_NOTICE( DBG_TRAP, "coming from IRQ\n" );
		{
			Task* event_handler;
			
			status = interrupt_handle( ctx, &event_handler );
			ASSERT( status == ERR_NONE );

			// Add served event handler back to ready queue
			status = sched_signal( ctx, event_handler );
			ASSERT( status == ERR_NONE );
		}
	} else if ( ( mode & CPSR_MODE_MASK ) == CPSR_MODE_USER ){
		syscall_handler( ctx, reason );
	}

	DEBUG_NOTICE( DBG_TRAP, "sched scheduling...\n" );
	status = sched_schedule( ctx, &(ctx->current_task) );
	ASSERT( status == ERR_NONE );
	DEBUG_PRINT( DBG_TRAP, "new task addr: 0x%x, tid: 0x%x, priority: %d, state: %d\n",
		     ctx->current_task, ctx->current_task->tid, ctx->current_task->priority, ctx->current_task->state );

	/* Shutdown kernel if no ready task can be scheduled */
	if (!(ctx->current_task)){
		DEBUG_NOTICE( DBG_TRAP, "shutting down...\n" );
		kernel_shutdown( kernelsp );
	}

	trap_handler_end( reason, ctx->current_task->stack, mode, kernelsp );
}
