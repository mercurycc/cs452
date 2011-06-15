#include <types.h>
#include <user/syscall.h>
#include <user/event.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <err.h>
#include <config.h>

#define EVENT_MAGIC     0xe11e1171

typedef struct Event_s Event_request;
typedef struct Event_s Event_reply;

struct Event_s {
#ifdef IPC_MAGIC
	uint magic;
#endif
	uint type;
};

enum Event_type {
	EVENT_START_WAIT,
	EVENT_ALWAYS,
	EVENT_QUIT
};

void event_handler()
{
	Event_request request;
	Event_reply reply;
	int tid = 0;
	int parent_tid = MyParentTid();
	int status = 0;
	Event_callback callback;
	uint event;
	uint always = 0;

#ifdef IPC_MAGIC
	reply.magic = EVENT_MAGIC;
#endif

	/* Init callback */
	status = Receive( &tid, ( char* )&request, sizeof( request ) );
	assert( status == sizeof( request ) );
#ifdef IPC_MAGIC
	assert( request.magic == EVENT_MAGIC );
#endif

	callback = (Event_callback)(request.type);
	reply.type = request.type;

	status = Reply( tid, ( char* )&reply, sizeof( reply ) );
	assert( status == SYSCALL_SUCCESS );

	/* Init event id */
	status = Receive( &tid, ( char* )&request, sizeof( request ) );
	assert( status == sizeof( request ) );
#ifdef IPC_MAGIC
	assert( request.magic == EVENT_MAGIC );
#endif

	event = request.type;
	reply.type = request.type;

	status = Reply( tid, ( char* )&reply, sizeof( reply ) );
	assert( status == SYSCALL_SUCCESS );

	
	DEBUG_PRINT( DBG_EVENT, "Init done, c/b = 0x%x\n", callback );
	
	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		assert( status == sizeof( request ) );
#ifdef IPC_MAGIC
		assert( request.magic == EVENT_MAGIC );
#endif

		reply.type = request.type;

		status = Reply( tid, ( char* )&reply, sizeof( reply ) );
		assert( status == SYSCALL_SUCCESS );

		if( request.type == EVENT_QUIT ){
			break;
		} else if( request.type == EVENT_ALWAYS ){
			always = 1;
		}
		
		DEBUG_NOTICE( DBG_EVENT, "received request\n" );
	
		do {
			AwaitEvent( event );
			DEBUG_NOTICE( DBG_EVENT, "received interrupt\n" );
			status = callback( parent_tid );
			assert( status == ERR_NONE );
		} while( always );
	}

	DEBUG_NOTICE( DBG_EVENT, "quit!\n" );
	Exit();
}

static int event_request( int tid, uint type )
{
	Event_request request;
	Event_reply reply;
	int status;

#ifdef IPC_MAGIC
	request.magic = EVENT_MAGIC;
#endif
	request.type = type;

	status = Send( tid, ( char* )&request, sizeof( request ), ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );
#ifdef IPC_MAGIC
	assert( reply.magic == EVENT_MAGIC );
#endif
	assert( reply.type == request.type );

	return ERR_NONE;
}

int event_init( int tid, uint event, Event_callback callback )
{
	int status = 0;
	status = event_request( tid, ( uint )callback );
	assert( status == ERR_NONE );

	return event_request( tid, event );
}

int event_start( int tid )
{
	return event_request( tid, EVENT_START_WAIT );
}

int event_always( int tid )
{
	return event_request( tid, EVENT_ALWAYS );
}

int event_quit( int tid )
{
	return event_request( tid, EVENT_QUIT );
}
