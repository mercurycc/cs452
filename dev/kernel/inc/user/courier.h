#ifndef _USER_COURIER_H_
#define _USER_COURIER_H_

#include <types.h>

void courier();
int courier_init( int courier_tid, int (*callback)( int tid, int arg ) );
int courier_go( int courier_tid, int tid, int arg );
int courier_quit( int courier_tid );

#endif /* _USER_COURIER_H_ */
