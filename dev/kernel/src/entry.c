/* Pinball entry point */

#include <types.h>
#include <err.h>

/* Kernel */
/* Hold kernel manipulation routines */
#include <kernel.h>

/* Devices */
/* Device init, function, shutdown */
#include <devices/clock.h>
#include <devices/console.h>

/* Trap handler */
/* Trap handler installation */
#include <trap.h>

/* Interrupt */
/* Interrupt manipulation */
// #include <interrupt.h>

/* Task */
/* Task abstraction and scheduling support */
#include <task.h>
#include <sched.h>

/* Context */
/* Kernel context, substitution of global variables */
/* Not sure if file scope variables are safe.  If not they can be
   moved into the kernel context */
#include <context.h>

/* Memory management */
/* Kernel memory allocation, allocation and deallocation of kernel
   pages, or allocate on kernel heap */
/* Kernel heap is not deallocatable.  We need to give enough memory to
   the kernel heap for all sub components to work properly. */
/* mem should also allocate stack for user apps */
#include <mem.h>

/* User space */
#include <session.h>

void dosyscall( uint reason )
{
	asm volatile( "swi 0" );
}

void doyield()
{
	dosyscall( 0xdeadbeef );
}

void doexit()
{
	dosyscall( 0xcafebabe );
}

void dokern()
{
	dosyscall( 0xcafe );
}

int userland1()
{
	DEBUG_NOTICE( DBG_KER, "entered\n" );
	doyield();
	
	while( 1 ){
		DEBUG_NOTICE( DBG_KER, "re-entered\n" );
		doexit();
	}
	return 0;
}

int userland2()
{
	DEBUG_NOTICE( DBG_KER, "entered\n" );

	while( 1 ){
		doexit();
		DEBUG_NOTICE( DBG_KER, "re-entered\n" );
	}
	return 0;
}

int main()
{
	Context ctxbody = {0};
	Context* ctx = &ctxbody;
	Console termbody = {0};
	uint currentsp = 0;

	ctxbody.terminal = &termbody;
	
	/* Init Kernel */
	kernel_init( ctx );

	DEBUG_PRINT( DBG_KER, "Starting user session, sp 0x%x, org sp 0x%x\n", (&currentsp) - 4096, &currentsp );
	/* Start user session */
	session_start( userland1, (&currentsp) - 4096 );

	return 0;
}
