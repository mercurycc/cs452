#ifndef _USER_SYSCALL_H_
#define _USER_SYSCALL_H_

enum SyscallError {
	/* TODO: Implement all the error codes */
	/* Create */
	CREATE_INVALID_PRIORITY = -1,
	CREATE_OUT_OF_TASK_DESCRIPTOR = -2,
	/* MyParentTid */
	MY_PARENT_TID_BURIED = -1,          /* Parent is zombiefied or destroied */
	/* Pass cannot have recoverable failure */
	/* Exit cannot have recoverable failure */
	/* Send */
	SEND_INVALID_TASK_ID = -1,
	SEND_TASK_DOES_NOT_EXIST = -2,
	SEND_INCOMPLETE_TRANSACTION = -3,
	/* Receive cannot fail */
	/* Reply */
	REPLY_INVALID_TASK_ID = -1,
	REPLY_TASK_DOES_NOT_EXIST = -2,
	REPLY_TASK_IN_WRONG_STATE = -3,
	/* RegisterAs */
	REGISTER_AS_INVALID_NAMESERVER = -1,
	REGISTER_AS_FALSE_NAME_SERVER = -2
};

/* Task Creation */
int Create( int priority, void(*code)() );
int MyTid();
int MyParentTid();
void Pass();
void Exit();
/* Inter-task Communication */
int Send( int tid, char* msg, int msglen, char* reply, int replylen );
int Receive( int* tid, char* msg, int msglen );
int Reply( int tid, char* reply, int replylen );
/* Name Server */
int RegisterAs( char* name );
int WhoIs( char* name );
/* Interrupt Processing */
int AwaitEvent( int eventid );
/* or ( int eventid, char* event, int eventlen ) */
/* Clock Server */
int Delay( int ticks );
int Time();
int DelayUntil( int ticks );
/* Input/Output */
int Getc( int channel );
int Putc( int channel );

#endif /* _USER_SYSCALL_H_ */
