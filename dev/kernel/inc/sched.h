#ifndef _SCHED_H_
#define _SCHED_H_

#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>

struct Sched_s {
	uint selector;
	uint highest_priority;
	List* task_queue[32];
	List* zombie;
	int blocked_task;
	int task_count;
};

int sched_init( Context* ctx, Sched* scheduler );
int sched_schedule( Context* ctx, Sched* sched, Task** next );
int sched_add( Context* ctx, Sched* sched, Task* task );
int sched_kill( Context* ctx, Sched* sched, Task* task );
int sched_pass( Context* ctx, Sched* sched, Task* task );

int sched_block( Context* ctx, Sched* sched );

int sched_signal( Context* ctx, Sched* sched, Task* task );


#endif /* _SCHED_H_ */
