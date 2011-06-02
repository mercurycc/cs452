#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include <types.h>
#include <task.h>

struct Interrupt_mgr_s {
	Task** interrupt_handlers;
};

void interrupt_trap();
int interrupt_init( Context* ctx );
int interrupt_register( Context* ctx, Task* event_waiter, uint interrupt_id );
int interrupt_handle( Context* ctx, Task** event_delivered );

#endif /* _INTERRUPT_H_ */
