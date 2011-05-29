#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <types.h>
#include <err.h>
#include <devices/console.h>
#include <config.h>

/* Circular inclusion */
/* TODO: add all the typedefs into types.h */

struct Context_s {
	Console* terminal;
	Console* train_set;
	Memmgr* mem;
	Task* task_array;
	Task* current_task;
	Task* last_task;
	uint next_tid;
	Sched* scheduler;
	Clock* timer_clk;
};

/* Ensure ctx is cleared */
int ctx_init( Context* ctx );

#endif /* _CONTEXT_H_ */
