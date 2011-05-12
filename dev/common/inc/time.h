#ifndef _TIME_H_
#define _TIME_H_

#include <types.h>

typedef struct Timespec_s {
	uint day;
	uint hour;
	uint min;
	uint sec;
	uint msec;
} Timespec;

int time_init();

/* Return uptime in milli-seconds */
int time_current( uint* wrap, time_t* current );

/* Return 1 if current time > last + duration, 0 o/w */
/* Due to range limit, this routine can at most time around 49 days.
   For safety, time atmost 48 days. */
int time_timer( time_t last, uint duration );

/* Convert uptime to timespec */
int time_convert( Timespec* out );	

#endif /* _TIME_H_ */
