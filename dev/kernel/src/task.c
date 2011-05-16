#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>
#include <task.h>
#include <mem.h>

/* As defined in task_init.s */
ptr task_init( void (*code)(), ptr stack );

int task_setup( Context* ctx, Task* task, void (*code)(), Task* parent, uint priority )
{
	ptr stack = 0;
	int status = 0;

	/* Allocate user stack */
	status = mem_alloc( ctx, MEM_STACK, ( void** )&stack, 1 );
	ASSERT( status == ERR_NONE );
	
	task->tid = ctx_next_tid( ctx );
	task->stack = task_init( code, stack );
	DEBUG_PRINT( DBG_TASK, "old stack 0x%x, new 0x%x, diff 0x%x\n", stack, task->stack, stack - task->stack );
	task->priority = priority;
	task->state = TASK_READY;
	task->parent = parent;

	return ERR_NONE;
}
