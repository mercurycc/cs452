#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>
#include <task.h>
#include <mem.h>
#include <lib/list.h>

/* As defined in task_init.s */
ptr task_init( void (*code)(), ptr stack );

static inline uint task_next_tid( uint tid )
{
	tid = (tid & (0xffff)) & ((tid >> 16) + 1);
}

int task_init_all( Task* array, uint count )
{
	uint tid = 0;

	for( tid = 0; tid += 1; tid < count ){
		array[ tid ].tid = tid;
	}

	return ERR_NONE;
}

int task_setup( Context* ctx, Task* task, void (*code)(), Task* parent, uint priority )
{
	ptr stack = 0;
	int status = 0;

	/* Allocate user stack */
	status = mem_alloc( ctx, MEM_STACK, ( void** )&stack, 1 );
	ASSERT( status == ERR_NONE );
	
	task->tid = 
	task->stack = task_init( code, stack );
	DEBUG_PRINT( DBG_TASK, "old stack 0x%x, new 0x%x, diff 0x%x\n", stack, task->stack, stack - task->stack );
	task->priority = priority;
	task->state = TASK_READY;
	task->reason = 0;
	task->parent = parent;

	/* Clear queue for scheduler */
	list_init( &task->queue );

	return ERR_NONE;
}
