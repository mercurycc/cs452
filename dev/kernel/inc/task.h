#ifndef _TASK_H_
#define _TASK_H_

#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>
#include <lib/list.h>

enum TaskStates {
	TASK_READY,
	TASK_ACTIVE,
	TASK_ZOMBIE
};

struct Task_s {
	uint tid;
	uint state;
	uint priority;
	ptr stack;
	Task* parent;
	List ready_queue;
};

int task_setup( Context* ctx, Task* task, void (*code)(), Task* parent, uint priority );

#endif /* _TASK_H_ */
