#ifndef _TASK_H_
#define _TASK_H_

#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>
#include <lib/list.h>
#include <user/syscall.h>

/* Tid end with 0x00 used for special purpose, so tid start with 1 */
enum TaskStates {
	TASK_UNUSED,
	TASK_READY,
	TASK_ACTIVE,
	TASK_ZOMBIE,
	TASK_BLOCK,
	TASK_RCV_BLK,
	TASK_RPL_BLK,
	TASK_SEND_BLK
};

struct Task_s {
	uint tid;
	uint state;
	uint priority;
	ptr stack_orig;                 /* For freeing the stack */
	ptr stack;
	uint parent_tid;
	Syscall* reason;
	Task* parent;
	List queue;
	List* send_queue;
};

int task_init_all( Task* array, uint count );
/* Initialize new task, return the TD at task */
int task_setup( Context* ctx, Task** task, void (*code)(), Task* parent, uint priority, int interrupt );
int task_zombiefy( Context* ctx, Task* task );
uint task_tid( Task* task );
int task_parent_tid( Task* task, int* ret );
uint task_array_index( Task* task );
uint task_array_index_tid( uint tid );
/* Return 0 if tid does not map to an existing task */
Task* task_get_by_tid( Context* ctx, uint tid );

#endif /* _TASK_H_ */
