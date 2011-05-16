#include <sched.h>

#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>

int sched_init( Context* ctx, Sched* scheduler ){
	scheduler->selector = 0;
	int i;
	for (i=0;i<32;i++){
		scheduler->stack_queue[i] = 0;
	}
	ctx->scheduler = scheduler;
}

int sched_schedule( Context* ctx, Task** next ){
	
}

int sched_add( Context* ctx, Task* task, uint priority ){
	List* target_queue = ctx->scheduler->task_queue[priority];	
	list_add_tail( target_queue, &(task->ready_queue) );
	
	//change the bit in selector
	uint selector_modifier = 0x80000000 >> priority;
	ctx->scheduler->selector = ctx->scheduler->selector | selector_modifier;
}

