#include <user/syscall.h>
#include <user/lib/sync.h>
#include <user/protocals.h>
#include <user/assert.h>
#include <err.h>

int sync_wait()
{
	uint buf = 0;
	int tid = 0;
	int status = 0;

	status = Receive( &tid, ( char* )&buf, sizeof( buf ) );
	assert( status == sizeof( buf ) );
	DEBUG_PRINT( DBG_SYNC, "received: 0x%x\n", buf );
	assert( buf == SYNC_MAGIC );
	status = Reply( tid, ( char* )&buf, sizeof( buf ) );
	assert( status == SYSCALL_SUCCESS );

	return ERR_NONE;
}

int sync_responde( int tid )
{
	uint buf = SYNC_MAGIC;
	int status = 0;

	status = Send( tid, ( char* )&buf, sizeof( buf ), ( char* )&buf, sizeof( buf ) );
	assert( status == sizeof( buf ) );
	assert( buf == SYNC_MAGIC );

	return ERR_NONE;
}

