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

void time_main(){

	int tid;
	Time_request msg;
	Time_reply reply;
	int status;
	uint result = 0;
	int clock_tid;
	
	int stop = 0;
	
	clock_tid = Create( 0, clock_main );
	
	status = RegisterAs( "time" );
	assert( status == 0 );

	while ( !stop ) {
		status = Receive(&tid, (char*)&msg, sizeof(msg));
		assert( status == 0 );
		
		bwprintf ( COM2, "timer server received request 0x%x, interval %d\n", msg.request, msg.interval );
		
		switch ( msg.request ) {
		case TIME_ASK:
			status = clock_current_time( clock_tid, &result );
			assert( status == 0 );
			reply.result = result;
			status = Reply( tid, (char*)&reply, sizeof(reply));
			assert( status == sizeof(reply) );
			break;
		case TIME_DELAY:
			status = clock_count_down( clock_tid, msg.interval );
			assert( status == 0 );
			break;
		case TIME_SIGNAL:
			// TODO find corresponding tid for the blocked task;
			reply.result = 0;
			status = Replay( clock_tid, (char*)reply, sizeof(reply) );
			assert( status == sizeof(reply) );
			break;
		default:
			bwprintf( COM2, "INVALID TIME MESSAGE" );
			break;
		}
	}

}

int time_request( int tid, Time_request_type request, int interval ){
	Time_request msg;
	Time_reply reply;

	msg.request = request;
	msg.interval = interval;

	Send( tid, (char*)msg, sizeof(msg), (char*)&reply, sizeof(reply) );
	return reply.result;
}

int time_ask( int tid ){
	return time_request( tid, TIME_ASK, 0 );
}

int time_delay( int tid, int interval ){
	return time_request( tid, TIME_ASK, interval );
}

int time_signal( int tid ) {
	return time_request( tid, TIME_SIGNAL, 0 );
}



