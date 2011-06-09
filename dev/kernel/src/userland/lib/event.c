#include <types.h>
#include <user/syscall.h>
#include <user/event.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <err.h>

#define EVENT_MAGIC     0xe11e1171

typedef struct Event_s Event_request;
typedef struct Event_s Event_reply;

struct Event_s {
	uint magic;
	uint type;
};

enum Event_type {
	EVENT_START_WAIT,
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

	reply.magic = EVENT_MAGIC;

	/* Init */
	status = Receive( &tid, ( char* )&request, sizeof( request ) );
	assert( status == sizeof( request ) );
	assert( request.magic == EVENT_MAGIC );

	callback = (Event_callback)(request.type);
	reply.type = request.type;

	status = Reply( tid, ( char* )&reply, sizeof( reply ) );
	assert( status == SYSCALL_SUCCESS );
	
	DEBUG_PRINT( DBG_EVENT, "Init done, c/b = 0x%x\n", callback );
	
	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		assert( status == sizeof( request ) );
		assert( request.magic == EVENT_MAGIC );

		reply.type = request.type;

		status = Reply( tid, ( char* )&reply, sizeof( reply ) );
		assert( status == SYSCALL_SUCCESS );

		if( request.type == EVENT_QUIT ){
			break;
		}

		DEBUG_NOTICE( DBG_EVENT, "received request\n" );
		
		AwaitEvent( EVENT_SRC_TC1UI );

		DEBUG_NOTICE( DBG_EVENT, "received interrupt\n" );
		
		status = callback( parent_tid );
		assert( status == ERR_NONE );
	}

	DEBUG_NOTICE( DBG_EVENT, "quit!\n" );
	Exit();
}

static int event_request( int tid, uint type )
{
	Event_request request;
	Event_reply reply;
	int status;

	request.magic = EVENT_MAGIC;
	request.type = type;

	status = Send( tid, ( char* )&request, sizeof( request ), ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );
	assert( reply.magic == EVENT_MAGIC );
	assert( reply.type == request.type );

	return ERR_NONE;
}

int event_init( int tid, Event_callback callback )
{
	return event_request( tid, ( uint )callback );
}

int event_start( int tid )
{
	return event_request( tid, EVENT_START_WAIT );
}

int event_quit( int tid )
{
	return event_request( tid, EVENT_QUIT );
}
