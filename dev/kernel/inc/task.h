#ifndef _TASK_H_
#define _TASK_H_

#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>
#include <lib/list.h>
#include <user/syscall.h>

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
	Syscall* reason;
	Task* parent;
	List queue;
};

int task_init_all( Task* array, uint count );
int task_setup( Context* ctx, Task* task, void (*code)(), Task* parent, uint priority );
uint task_tid( Task* task );
uint task_array_index( Task* task );

#endif /* _TASK_H_ */
