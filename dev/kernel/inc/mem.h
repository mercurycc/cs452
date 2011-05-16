/* Simple memory allocator/deallocator.  At the moment all memory
   (except stack, which is simply grabbed from free memory) needed
   must be pre-allocated on the kernel stack and passed in through
   mem_init. */
#ifndef _MEM_H_
#define _MEM_H_

#include <types.h>
#include <err.h>
#include <config.h>
#include <lib/rbuf.h>
#include <context.h>

enum Memmgr_mem_type {
	MEM_TASK,
	MEM_STACK
};

struct Memmgr_s {
	Rbuf* task;
	Rbuf* stack;
};

/* mem_init

   Initialize one of the rbufs inside the Memmgr with a buffer of size ( size * count ).
   
   buffer:    The actual pre-allocated memory block
   
   container: rbuf requires a container to contain all pre-allocated
              pointers.  The container size must be at least sizeof(
              void* ) x ( count + 1 )
*/
int mem_init( Context* ctx, uint type, Rbuf* ring, uchar* container, uchar* buffer, uint size, uint count );

/* mem_alloc

   Return count pointers from the rbuf inside the Memmgr */
int mem_alloc( Context* ctx, uint type, void** ret, uint count );

/* mem_free

   Put count pointers back to the rbuf inside the Memmgr */
int mem_free( Context* ctx, uint type, void** ptrs, uint count );

#endif /* _MEM_H_ */
