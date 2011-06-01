#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include <types.h>

void interrupt_trap();
int interrupt_init( Context* ctx );
int interrupt_enable( Context* ctx, uint interrupt_id );
int interrupt_disable( Context* ctx, uint interrupt_id );

#endif /* _INTERRUPT_H_ */
