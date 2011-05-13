/* Devices */
#include <arch/clock.h>
#include <arch/uart.h>

/* Trap handler */
#include <arch/trap.h>

/* Interrupt */
#include <arch/interrupt.h>

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
	/* Initialize kernel context */
	// ctx_init( ctx );

	/* Initialize kernel memory management */
	// mem_init( ctx );

	/* Install trap handler */
	/* Trap handler should call kernel_trap_handler and provide
	   the trap reason, where kernel_trap_handler would handle the
	   interrupt */
	// trap_init( ctx );

	/* We need at least 2 clocks: one for preemption (not needed
	   for K1), one for timing */
	// clock_init( ctx );

	/* Here we need to initialize all uarts */
	uart_init( ctx );

	/* Initialize scheduler */
	// sched_init( ctx );

	return ERR_NONE;
}

int kernel_shutdown( Context* ctx )
{
	return ERR_NONE;
}
