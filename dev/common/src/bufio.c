#include <types.h>
#include <err.h>
#include <bufio.h>
#include <console.h>

int iobuf_init( Iobuf* container, Console* cons )
{
	container->cons = cons;
	container->buffer = 0;
	container->count = 0;
	container->remain = 0;

	return ERR_NONE;
}

int iobuf_flushed( Iobuf* buf )
{
	ASSERT( buf->remain >= 0 );
	return ( buf->remain == 0 );
}

int iobuf_flush( Iobuf* buf )
{
	int status = 0;

	if( buf->remain ){
		DEBUG_PRINT( DBG_BUFIO, "count = %u, remain = %u, cons = %d\n", buf->count, buf->remain, buf->cons );
		status = console_write( buf->cons, buf->buffer + buf->count, buf->remain );
		
		buf->count += buf->cons->last_write;
		buf->remain -= buf->cons->last_write;

		if( ! buf->remain ){
			DEBUG_NOTICE( DBG_BUFIO, "Completed\n" );
		}
	}
	       
	return ERR_NONE;
}

int iobuf_write( Iobuf* buf, uchar* data, uint size )
{
	buf->buffer = data;
	buf->count = 0;
	buf->remain = size;

	return ERR_NONE;
}

uint iobuf_written( Iobuf* buf )
{
	return buf->count;
}

int iobuf_skip( Iobuf* buf, uint n )
{
	buf->count += n;
	buf->remain -= n;

	ASSERT_M( buf->remain >= 0, "n = %d", n );

	return ERR_NONE;
}
