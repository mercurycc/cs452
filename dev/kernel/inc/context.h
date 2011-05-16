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
	Task* current_task;
	Task* last_task;
	uint next_tid;
	Sched* scheduler;
};

/* Ensure ctx is cleared */
int ctx_init( Context* ctx );
uint ctx_next_tid( Context* ctx );

#endif /* _CONTEXT_H_ */
