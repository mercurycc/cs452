#ifndef _RBUF_H_
#define _RBUF_H_

#include <types.h>

typedef struct Rbuf_t {
	uchar* buffer;         /* Buffer pointer */
	uint element_size;     /* Size of each element */
	uint count;            /* Number of elements that can be fit in */
	uint put;              /* Write to here */
	uint get;              /* Read from here */
} Rbuf;

int rbuf_init( Rbuf* buf, uchar* buffer, uint element_size, uint size );
int rbuf_reset( Rbuf* buf );
int rbuf_empty( Rbuf* buf );
int rbuf_put_front( Rbuf* buf, const uchar* data );
int rbuf_put( Rbuf* buf, const uchar* data );
int rbuf_get( Rbuf* buf, uchar* data );

#endif /* _RBUF_H_ */
