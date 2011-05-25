#ifndef _TRAP_REASON_H_
#define _TRAP_REASON_H_

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
	/* Interrupt Processing */
	TRAP_AWAIT_EVENT,
	/* Input/Output */
	TRAP_GETC,
	TRAP_PUTC
};

#endif /* _TRAP_REASON_H_ */
