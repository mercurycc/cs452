#ifndef _USER_TIME_H_
#define _USER_TIME_H_

enum TimeError {
	TIME_TID_INVALID = -1,
	TIME_TID_NOT_CLOCK = -2
};

int time_ask( int tid );
int time_delay( int tid, int interval );
int time_delay_until( int tid, int time );
int time_signal( int tid );
int time_suicide( int tid );


#endif
