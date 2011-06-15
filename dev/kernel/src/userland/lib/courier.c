#include <types.h>
#include <user/syscall.h>
#include <user/event.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <user/courier.h>
#include <err.h>
#include <config.h>

#define COURIER_MAGIC   0xc0091e87

typedef struct Courier_s Courier_request;
typedef struct Courier_s Courier_reply;

struct Courier_s {
#ifdef IPC_MAGIC
	uint magic;
#endif
	uint arg1;
	uint arg2;
};

enum Courier_type {
	COURIER_GO,
	COURIER_QUIT
};


void courier()
{
	Courier_callback callback;
	Courier_request request;
	Courier_reply reply;
	int tid;
	int status;

#ifdef IPC_MAGIC
	reply.magic = COURIER_MAGIC;
#endif

	/* Initialize callback */
	status = Receive( &tid, ( char* )&request, sizeof( request ) );
	assert( status == sizeof( request ) );
#ifdef IPC_MAGIC
	assert( request.magic == COURIER_MAGIC );
#endif
	
	status = Reply( tid, ( char* )&reply, sizeof( reply ) );
	assert( status == SYSCALL_SUCCESS );

	callback = ( Courier_callback )request.arg1;

	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		assert( status == sizeof( request ) );
#ifdef IPC_MAGIC
		assert( request.magic == COURIER_MAGIC );
#endif
		status = Reply( tid, ( char* )&reply, sizeof( reply ) );
		assert( status == SYSCALL_SUCCESS );

		if( ! request.arg1 ){
			break;
		}

		status = callback( request.arg1, request.arg2 );
		assert( status == ERR_NONE );
	}

	Exit();
}

static int courier_request( int courier_tid, uint arg1, uint arg2 )
{
	Courier_request request;
	Courier_reply reply;
	int status;

#ifdef IPC_MAGIC
	request.magic = COURIER_MAGIC;
#endif
	request.arg1 = arg1;
	request.arg2 = arg2;

	status = Send( courier_tid, ( char* )&request, sizeof( request ), ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );

#ifdef IPC_MAGIC
	assert( reply.magic == COURIER_MAGIC );
#endif

	return ERR_NONE;
}

int courier_init( int courier_tid, Courier_callback callback )
{
	return courier_request( courier_tid, ( uint )callback, 0 );
}

int courier_go( int courier_tid, int tid, uint arg )
{
	if( tid == 0 ){
		tid = 1;
	}
	return courier_request( courier_tid, ( uint )tid, arg );
}

int courier_quit( int courier_tid )
{
	return courier_request( courier_tid, 0, 0 );
}
