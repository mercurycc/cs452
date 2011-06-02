#ifndef _USER_DEVICES_CLOCK_H_
#define _USER_DEVICES_CLOCK_H_

#define CLOCK_COUNT_DOWN_MS_PER_TICK      10

int clock_current_time( int tid, uint* time );
int clock_count_down( int tid, uint ticks );
int clock_quit( int tid );

#endif /* _USER_DEVICES_CLOCK_H_ */
