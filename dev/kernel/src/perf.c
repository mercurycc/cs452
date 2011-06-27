#include <types.h>
#include <perf.h>
#include <ts7200.h>
#include <regopts.h>
#include <err.h>

uint perf_init()
{
	HW_WRITE( PERF_TIMER_BASE, PERF_TIMER_VALH_OFFSET, PERF_TIMER_EN_MASK );

	return ERR_NONE;
}

uint perf_deinit()
{
	HW_WRITE( PERF_TIMER_BASE, PERF_TIMER_VALH_OFFSET, ~PERF_TIMER_EN_MASK );

	return ERR_NONE;
}	

uint perf_timer_time()
{
	return HW_READ( PERF_TIMER_BASE, PERF_TIMER_VALL_OFFSET );
}

uint perf_timer_clear()
{
	perf_deinit();
	return perf_init();
}
