#include <types.h>
#include <err.h>
#include <config.h>
#include <trap_reason.h>
#include <syscall.h>
#include <user/syscall.h>

static void syscall_trap( Syscall* reason )
{
	asm volatile( "swi 0" );
}

static inline int syscall_make( uint reason, uint target_tid, void* data, uint datalen, void* buffer, uint bufferlen )
{
	Syscall call_reason = { 0 };

	/* Setup syscall parameters */
	call_reason.code = reason;
	call_reason.target_tid = target_tid;
	call_reason.data = data;
	call_reason.datalen = datalen;
	call_reason.buffer = buffer;
	call_reason.bufferlen = bufferlen;

	/* Trap into kernel */
	syscall_trap( &call_reason );

	return call_reason.result;
}

int Create( int priority, void (*code)() )
{
	/* TODO: code must be word-aligned non NULL pointer */
	return syscall_make( TRAP_CREATE, 0, code, priority, 0, 0 );
}

int Create_drv( int priority, void (*code)() )
{
	return syscall_make( TRAP_CREATE_DRV, 0, code, priority, 0, 0 );
}

int MyTid()
{
	return syscall_make( TRAP_MY_TID, 0, 0, 0, 0, 0 );
}

int MyParentTid()
{
	return syscall_make( TRAP_MY_PARENT_TID, 0, 0, 0, 0, 0 );
}

void Pass()
{
	syscall_make( TRAP_PASS, 0, 0, 0, 0, 0 );
}

void Exit()
{
	syscall_make( TRAP_EXIT, 0, 0, 0, 0, 0 );
}

int Send( int tid, char* msg, int msglen, char* reply, int replylen )
{
	return syscall_make( TRAP_SEND, tid, msg, msglen, reply, replylen );
}

int Receive( int* tid, char* msg, int msglen )
{
	return syscall_make( TRAP_RECEIVE, 0, tid, 0, msg, msglen );
}

int Reply( int tid, char* reply, int replylen )
{
	return syscall_make( TRAP_REPLY, tid, reply, replylen, 0, 0 );
}

int Exist( int tid )
{
	return syscall_make( TRAP_EXIST, tid, 0, 0, 0, 0 );
}

int KernelContext( void** target )
{
	return syscall_make( TRAP_KERNEL_CONTEXT, 0, 0, 0, (char*)target, 0 );
}

int AwaitEvent( int eventid )
{
	return syscall_make( TRAP_AWAIT_EVENT, eventid, 0, 0, 0, 0 );
}
