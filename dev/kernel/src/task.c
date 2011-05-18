#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>
#include <task.h>
#include <mem.h>
#include <lib/list.h>
#include <sched.h>

/* As defined in task_init.s */
ptr task_init( void (*code)(), ptr stack );

static inline uint task_next_tid( uint tid )
{
	tid += (1 << 16) ;

	return tid;
}

int task_init_all( Task* array, uint count )
{
	uint tid = 0;

	for( tid = 0; tid < count; tid += 1 ){
		array[ tid ].tid = tid;
	}

	return ERR_NONE;
}

int task_setup( Context* ctx, Task* task, void (*code)(), Task* parent, uint priority )
{
	DEBUG_PRINT( DBG_TASK, "priority %d\n",priority );

	ptr stack = 0;
	int status = 0;

	/* Allocate user stack */
	status = mem_alloc( ctx, MEM_STACK, ( void** )&stack, 1 );
	ASSERT( status == ERR_NONE );
	
	task->tid = task_next_tid( task->tid );
	task->stack = task_init( code, stack );
	DEBUG_PRINT( DBG_TASK, "priority %d\n",priority );
	task->priority = priority;
	task->state = TASK_READY;
	task->reason = 0;
	task->parent = parent;

	/* Clear queue for scheduler */
	list_init( &task->queue );


	DEBUG_PRINT( DBG_TASK, "task tid 0x%x priority %d\n", task->tid, task->priority );
	status = sched_add( ctx, task );
	ASSERT( status == ERR_NONE );

	return ERR_NONE;
}

uint task_tid( Task* task )
{
	if( ! task ){
		return 0;
	} else {
		return task->tid;
	}
}

uint task_array_index( Task* task )
{
	return task->tid & 0xffff;
}
