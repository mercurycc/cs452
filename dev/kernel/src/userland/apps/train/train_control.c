#include <types.h>
#include <err.h>
#include <lib/str.h>
#include <user/apps_entry.h>
#include <user/assert.h>
#include <user/display.h>
#include <user/syscall.h>
#include <user/train.h>
#include <user/uart.h>
#include <user/lib/sync.h>

#define MAX_BUFFER_SIZE 128
#define MAX_SCREEN_SIZE 1024
#define PARSE_INT_FAIL -1

enum Command_type {
	TR,
	RV,
	SW,
	WH,
	ST,
	Q,
	PT,		// pressure test
	SA,		// switch all
	N,		// empty line
	X		// unrecognized input
};

struct Command {
	uint command;
	int args[2];
};


typedef struct Screen_s {
	char line[5][MAX_BUFFER_SIZE];
	int head;
	char nextline[MAX_BUFFER_SIZE];
} Screen;

int ack( Region* r, char* str, Screen* screen ) {
	// give the str to the print server
	int status = region_clear( r );
	assert( status == ERR_NONE );

	screen->head = (screen->head + 4) % 5;
	int head = screen->head;
	status = sprintf( screen->line[head], "%s: %s", screen->nextline, str );
	assert( status );

	status = region_printf( r, "%s\n%s\n%s\n%s\n%s\n > ", screen->line[(head+4)%5], screen->line[(head+3)%5], screen->line[(head+2)%5], screen->line[(head+1)%5], screen->line[head] );
	assert( status == ERR_NONE );
	return 0;
}

int ack_st( Region* r, int id, char state, Screen* screen ) {
	char str[64];
	int status = sprintf( str, "switch %d is in state %c", id, state );
	//assert( status < 64 );

	return ack( r, str, screen);
}

int ack_wh( Region* r, int id, Screen* screen ) {
	char str[64];
	int status = sprintf( str, "the last sensor triggered is %c%d", (id / 32 + 'A'), (id % 32));
	//assert( status < 64 );

	return ack( r, str, screen );
}


int echo( Region* r, char* str ) {
	// give the str to the print server
	int status = region_printf( r, " > %s ", str);
	assert( status == ERR_NONE );
	return 0;
}

int parse_int( char* str, int start, int end, int* stop ){
	// parse the first int from str[start] to str[end-1]
	if ( start >= end )
		return PARSE_INT_FAIL;
	
	if ( str[start] == '0' ){
		if (( start+1 == end )||( str[start+1] > '9' )||( str[start+1] < '0' )) {
			*stop = start+1;
			return 0;
		}
		else {
			return PARSE_INT_FAIL;
		}
	}
	
	int ret = 0;
	
	while ( start < end ) {
		if (( str[start] > '9' )||( str[start] < '0' )) {
			break;
		}
		
		ret = ret*10+str[start]-'0';
		start++;
	}
	
	*stop = start;
	return ret;
}

void train_control() {

	int module_id;
	int quit = 0;
	int status;
	char data;
	char buf[MAX_BUFFER_SIZE];
	Screen screen_data;
	Screen* screen = &screen_data;
	int buf_i = 0;
	int i;
	for ( i = 0; i < MAX_BUFFER_SIZE; i++ ){
		buf[i] = 0;
	}
	for ( i = 0; i < 5; i++ ){
		screen->line[i][0] = 0;
	}
	screen->head = 0;
	screen->nextline[0] = 0;
	
	Region ack_rect = {5, 15, 8, 70, 0, 1};
	Region *ack_region = &ack_rect;
	status = region_init( ack_region );
	assert( status == ERR_NONE );
	status = region_clear( ack_region );
	assert( status == ERR_NONE );	
	
	Region echo_rect = {6, 21, 1, 68, 0, 0};
	Region *echo_region = &echo_rect;
	status = region_init( echo_region );
	assert( status == ERR_NONE );
	status = region_clear( echo_region );
	assert( status == ERR_NONE );
	

	status = region_printf( echo_region, "Please wait for the track to initialize" );
	assert( status == ERR_NONE );

	module_id = Create( TRAIN_MODULE_PRIORITY, train_module );
	struct Command cmd;	

	sync_wait();
	status = region_clear( echo_region );
	assert( status == ERR_NONE );
	status = region_printf( echo_region, " > " );
	assert( status == ERR_NONE );
	

	while ( !quit ) {
		// await input
		data = Getc( COM_2 );

/*
		if ( !buf_i ) {
			status = region_clear( echo_region );
			assert( status == ERR_NONE );
			status = region_printf( echo_region, "" );
			assert( status == ERR_NONE );
		}
*/

		// parse input
		int start;
		int arg0;
		int arg1;
		
		switch (data) {
		case '\r':
			// trigger
			if ( buf_i == 0 ) {
				/* empty line */
				cmd.command = N;
			}
			else if ( buf_i == 1 ) {
				if ( buf[0] == 'q' ) {
					/* q: quit */
					cmd.command = Q;
				}
				else {
					cmd.command = X;
				}
			}
			else if ( buf_i == 2 ) {
				if (( buf[0] == 'w' )&&( buf[1] == 'h' )) {
					/* wh: the last sensor */
					cmd.command = WH;
				}
				else if (( buf[0] == 'p' )&&( buf[1] == 't' )) {
					/* PR: the last sensor */
					cmd.command = PT;
				}
				else {
					cmd.command = X;
				}
			}
			else if (( buf_i == 4 )&&( buf[0] == 's' )&&( buf[1] == 'a' )&&( buf[2] == 'l' )&&( buf[3] == 'l' )){
				cmd.command = SA;
				cmd.args[0] = 33;
			}
			else if (( buf_i == 4 )&&( buf[0] == 'c' )&&( buf[1] == 'a' )&&( buf[2] == 'l' )&&( buf[3] == 'l' )){
				cmd.command = SA;
				cmd.args[0] = 34;
			}
			else if (( buf[0] == 't' )&&( buf[1] == 'r' )) {
				/* TR: set train speed */
				arg0 = parse_int( buf, 3, buf_i, &start );
				arg1 = parse_int( buf, start+1, buf_i, &start );

				if (( arg0 == PARSE_INT_FAIL )||( arg1 == PARSE_INT_FAIL )||( start != buf_i )) {
					cmd.command = X;
				}
				else {
					cmd.command = TR;
					cmd.args[0] = arg0;
					cmd.args[1] = arg1;
				}
			}
			else if (( buf[0] == 'r' )&&( buf[1] == 'v' )){
				/* RV: rv train movement */
				arg0 = parse_int( buf, 3, buf_i, &start );

				if (( arg0 == PARSE_INT_FAIL )||( start != buf_i )) {
					cmd.command = X;
				}
				else {
					cmd.command = RV;
					cmd.args[0] = arg0;
				}
			}
			else if (( buf[0] == 's' )&&( buf[1] == 't' )){
				/* RV: rv train movement */
				arg0 = parse_int( buf, 3, buf_i, &start );

				if (( arg0 == PARSE_INT_FAIL )||( start != buf_i )) {
					cmd.command = X;
				}
				else {
					cmd.command = ST;
					cmd.args[0] = arg0;
				}
			}
			else if (( buf[0] == 's' )&&( buf[1] == 'w' )&&(( buf[buf_i-1] == 'S' )||( buf[buf_i-1] == 'C' ))){
				/* SW: shift switch */
				arg0 = parse_int( buf, 3, buf_i, &start );

				if (( arg0 == PARSE_INT_FAIL )||( start != buf_i-2 )) {
					cmd.command = X;
				}
				else {
					cmd.command = SW;
					cmd.args[0] = arg0;
					if ( buf[buf_i-1] == 'S' ){
						cmd.args[1] = 33;
					}
					else {
						cmd.args[1] = 34;
					}
				}
			}
			else {
				cmd.command = X;
			}
			echo( echo_region, " \n" );
			for ( i = 0; i < MAX_BUFFER_SIZE; i++ ){
				screen->nextline[i] = buf[i];
				if ( buf[i] == 0 ) break;
				buf[i] = 0;
			}
			buf_i = 0;
			break;
		case '\b':
			// undo
			if ( buf_i > 0 ){
				buf_i--;
				buf[buf_i] = 0;
				echo( echo_region, buf );
			}
			else {
				echo( echo_region, " \n" );
			}
			continue;
		default: 
			// echo
			buf[buf_i] = data;
			buf_i++;
			assert( buf_i < MAX_BUFFER_SIZE );
			echo( echo_region, buf );
			continue;
		}
		
		// do action
		switch ( cmd.command ) {
		case N:
			break;
		case Q:
			quit = 1;
			ack( ack_region, "Goodbye!", screen );
			break;
		case TR:
			ack( ack_region, "Train speed changes", screen );
			status = train_set_speed( cmd.args[0], cmd.args[1] );
			assert( status == ERR_NONE );
			break;
		case RV:
			ack( ack_region, "Train reverses", screen );
			status = train_reverse( cmd.args[0] );
			assert( status == ERR_NONE );
			break;
		case SW:
			ack( ack_region, "Switch shifts", screen );
			status = train_switch( cmd.args[0], cmd.args[1] );
			assert( status == ERR_NONE );
			break;
		case ST:
			status = train_check_switch( cmd.args[0] );
			ack_st( ack_region, cmd.args[0], (char)status, screen );
			break;
		case WH:
			status = train_last_sensor();
			if ( status ) {
				ack_wh( ack_region, status, screen );
			}
			break;
		case SA:
			ack( ack_region, "Shift all switches", screen );
			status = train_switch_all( cmd. args[0] );
			assert( status == ERR_NONE );
			break;
		case PT:
			ack( ack_region, "pressure test", screen );
			break;
		default:
			ack( ack_region, "Invalid command", screen );
			break;
		}
	}

	// tell children to suicide
	status = train_module_suicide();
	assert( status == 0 );

	sync_responde( MyParentTid() );

	DEBUG_NOTICE( DBG_USER, "quit!\n" );

	Exit();
}

