#ifndef _TASK_H_
#define _TASK_H_

#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>

typedef struct Task_s {
	uint tid;
	uint state;
	uint priority;
	ptr stack;
	struct Task_s* parent;
	struct Task_s* next;
} Task;

int task_setup( Context* ctx, Task* ctx, ptr stack, void (*code)() );
int task_start( Context* ctx, Task* ctx );

#endif /* _TASK_H_ */
