#include <bwio.h>
#include <err.h>
#include <types.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <user/name_server.h>
#include <user/time.h>
#include <user/syscall.h>
#include <devices/clock.h>
#include <user/devices/clock.h>
#include <user/assert.h>
#include <user/drivers_entry.h>
#include <user/servers_entry.h>
#include <user/lib/heap.h>
#include <user/clock_server.h>
#include <config.h>

typedef struct Time_request_s Time_request;
typedef struct Time_reply_s Time_reply;

enum Time_request_type {
	TIME_ASK,
	TIME_DELAY,
	TIME_DELAY_UNTIL,
	TIME_SIGNAL,
	TIME_SUICIDE
};

struct Time_request_s {
	unsigned int request;
	int interval;
};

struct Time_reply_s {
	int result;
};

struct Time_tid_node {
	uint time;
	int tid;
};


void time_main(){

	int tid;
	Time_request msg;
	Time_reply reply;
	int status;
	uint result = 0;
	int clock_tid;
	int cur_time;
	int stop = 0;
	Heap heap;
	struct Time_tid_node node;
	
	status = heap_init( &heap );
	assert( status == 0 );

	assert( MyTid() == CLOCK_SERVER_TID );
	
	clock_tid = Create_drv( SLOW_DRIVER_PRIORITY, clock_main );
	
	status = RegisterAs( "time" );
	assert( status == 0 );

	while ( !stop ) {
		status = Receive(&tid, (char*)&msg, sizeof(msg));
		assert( status == sizeof(msg) );
		
		DEBUG_PRINT( DBG_TIME, "received request 0x%x, interval %d\n", msg.request, msg.interval );

		if ( msg.request < TIME_SIGNAL ) {
			status = clock_current_time( clock_tid, &result );
			assert( status == 0 );
		}

		switch ( msg.request ) {
		case TIME_ASK:
			reply.result = result;
			status = Reply( tid, (char*)&reply, sizeof( reply ) );
			assert( status == 0 );
			break;
		case TIME_DELAY:
			if( msg.interval > 0 ){
				msg.interval += result;
			} else {
				msg.interval = 0;
			}
		case TIME_DELAY_UNTIL:
			if ( msg.interval <= result ) {
				reply.result = 0;
				status = Reply( tid, (char*)&reply, sizeof(reply) );
				assert ( status == 0 );
				break;
			}

			node.time = msg.interval;
			node.tid = tid;

			heap_insert( &heap, (Heap_node*)&node );

			DEBUG_PRINT( DBG_TIME, "delay until %d\n", msg.interval );

			status = clock_count_down( clock_tid, node.time );
			assert( status == 0 );
			break;
		case TIME_SIGNAL:
			DEBUG_NOTICE( DBG_TIME, "received signal\n" );
			
			// TODO rewrite structure
			reply.result = 0;
			status = Reply( tid, (char*)&reply, sizeof(reply) );
			assert( status == 0 );

			// TODO find corresponding tid for the blocked task;
			status = heap_read_top( &heap, (Heap_node*)&node );
			assert( status == 0 );
			cur_time = node.time;
			
			while ( node.time == cur_time ){
				status = Reply( node.tid, (char*)&reply, sizeof(reply) );
				DEBUG_PRINT( DBG_TIME, "status = 0x%x\n", status );
				assert( status == 0 );
				status = heap_remove_top( &heap, (Heap_node*)&node );
				assert( status == 0 );
				status = heap_read_top( &heap, (Heap_node*)&node );
				if ( status == ERR_HEAP_EMPTY ) {
					break;
				}
				assert( status == 0 );
				ASSERT_M( node.time >= cur_time, "expect %d, get %d\n", cur_time, node.time );
			}

			DEBUG_PRINT( DBG_TIME, "status = 0x%x\n", status );

			if ( status == 0 ) {
				DEBUG_PRINT( DBG_TIME, "sending new interrupt of interval %d\n", node.time );
				status = clock_count_down( clock_tid, node.time );
				assert( status == 0 );
			}
			break;
		case TIME_SUICIDE:
			if ( tid != MyParentTid() ) {
				bwprintf( COM2, "SUICIDE message from a task that is not my parent, fake message?\n" );
			}
			DEBUG_NOTICE( DBG_TIME, "quiting clock driver\n" );
			status = clock_quit( clock_tid );
			assert( status == 0 );
			DEBUG_NOTICE( DBG_TIME, "clock driver quit\n" );
			reply.result = 0;
			status = Reply( tid, (char*)&reply, sizeof(reply) );
			assert( status == 0 );
			stop = 1;
			break;
		default:
			bwprintf( COM2, "INVALID TIME MESSAGE" );
			break;
		}
	}
	DEBUG_NOTICE( DBG_USER, "time server suicided\n" );
	Exit();
}

static int time_request( int tid, uint request, int interval ){
	Time_request msg;
	Time_reply reply;

	msg.request = request;
	msg.interval = interval;

	int status = Send( tid, (char*)&msg, sizeof(msg), (char*)&reply, sizeof(reply) );
	if ( status == SEND_INVALID_TASK_ID ) return TIME_TID_INVALID;
	return reply.result;
}

int time_ask( int tid ) {
	return time_request( tid, TIME_ASK, 0 );
}

int time_delay( int tid, int interval ) {
	return time_request( tid, TIME_DELAY, interval );
}

int time_delay_until( int tid, int time ) {
	return time_request( tid, TIME_DELAY_UNTIL, time );
}

int time_signal( int tid ) {
	return time_request( tid, TIME_SIGNAL, 0 );
}

int time_suicide( int tid ) {
	return time_request( tid, TIME_SUICIDE, 0 );
}

int Time() {
	return time_ask( CLOCK_SERVER_TID );
}

int Delay( int ticks ) {
	return time_delay( CLOCK_SERVER_TID, ticks );
}

int DelayUntil( int ticks ) {
	return time_delay_until( CLOCK_SERVER_TID, ticks );
}

int time_tick_to_spec( Timespec* spec, int time )
{
	spec->fraction = time % 100;
	time /= 100;

	spec->second = time % 60;
	time /= 60;

	spec->minute = time % 60;
	time /= 60;

	spec->hour = time % 24;
	time /= 24;

	spec->day = time;

	return ERR_NONE;
}
