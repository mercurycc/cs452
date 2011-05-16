#ifndef _TRAP_H_
#define _TRAP_H_

#include <types.h>
#include <err.h>
#include <context.h>

enum TrapReason {
	/* Task Creation */
	TRAP_CREATE,
	TRAP_MY_TID,
	TRAP_MY_PARENT_TID,
	TRAP_PASS,
	TRAP_EXIT,
	TRAP_DESTROY,
	/* Inter-task Communication */
	TRAP_SEND,
	TRAP_RECEIVE,
	TRAP_REPLY,
	/* Name Server */
	TRAP_REGISTER_AS,
	TRAP_WHO_IS,
	/* Interrupt Processing */
	TRAP_AWAIT_EVENT,
	/* Clock Server */
	TRAP_DELAY,
	TRAP_TIME,
	TRAP_DELAY_UNTIL,
	/* Input/Outpu */
	TRAP_GETC,
	TRAP_PUTC
};

/* Install trap_handler_begin */
int trap_init( Context* ctx );

/* Userland program will call into trap_handler_begin with swi */
void trap_handler_begin( uint reason, uint sp_usr );

/* Trap handler body */
void trap_handler( uint reason, uint sp_caller, uint mode, uint* kernelsp );

/* Trap hander return to userland through this */
void trap_handler_end( uint reason, uint sp_usr, uint mode, uint* kernelsp );

#endif /* _TRAP_H_ */
