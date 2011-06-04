#ifndef _USER_CLOCK_SERVER_H_
#define _USER_CLOCK_SERVER_H_

#define CLOCK_SERVER_TID 0x3

int Time();
int Delay( int ticks );
int DelayUntil( int ticks );

#endif

