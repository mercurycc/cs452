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
	/* Name Server */
	TRAP_REGISTER_AS,
	TRAP_WHO_IS,
	/* Interrupt Processing */
	TRAP_AWAIT_EVENT,
	/* Clock Server */
	TRAP_DELAY,
	TRAP_TIME,
	TRAP_DELAY_UNTIL,
	/* Input/Output */
	TRAP_GETC,
	TRAP_PUTC
};

#endif /* _TRAP_REASON_H_ */
