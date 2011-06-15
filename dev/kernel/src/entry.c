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
#include <devices/console.h>
#include <devices/clock.h>
#include <user/apps_entry.h>
#include <interrupt.h>
#include <watchdog.h>

int main()
{
	Context ctxbody = {0};
	Context* ctx = &ctxbody;

	/* Scheduler */
	Sched scheduler;

	/* Interrupt manager */
	Interrupt_mgr interrupt_mgr;
	Task* interrupt_handlers[ INTERRUPT_COUNT ] = { 0 };

	/* Console descriptor */
	Console console_descriptors[ KERNEL_NUM_CONSOLES ] = {{0}};

	/* Clock descriptors */
	Clock clock_descriptors[ CLK_COUNT ] = {{0}};

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

	/* Initialize interrupt manager data */
	interrupt_mgr.interrupt_handlers = interrupt_handlers;
	ctx->interrupt_mgr = &interrupt_mgr;

	/* Initialize all the tids */
	status = task_init_all( task_descriptors, KERNEL_MAX_NUM_TASKS );
	ASSERT( status == ERR_NONE );
	
	ctx->task_array = task_descriptors;

	DEBUG_NOTICE( DBG_KER, "Task init done\n" );

	status = ctx_init( ctx );
	ASSERT( status == ERR_NONE );

	/* Assign consoles */
	ctx->mem = &membody;
	ctx->terminal = console_descriptors;
	ctx->train_set = console_descriptors + 1;

	/* Assign timers */
	ctx->timer_clk = clock_descriptors + CLK_3;

	/* Initialize the memory manager */
	/* Tasks */
	status = mem_init( ctx, MEM_TASK, &task_ring, ( uchar* )task_container, ( uchar* )task_descriptors, sizeof( Task ), KERNEL_MAX_NUM_TASKS );
	ASSERT( status == ERR_NONE );
	/* Stack */
	status = mem_init( ctx, MEM_STACK, &stack_ring, ( uchar* )stack_container, ( uchar* )stack_space, KERNEL_PAGE_SIZE * USER_STACK_PAGE, KERNEL_MAX_NUM_TASKS );
	ASSERT( status == ERR_NONE );

	DEBUG_NOTICE( DBG_KER, "Memory manager init done\n" );
	
	/* Init Scheduler */
	sched_init( ctx, &scheduler );

	DEBUG_NOTICE( DBG_KER, "Sched init done\n" );

	/* Init Kernel */
	kernel_init( ctx );

	DEBUG_NOTICE( DBG_KER, "Kernel init done\n" );

	/* Start user session by getting into the init */
	status = task_setup( ctx, &user_init_td, user_init, 0, KERNEL_INIT_TASK_PRIORITY, 1 );
	ASSERT( status == ERR_NONE );

	DEBUG_PRINT( DBG_KER, "User session, td 0x%x, sp 0x%x\n", user_init_td, user_init_td->stack );

	/* Hardcode first user task */
	ctx->current_task = user_init_td;

	session_start( ctx, (uchar*)user_init_td->stack );

	/* Disable watchdog */
	watchdog_deinit();

	return 0;
}
