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

/* Watchdog */
#include <watchdog.h>

/* Memory */
#include <mem.h>

/* CLK_4 */
#include <perf.h>

#include <ts7200.h>

#include <config.h>

int kernel_init( Context* ctx )
{
	int status = 0;
	volatile uint* clkset1 = ( uint* )CLKSET1_ADDR;
	int val;
	
	/* Init processor */
	val = *clkset1;
	val &= ( ~CLKSET1_FCLKDIV_MASK );
	*clkset1 = val;

	/* Required to set the clock */
	asm volatile( "nop\n\t"
		      "nop\n\t"
		      "nop\n\t"
		      "nop\n\t"
		      "nop\n\t" );

	/* Enable I and D cache */
	cache_init();

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

	/* Initialize perf clock */
	status = perf_init();
	ASSERT( status == ERR_NONE );
	status = perf_timer_clear();
	ASSERT( status == ERR_NONE );
	
	/* Initialize interrupt */
	interrupt_init( ctx );

#ifdef KERNEL_ENABLE_WATCHDOG
	/* Initialize watchdog */
	watchdog_init();
#endif

	return ERR_NONE;
}
