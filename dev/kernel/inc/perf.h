/* Performance measures */
#ifndef _PERF_H_
#define _PERF_H_

#include <types.h>
#include <config.h>

/* As per ep9302 manual */
/* Notice given this frequency the perf will only be available for at most 1
   hour 12 minutes, at which if a timing is request across the boundary it will
   screw up */
#define PERF_TIMER_FREQ   983

#define PERF_TIMER_TO_USEC( cycles )   ( ( cycles ) * 1000 / PERF_TIMER_FREQ )

uint perf_init();
uint perf_deinit();
uint perf_timer_time();        /* Report clk_4 read low */
uint perf_timer_clear();       /* Reset clk_4 read */

#endif /* _PERF_H_ */
