enum TimeRequest {
	TIME_ASK;
	TIME_DELAY;
	TIME_SIGNAL;
};

struct TimeMessage {
	TimeRequest request;
	int interval;
}

#include <user/name_server>


void time_main(){

	int tid;
	struct TimeMessage msg;
	int status;
	unsigned int result;
	int clock_tid;
	
	int stop = 0;
	
	clock_tid = Create( 0, clock_main );
	
	status = RegisterAs( "time" );
	assert( status == 0 );

	while ( !stop ) {
		Receive(&tid, msg, SIZE_TIME_MESSAGE);
		switch ( msg.request ) {
		case TIME_ASK:
			status = clock_current_time( clock_tid, (char*)&result );
			assert( status == 0 );
			status = Reply( tid, (char*)&result, sizeof(result));
			assert( status == sizeof(result) );
			break;
		case TIME_DELAY:
			status = clock_count_down( clock_tid, msg.interval );
			assert( status == 0 );
			break;
		case TIME_SIGNAL:
			// TODO find corresponding tid for the blocked task;
			result = 0;
			status = Replay( clock_tid, (char*)result, sizeof(result) );
			assert( status == 0 );
			break;
		default:
			bwprintf( COM2, "INVALID TIME MESSAGE" );
			break;
		}
	}

}

int time_request( int tid, TimeRequest request, int interval ){
	struct msg;
	int result;

	msg.request = request;
	msg.interval = interval;

	Send( tid, (char*)msg, sizeof(msg), (char*)&result, sizeof(result) );
	return result;
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



