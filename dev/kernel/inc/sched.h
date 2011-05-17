#ifndef _SCHED_H_
#define _SCHED_H_

#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>

struct Sched_s {
	uint selector;
	List* task_queue[32];
	List* zombie;
};

int sched_init( Context* ctx, Sched* scheduler );
int sched_schedule( Context* ctx, Task** next );
int sched_add( Context* ctx, Task* task );
int sched_kill( Context* ctx, Task* task );



#endif /* _SCHED_H_ */
