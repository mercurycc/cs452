/* Pinball entry point */

#include <types.h>
#include <err.h>
#include <config.h>
#include <kernel.h>
#include <trap.h>
#include <task.h>
#include <sched.h>
#include <context.h>
#include <mem.h>
#include <session.h>
#include <user/apps_entry.h>

int main()
{
	Context ctxbody = {0};
	Context* ctx = &ctxbody;

	/* Console descriptor */
	Console console_descriptors[ KERNEL_NUM_CONSOLES ] = {{0}};

	/* Memory manager */
	Memmgr membody = {0};

	/* Memory block for task descriptors */
	Rbuf task_ring = {0};
	Task* task_container[ KERNEL_MAX_NUM_TASKS + 1 ] = { 0 };
	Task task_descriptors[ KERNEL_MAX_NUM_TASKS ] = {{0}};

	/* Memory block for stack pointers */
	Rbuf stack_ring = {0};
	uchar* stack_container[ KERNEL_MAX_NUM_TASKS + 1 ] = { 0 };
	uchar* stack_space = 0;

	/* First task information */
	Task* user_init_td = 0;
	int status = 0;

	/* Calculation of stack base pointer */
	{
		uint stack_end = ( uint )( &stack_space ) - KERNEL_STACK_PAGE * KERNEL_PAGE_SIZE;
		uint stack_space_size = KERNEL_MAX_NUM_TASKS * USER_STACK_PAGE * KERNEL_PAGE_SIZE;
		
		stack_space = ( uchar* )( stack_end - stack_space_size );
		/* Make stack page-aligned */
		stack_space -= (uint)stack_space % KERNEL_PAGE_SIZE;
	}
	
	DEBUG_PRINT( DBG_KER, "Intial stack pointer: 0x%x, ctx: 0x%x\n", stack_space, ctx );

	/* Initialize all the tids */
	status = task_init_all( task_descriptors, KERNEL_MAX_NUM_TASKS );
	ASSERT( status == ERR_NONE );

	status = ctx_init( ctx );
	ASSERT( status == ERR_NONE );
	
	ctx->mem = &membody;
	ctx->terminal = console_descriptors;
	ctx->train_set = console_descriptors + 1;

	/* Initialize the memory manager */
	/* Tasks */
	status = mem_init( ctx, MEM_TASK, &task_ring, ( uchar* )task_container, ( uchar* )task_descriptors, sizeof( Task ), KERNEL_MAX_NUM_TASKS );
	ASSERT( status == ERR_NONE );
	/* Stack */
	status = mem_init( ctx, MEM_STACK, &stack_ring, ( uchar* )stack_container, ( uchar* )stack_space, KERNEL_PAGE_SIZE * USER_STACK_PAGE, KERNEL_MAX_NUM_TASKS );
	ASSERT( status == ERR_NONE );
	
	/* Init Scheduler */
	Sched scheduler;
	sched_init( ctx, &scheduler );

	/* Init Kernel */
	kernel_init( ctx );

	DEBUG_NOTICE( DBG_KER, "Kernel init done\n" );

	/* Start user session by getting into the init */
	status = mem_alloc( ctx, MEM_TASK, ( void** )&user_init_td, 1 );
	ASSERT( status == ERR_NONE );

	status = task_setup( ctx, user_init_td, init_user, 0, 0 );
	ASSERT( status == ERR_NONE );

	DEBUG_PRINT( DBG_KER, "User session, td 0x%x, sp 0x%x\n", user_init_td, user_init_td->stack );

	/* Hack for testing */
	ctx->current_task = user_init_td;

	/* TODO: We do need to save the kernel context here on the
	   stack if we ever want to correctly exit the kernel.  To
	   exit, just restore the kernel context we saved here and
	   continue execution. */
	session_start( ctx, (uchar*)user_init_td->stack );

	ASSERT_M( 0, "We should never reach here! %s\n", "crap" );
	
	return 0;
}
