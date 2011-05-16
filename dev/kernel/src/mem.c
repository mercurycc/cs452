#include <types.h>
#include <err.h>
#include <config.h>
#include <lib/rbuf.h>
#include <context.h>
#include <mem.h>

static inline Rbuf* mem_get_rbuf( Memmgr* mem, uint type )
{
	Rbuf* ret = 0;
	
	switch( type ){
	case MEM_TASK:
		ret = mem->task;
		break;
	case MEM_STACK:
		ret = mem->stack;
		break;
	}

	return ret;
}

int mem_init( Context* ctx, uint type, Rbuf* ring, uchar* container, uchar* buffer, uint size, uint count )
{
	Memmgr* mem = ctx->mem;
	int status = 0;

	ASSERT( buffer && container );

	status = rbuf_init( ring, container, sizeof( void* ), sizeof( void* ) * ( count + 1 ) );
	ASSERT( status == ERR_NONE );

	for( ; count; count -= 1 ){
		status = rbuf_put( ring, &buffer );
		buffer += size;
		ASSERT_M( status == ERR_NONE, "status = %d", status );
	}

	switch( type ){
	case MEM_TASK:
		mem->task = ring;
		break;
	case MEM_STACK:
		mem->stack = ring;
		break;
	}

	return ERR_NONE;
}

int mem_alloc( Context* ctx, uint type, void** ret, uint count )
{
	Rbuf* pool = mem_get_rbuf( ctx->mem, type );
	int status = 0;

	for( ; count; count -= 1, ret += 1 ){
		status = rbuf_get( pool, ( uchar* )ret );
		if( status != ERR_NONE ){
			break;
		}
	}

	return status;
}

int mem_free( Context* ctx, uint type, void** ptrs, uint count )
{
	Rbuf* pool = mem_get_rbuf( ctx->mem, type );
	int status = 0;

	for( ; count; count -= 1, ptrs += 1 ){
		status = rbuf_put( pool, ( uchar* )ptrs );
		if( status != ERR_NONE ){
			break;
		}
	}

	return status;
}

