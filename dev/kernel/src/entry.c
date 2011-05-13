/* Pinball entry point */

#if 0
/* Kernel */
/* Hold kernel manipulation routines */
#include <kernel.h>

/* Devices */
/* Device init, function, shutdown */
#include <devices/clock.h>
#include <devices/uart.h>

/* Trap handler */
/* Trap handler installation */
#include <trap.h>

/* Interrupt */
/* Interrupt manipulation */
#include <interrupt.h>

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

#endif /* if 0 */

int main()
{
	Context ctxbody = {0};
	Context* ctx = &ctxbody;

	/* Init Kernel */
	kernel_init();
	
	/* Start user session */
	session_init( ctx );
	session_start( ctx );

	/* Session ends, shutdown */
	kernel_shutdown( ctx );
	
	return 0;
}
