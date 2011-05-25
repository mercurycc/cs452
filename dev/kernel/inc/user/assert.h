#ifndef _USER_ASSERT_H_
#define _USER_ASSERT_H_

#include <bwio.h>

#define assert( cond )							\
	do {								\
		if( ! ( cond ) ){					\
			bwprintf( COM2, "%s (%d) %s assert: %s\n", __FILE__, __LINE__, __func__, #cond ); \
			while( 1 );					\
		}							\
	} while( 0 )

#endif /* _USER_ASSERT_H_ */
