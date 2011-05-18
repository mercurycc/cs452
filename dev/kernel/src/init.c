/* Devices */
#include <devices/clock.h>
#include <devices/console.h>

/* Trap handler */
#include <trap.h>

/* Interrupt */
// #include <interrupt.h>

/* Task */
#include <task.h>
#include <sched.h>

/* Context */
#include <context.h>

/* Memory management */
#include <mem.h>

/* User space */
#include <session.h>

int kernel_init( Context* ctx )
{
	int status = 0;
	
	/* Install trap handler */
	trap_init( ctx );

	/* We need at least 2 clocks: one for preemption (not needed
	   for K1), one for timing */
	// clock_init( ctx );

	/* Initialize terminal console */
	status = console_setup( ctx->terminal, CONSOLE_2, 115200, 0, 0, 0 );
	ASSERT( status == ERR_NONE );
	status = console_init( ctx->terminal );
	ASSERT( status == ERR_NONE );

	/* Initialize train console */
	status = console_setup( ctx->train_set, CONSOLE_1, 2400, 0, 1, 1 );
	ASSERT( status == ERR_NONE );
	status = console_init( ctx->train_set );
	ASSERT( status == ERR_NONE );

	return ERR_NONE;
}
