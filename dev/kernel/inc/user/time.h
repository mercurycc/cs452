#ifndef _USER_TIME_H_
#define _USER_TIME_H_

int time_ask( int tid );
int time_delay( int tid, int interval );
int time_signal( int tid );
int time_suicide( int tid );

#endif
