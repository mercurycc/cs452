/* Pinball entry point */

/* Kernel */
/* Hold kernel manipulation routines */
#include <kernel.h>

/* Devices */
/* Device init, function, shutdown */
#include <arch/clock.h>
#include <arch/uart.h>

/* Trap handler */
/* Trap handler installation */
#include <arch/trap.h>

/* Interrupt */
/* Interrupt manipulation */
#include <arch/interrupt.h>

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
#include <mem.h>

/* User space */
#include <session.h>

int main()
{
	Context ctxbody = {0};
	Context* ctx = &ctxbody;


	/* Start user session */
	session_init( ctx );
	session_start( ctx );

	/* Session ends, shutdown */
	kernel_shutdown( ctx );
	
	return 0;
}
