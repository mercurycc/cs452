#include <sched.h>

#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>

#define SELECTOR_MASK { 0xFFFF, 0xFF, 0xF, 0x3, 0x1, 0 }
#define BIT_MASK { 16, 8, 4, 2, 1, 0 }


int sched_init( Context* ctx, Sched* scheduler ){
	scheduler->selector = 0;
	int i;
	for (i=0;i<32;i++){
		scheduler->stack_queue[i] = 0;
	}
	ctx->scheduler = scheduler;
}

int sched_schedule( Context* ctx, Task** next ){

	uint selector = ctx->scheduler->selector;
	uint priority = 0;
	uint masks[] = SELECTOR_MASK;
	uint bits[] = BIT_MASK;
	uint i = 5;

	while (i) {
		uint high = selector & masks[i];
		uint low = (selector >> bits[i]) & mask[i];
		if (high)  {
			selector = high;
			priority += bits[i]l
		}
		else {
			selector = low;
		}
		i -= 1;
	}
	priority -= 1;

	List* elem;
	uint err = list_remove_head( &(ctx->scheduler->task_queue[priority]), &elem );
	if (err) {
		return err;
	}

	*next = elem;
	return 0;
}

int sched_add( Context* ctx, Task* task, uint priority ){
	List* target_queue = ctx->scheduler->task_queue[priority];
	uint err = list_add_tail( target_queue, &(task->ready_queue) )
	if (err) {
		return err;
	}

	//change the bit in selector
	uint selector_modifier = 0x80000000 >> priority;
	ctx->scheduler->selector = ctx->scheduler->selector | selector_modifier;
	return 0;
}


