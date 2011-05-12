/* One-shot Buffered I/O */
/* The user is responsible for providing a buffer */
#ifndef _IOBUF_H_
#define _IOBUF_H_

#include <types.h>
#include <console.h>
#include <err.h>

typedef struct Iobuf_s {
	Console* cons;
	uchar* buffer;
	int count;
	int remain;
} Iobuf;

int iobuf_init( Iobuf* container, Console* cons );
int iobuf_flushed( Iobuf* buf );
int iobuf_flush( Iobuf* buf );
/* iobuf_write will not flush the buffer */
int iobuf_write( Iobuf* buf, uchar* data, uint size );
/* Return completed character count */
uint iobuf_written( Iobuf* buf );
/* Skip n characters */
int iobuf_skip( Iobuf* buf, uint n );
/* TODO */
int iobuf_read( Iobuf* buf, uchar* data, uint size );

#endif
