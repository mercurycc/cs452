/* Devices */
#include <devices/clock.h>
#include <devices/console.h>

/* Trap handler */
#include <trap.h>

/* Interrupt */
#include <interrupt.h>

/* Task */
#include <task.h>
#include <sched.h>

/* Context */
#include <context.h>

int kernel_init( Context* ctx )
{
	int status = 0;
	
	/* Install trap handler */
	trap_init( ctx );

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

	/* Initialize interrupt */
	interrupt_init( ctx );

	return ERR_NONE;
}
