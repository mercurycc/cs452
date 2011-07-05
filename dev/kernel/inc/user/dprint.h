#ifndef _USER_DPRINT_H_
#define _USER_DPRINT_H_

#include <user/display.h>

#ifdef LOCAL_DEBUG
#define dprintf( fmt, arg... )				\
	do {						\
		scroll_printf( fmt, arg );		\
	} while( 0 )

#define dnotice( msg )					\
	do {						\
		scroll_printf( msg );			\
	} while( 0 )
#else
#define dprintf( fmt, arg... )
#define dnotice( msg )
#endif

#endif /* _USER_DPRINT_H_ */
