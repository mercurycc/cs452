#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>
#include <task.h>
#include <sched.h>
#include <lib/list.h>
#include <ts7200.h>
#include <regopts.h>


#define SELECTOR_MASK { 0, 0x1, 0x3, 0xF, 0xFF, 0xFFFF }
#define BIT_MASK { 0, 1, 2, 4 ,8 ,16 }

int sched_update_highest( Context* ctx ){
	// find the highest priority queue
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
	scheduler->blocked_task = 0;
	int i;
	for (i=0;i<32;i++){
		scheduler->task_queue[i] = 0;
	}
	ctx->scheduler = scheduler;

	// enable halt on cpu, need to turn off swlock
	uint reg_val = HW_READ( CPU_POWER_ADDR, CPU_DEVICECFG_OFFSET );
	HW_WRITE( CPU_POWER_ADDR, CPU_SYSSWLOCK_OFFSET, CPU_SWLOCK_MASK );
	HW_WRITE( CPU_POWER_ADDR, CPU_DEVICECFG_OFFSET, CPU_SHENA_MASK|reg_val );
	HW_WRITE( CPU_POWER_ADDR, CPU_SYSSWLOCK_OFFSET, 0 );
	reg_val = HW_READ( CPU_POWER_ADDR, CPU_DEVICECFG_OFFSET );
	DEBUG_PRINT( DBG_SCHED,"device config val 0x%x is now set to 0x%x\n", reg_val, CPU_SHENA_MASK|reg_val );
	return 0;
}

int sched_schedule( Context* ctx, Task** next ){
	uint selector = ctx->scheduler->selector;
	if ( !selector ) {
		if ( ctx->scheduler->blocked_task ) {
			DEBUG_PRINT( DBG_SCHED,"Currently blocked %d tasks\n", ctx->scheduler->blocked_task );
			uint reg_val = HW_READ( CPU_POWER_ADDR, CPU_HALT_OFFSET );
			DEBUG_PRINT( DBG_SCHED,"reg val read 0x%x\n", reg_val );
			// TODO: how to handle this new interrupt?
			ASSERT( 0 );
			return 0;
		}
		// no task in scheduler
		*next = 0;
		return 0;
	}

	uint priority = ctx->scheduler->highest_priority;

	// get the corresponding element
	List* elem = ctx->scheduler->task_queue[priority];

	*next = list_entry( Task, elem, queue);
	(*next)->state = TASK_ACTIVE;

	DEBUG_PRINT( DBG_SCHED,"selected task 0x%x at priority %d\n", (*next)->tid, priority );

	return 0;
}

int sched_add( Context* ctx, Task* task ){
	uint priority = task->priority;
	DEBUG_PRINT( DBG_SCHED, "task tid 0x%x priority %d\n", task->tid, priority );
	ASSERT( (0 <= priority) && (priority < 32) );

	List** target_queue_ptr = &(ctx->scheduler->task_queue[priority]);
	uint err = list_add_tail( target_queue_ptr, &(task->queue) );
	if (err) {
		return err;
	}
	task->state = TASK_READY;

	// change the bit in selector
	uint selector_modifier = 0x80000000 >> priority;
	ctx->scheduler->selector = ctx->scheduler->selector | selector_modifier;

	if ( priority < ctx->scheduler->highest_priority ) {
		ctx->scheduler->highest_priority = priority;
	}
	//DEBUG_PRINT( DBG_SCHED, "selector = 0x%x\n", ctx->scheduler->selector );
	return 0;
}

int sched_kill( Context* ctx, Task* task){
	/* TODO: Combine the first portion with sched_block */
	uint priority = task->priority;
	ASSERT( (0 <= priority) && (priority < 32) );
	List** target_queue_ptr = &(ctx->scheduler->task_queue[priority]);
	List** zombie_queue_ptr = &(ctx->scheduler->zombie);

	List* elem;
	uint err = list_remove_head( target_queue_ptr, &elem );

	ASSERT( &(task->queue) == elem );
	
	if (err) {
		return err;
	}
	DEBUG_PRINT( DBG_SCHED,"current task is %d\n", task->tid );

	if ( !(*target_queue_ptr) ){
		uint selector_modifier = ~(0x80000000 >> priority);
		ctx->scheduler->selector = ctx->scheduler->selector & selector_modifier;
		err = sched_update_highest( ctx );
		ASSERT( err == ERR_NONE );
		DEBUG_PRINT( DBG_SCHED,"selector modified to %x\n", ctx->scheduler->selector );
	}

	err = list_add_tail( zombie_queue_ptr, elem );
	if (err) {
		return err;
	}
	task->state = TASK_ZOMBIE;
	//DEBUG_PRINT( DBG_SCHED,"current task is %d\n", task->tid );

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

int sched_block( Context* ctx ) {
	// remove self from head of the queue
	Task* task = ctx->current_task;
	uint priority = task->priority;
	List** target_queue_ptr = &(ctx->scheduler->task_queue[priority]);
	List* elem;
	uint err = list_remove_head( target_queue_ptr, &elem );
	if ( err ){
		return err;
	}
	ASSERT( &(task->queue) == elem );

	if ( !(*target_queue_ptr) ){
		uint selector_modifier = ~(0x80000000 >> priority);
		ctx->scheduler->selector = ctx->scheduler->selector & selector_modifier;
		err = sched_update_highest( ctx );
		ASSERT( err == ERR_NONE );
		DEBUG_PRINT( DBG_SCHED,"selector modified to %x\n", ctx->scheduler->selector );
	}

	DEBUG_PRINT( DBG_SCHED, "task blocked: 0x%x\n", task->tid );
	task->state = TASK_BLOCK;
	//update number of blocked tasks
	ctx->scheduler->blocked_task++;
	ASSERT( ctx->scheduler->blocked_task > 0 );
	return ERR_NONE;
}

int sched_signal( Context* ctx, Task* task ){
	int status = sched_add( ctx, task );
	if ( status == ERR_NONE )
		ctx->scheduler->blocked_task--;
	ASSERT( ctx->scheduler->blocked_task >= 0 );
	return status;
}
