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

static int syscall_make( uint reason, void* data, uint datalen, void* buffer, uint bufferlen )
{
	Syscall call_reason = { 0 };

	call_reason.code = reason;
	call_reason.data = data;
	call_reason.datalen = datalen;
	call_reason.buffer = buffer;
	call_reason.bufferlen = bufferlen;

	syscall_trap( &call_reason );

	return call_reason.result;
}

int Create( int priority, void(*code)() )
{
	/* TODO: code must be word-aligned non NULL pointer */
	return syscall_make( TRAP_CREATE, code, priority, 0, 0 );
}

int MyTid()
{
	return syscall_make( TRAP_MY_TID, 0, 0, 0, 0 );
}

int MyParentTid()
{
	return syscall_make( TRAP_MY_PARENT_TID, 0, 0, 0, 0 );
}

void Pass()
{
	syscall_make( TRAP_PASS, 0, 0, 0, 0 );
}

void Exit()
{
	syscall_make( TRAP_EXIT, 0, 0, 0, 0 );
}
