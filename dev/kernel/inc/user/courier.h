#ifndef _USER_COURIER_H_
#define _USER_COURIER_H_

#include <types.h>

typedef int (*Courier_callback)( int tid, int arg );

void courier();
int courier_init( int courier_tid, Courier_callback callback );
int courier_go( int courier_tid, int tid, uint arg );
int courier_quit( int courier_tid );

#endif /* _USER_COURIER_H_ */
