#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>
#include <task.h>
#include <sched.h>
#include <lib/list.h>


#define SELECTOR_MASK { 0, 0x1, 0x3, 0xF, 0xFF, 0xFFFF }
#define BIT_MASK { 0, 1, 2, 4 ,8 ,16 }


int sched_init( Context* ctx, Sched* scheduler ){
	scheduler->selector = 0;
	scheduler->cur_priority = 32;
	int i;
	for (i=0;i<32;i++){
		scheduler->task_queue[i] = 0;
	}
	ctx->scheduler = scheduler;

	return 0;
}

int sched_schedule( Context* ctx, Task** next ){
	uint selector = ctx->scheduler->selector;
	if (selector == 0) {
		//no task in scheduler
		*next = 0;
		return 0;
	}

	//find the highest priority queue
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

	//get the corresponding element
	List* elem = ctx->scheduler->task_queue[priority];

	*next = list_entry( Task, elem, queue);
	return 0;
}

int sched_add( Context* ctx, Task* task ){
	uint priority = task->priority;
	ASSERT( (0 <= priority) && (priority < 32) );

	List* target_queue = ctx->scheduler->task_queue[priority];
	uint err = list_add_tail( target_queue, &(task->queue) );
	if (err) {
		return err;
	}

	//change the bit in selector
	uint selector_modifier = 0x80000000 >> priority;
	ctx->scheduler->selector = ctx->scheduler->selector | selector_modifier;
	return 0;
}

int sched_kill( Context* ctx, Task* task){
	uint priority = task->priority;
	ASSERT( (0 <= priority) && (priority < 32) );
	List* target_queue = ctx->scheduler->task_queue[priority];
	List* zombie_queue = ctx->scheduler->zombie;

	List* elem;
	uint err = list_remove_head( &(target_queue), &elem );
	if (err) {
		return err;
	}
	err = list_add( &(zombie_queue), elem );
	if (err) {
		return err;
	}
	return 0
}
