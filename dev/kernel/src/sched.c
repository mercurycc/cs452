#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>
#include <task.h>
#include <sched.h>
#include <lib/list.h>


#define SELECTOR_MASK { 0, 0x1, 0x3, 0xF, 0xFF, 0xFFFF }
#define BIT_MASK { 0, 1, 2, 4 ,8 ,16 }

int sched_update_highest( Context* ctx ){
	//find the highest priority queue
	uint selector = ctx->scheduler->selector;
	if ( !selector ) {
		ctx->scheduler->highest_priority = 32;
		return ERR_NONE;
	}
	uint priority = 0;
	uint masks[] = SELECTOR_MASK;
	uint bits[] = BIT_MASK;
	uint i = 5;
	while (i) {
		uint low = selector & masks[i];
		uint high = (selector >> bits[i]) & masks[i];
		if (high)  {
			selector = high;
			priority += bits[i];
		}
		else {
			selector = low;
		}
		i -= 1;
	}
	ctx->scheduler->highest_priority = 31 - priority;
	return ERR_NONE;
}

int sched_init( Context* ctx, Sched* scheduler ){
	scheduler->selector = 0;
	scheduler->highest_priority = 32;
	scheduler->zombie = 0;
	int i;
	for (i=0;i<32;i++){
		scheduler->task_queue[i] = 0;
	}
	ctx->scheduler = scheduler;

	return 0;
}

int sched_schedule( Context* ctx, Task** next ){
	uint selector = ctx->scheduler->selector;
	if ( !selector ) {
		//no task in scheduler
		*next = 0;
		return 0;
	}

	uint priority = ctx->scheduler->highest_priority;
	DEBUG_PRINT( DBG_SCHED,"SCHED_schedule: current priority is %d\n", priority );

	//get the corresponding element
	List* elem = ctx->scheduler->task_queue[priority];

	*next = list_entry( Task, elem, queue);
	(*next)->state = TASK_ACTIVE;

	DEBUG_PRINT( DBG_SCHED,"SCHED_schedule: current ptr is %x\n", ctx->scheduler->task_queue[priority] );

	return 0;
}

int sched_add( Context* ctx, Task* task ){
	uint priority = task->priority;
	ASSERT( (0 <= priority) && (priority < 32) );
	DEBUG_PRINT( DBG_SCHED, "task tid 0x%x priority %d\n", task->tid, priority );

	List** target_queue_ptr = &(ctx->scheduler->task_queue[priority]);
	uint err = list_add_tail( target_queue_ptr, &(task->queue) );
	if (err) {
		return err;
	}
	task->state = TASK_READY;

	//change the bit in selector
	uint selector_modifier = 0x80000000 >> priority;
	ctx->scheduler->selector = ctx->scheduler->selector | selector_modifier;
	err = sched_update_highest( ctx );
	ASSERT( err == 0 );
	DEBUG_PRINT( DBG_SCHED, "selector = 0x%x\n", ctx->scheduler->selector );
	return 0;
}

int sched_kill( Context* ctx, Task* task){
	uint priority = task->priority;
	ASSERT( (0 <= priority) && (priority < 32) );
	List** target_queue_ptr = &(ctx->scheduler->task_queue[priority]);
	List** zombie_queue_ptr = &(ctx->scheduler->zombie);

	List* elem;
	uint err = list_remove_head( target_queue_ptr, &elem );
	if (err) {
		return err;
	}
	DEBUG_PRINT( DBG_SCHED,"current task is %d\n", task->tid );

	if (!(*target_queue_ptr)){
		uint selector_modifier = ~(0x80000000 >> priority);
		ctx->scheduler->selector = ctx->scheduler->selector & selector_modifier;
		err = sched_update_highest( ctx );
		ASSERT( err == 0 );
		DEBUG_PRINT( DBG_SCHED,"selector modified to %x\n", ctx->scheduler->selector );
	}

	err = list_add_tail( zombie_queue_ptr, elem );
	if (err) {
		return err;
	}
	task->state = TASK_ZOMBIE;
	DEBUG_PRINT( DBG_SCHED,"current task is %d\n", task->tid );

	return 0;
}

int sched_pass( Context* ctx, Task* task ){
	DEBUG_PRINT( DBG_SCHED,"SCHED_PASS: current task is %d\n", task->tid );
	DEBUG_PRINT( DBG_SCHED,"SCHED_PASS: current priority is %d\n", task->priority );
	
	uint priority = task->priority;
	DEBUG_PRINT( DBG_SCHED,"SCHED_PASS: current ptr is %x\n", ctx->scheduler->task_queue[priority] );

	List** target_queue_ptr = &(ctx->scheduler->task_queue[priority]);
	uint err = list_rotate_head( target_queue_ptr );
	task->state = TASK_READY;

	DEBUG_PRINT( DBG_SCHED,"SCHED_PASS: renewed ptr is %x\n", ctx->scheduler->task_queue[priority] );

	return err;
}

int sched_block( Context* ctx, Task* task ) {
	//remove from head of the queue
	uint priority = task->priority;
	List** target_queue_ptr = &(ctx->scheduler->task_queue[priority]);
	List* elem;
	uint err = list_remove_head( target_queue_ptr, &elem );
	if ( err ){
		return err;
	}
	ASSERT( task == elem );
	
	task->state = TASK_BLOCK;
	return ERR_NONE;
}

int sched_rcv_block( Context* ctx, Task* task ){
	sched_block( ctx, task );
	task->state = TASK_RCV_BLOCK;
}

int sched_rpl_block( Context* ctx, Task* task ){
	sched_block( ctx, task );
	task->state = TASK_RPL_BLOCK;
}

int sched_send_block( Context* ctx, Task* task ){
	sched_block( ctx, task );
	task->state = TASK_SEND_BLOCK;
}

int sched_signal( Context* ctx, Task* task ){
	sched_add( ctx, task );
}
