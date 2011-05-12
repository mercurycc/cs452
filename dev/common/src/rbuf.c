#include <types.h>
#include <err.h>
#include <rbuf.h>
#include <str.h>

static inline uchar* rbuf_get_element_address( Rbuf* buf, uint index )
{
	uchar* base = buf->buffer + buf->element_size * index;

	ASSERT( index < buf->count );
	
	return base;
}

int rbuf_init( Rbuf* buf, uchar* buffer, uint element_size, uint size )
{
	buf->buffer = buffer;
	buf->element_size = element_size;
	buf->count = size / element_size;
	buf->get = 0;
	buf->put = 0;

	ASSERT( buf->count );

	return ERR_NONE;
}

static inline uint rbuf_next_index( Rbuf* buf, uint index )
{
	return ( index + 1 ) % buf->count;
}

int rbuf_reset( Rbuf* buf )
{
	buf->get = 0;
	buf->put = 0;

	return ERR_NONE;
}

int rbuf_empty( Rbuf* buf )
{
	return buf->put == buf->get;
}

int rbuf_put( Rbuf* buf, const uchar* data )
{
	uchar* element_addr = 0;

	ENTERFUNC();

	if( rbuf_next_index( buf, buf->put ) != buf->get ){
		element_addr = rbuf_get_element_address( buf, buf->put );
		memcpy( element_addr, data, buf->element_size );
		buf->put = rbuf_next_index( buf, buf->put );
	} else {
		return ERR_RBUF_FULL;
	}

	return ERR_NONE;
}

int rbuf_get( Rbuf* buf, uchar* data )
{
	uchar* element_addr = 0;

	ENTERFUNC();
	
	if( buf->get != buf->put ){
		element_addr = rbuf_get_element_address( buf, buf->get );
		memcpy( data, element_addr, buf->element_size );
		buf->get = rbuf_next_index( buf, buf->get );
	} else {
		return ERR_RBUF_EMPTY;
	}

	return ERR_NONE;
}

