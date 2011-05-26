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
	tid += KERNEL_MAX_NUM_TASKS;

	return tid;
}

int task_init_all( Task* array, uint count )
{
	uint tid = 0;

	for( tid = 0; tid < count; tid += 1 ){
		array[ tid ].tid = tid + 1;
		array[ tid ].state = TASK_UNUSED;
	}

	return ERR_NONE;
}

int task_setup( Context* ctx, Task** task, void (*code)(), Task* parent, uint priority )
{
	Task* newtask;
	ptr stack = 0;
	int status = 0;

	DEBUG_PRINT( DBG_TASK, "priority %d\n",priority );

	if( priority < KERNEL_HIGH_PRIORITY || priority > KERNEL_LOW_PRIORITY ){
		return ERR_INVALID_PRIORITY;
	}

	status = mem_alloc( ctx, MEM_TASK, ( void** )&newtask, 1 );
	if( status == ERR_OUT_OF_MEMORY ){
		return ERR_OUT_OF_TASK_DESCRIPTOR;
	}

	/* Allocate user stack */
	status = mem_alloc( ctx, MEM_STACK, ( void** )&stack, 1 );
	ASSERT( status == ERR_NONE );
	
	newtask->stack_orig = stack;
	newtask->stack = task_init( code, stack );
	DEBUG_PRINT( DBG_TASK, "priority %d\n",priority );
	newtask->priority = priority;
	newtask->state = TASK_READY;
	newtask->reason = 0;
	newtask->parent_tid = task_tid( parent );
	newtask->parent = parent;
	newtask->send_queue = 0;

	/* Clear queue for scheduler */
	list_init( &newtask->queue );

	DEBUG_PRINT( DBG_TASK, "task tid 0x%x priority %d\n", newtask->tid, newtask->priority );
	status = sched_add( ctx, newtask );
	ASSERT( status == ERR_NONE );

	/* Return task if asked */
	if( task ){
		*task = newtask;
	}

	return ERR_NONE;
}

int task_zombiefy( Context* ctx, Task* task )
{
	int status = 0;

	ASSERT( task );
	
	task->tid = task_next_tid( task->tid );

	status = mem_free( ctx, MEM_STACK, ( void** )task->stack_orig, 1 );
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

int task_parent_tid( Task* task, int* ret )
{
	ASSERT( task );
	if( task->parent_tid != task_tid( task->parent ) ){
		return ERR_PARENT_EXIT;
	}

	*ret = task->parent->tid;
	
	return ERR_NONE;
}

uint task_array_index( Task* task )
{
	return task_array_index_tid( task->tid );
}

uint task_array_index_tid( uint tid )
{
	return ( tid & ( KERNEL_MAX_NUM_TASKS - 1 ) ) - 1;
}

Task* task_get_by_tid( Context* ctx, uint tid )
{
	Task* ret = 0;

	/* Invalid tid count as non-existent */
	if( tid > KERNEL_MAX_NUM_TASKS ){
		return 0;
	}

	ret = ctx->task_array + task_array_index_tid( tid );

	if( ( ret->tid == tid ) && ( ret->state != TASK_UNUSED ) && ( ret->state != TASK_ZOMBIE ) ){
		return ret;
	}

	return 0;
}
