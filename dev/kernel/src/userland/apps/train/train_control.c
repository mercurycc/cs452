#include <types.h>
#include <err.h>
#include <user/apps_entry.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/train.h>
#include <user/uart.h>
#include <user/lib/sync.h>

#define MAX_BUFFER_SIZE 256
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

int echo( char* str ) {
	// give the str to the print server
	// test version
	int status = Putc( COM_2, '\n' );
	assert( status == 0 );
	char* c = str;
	while ( *c ) {
		status = Putc( COM_2, *c );
		assert( status == 0 );
		c++;
	}
	status = Putc( COM_2, '\n' );
	assert( status == 0 );
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
	int buf_i = 0;
	int i;
	for ( i = 0; i < MAX_BUFFER_SIZE; i++ ){
		buf[i] = 0;
	}

	module_id = Create( TRAIN_MODULE_PRIORITY, train_module );
	struct Command cmd;
	
	while ( !quit ) {
		// await input
		data = Getc( COM_2 );

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
			status = Putc( COM_2, '\n' );
			assert( status == ERR_NONE );
			for ( i = 0; i < MAX_BUFFER_SIZE; i++ ){
				buf[i] = 0;
			}
			buf_i = 0;
			break;
		case '\b':
			// undo
			if ( buf_i > 0 ){
				buf_i--;
				status = Putc( COM_2, data );
				assert( status == ERR_NONE );
			}
			continue;
		default: 
			// echo
			buf[buf_i] = data;
			buf_i++;
			assert( buf_i < MAX_BUFFER_SIZE );
			status = Putc( COM_2, data );
			assert( status == ERR_NONE );
			continue;
		}
		
		// do action
		switch ( cmd.command ) {
		case N:
			echo( "" );
			break;
		case Q:
			quit = 1;
			echo( "Goodbye!" );
			break;
		case TR:
			echo( "Train speed changes" );
			status = train_set_speed( cmd.args[0], cmd.args[1] );
			assert( status == ERR_NONE );
			break;
		case RV:
			echo( "Train reverses" );
			status = train_reverse( cmd.args[0] );
			assert( status == ERR_NONE );
			break;
		case SW:
			echo( "Switch shifts" );
			status = train_switch( cmd.args[0], cmd.args[1] );
			assert( status == ERR_NONE );
			break;
		case ST:
			echo( "Switch state" );
			status = train_check_switch( cmd.args[0] );
			assert( status == ERR_NONE );
			break;
		case WH:
			echo( "LAST SENSOR" );
			break;
		case SA:
			echo( "Shift all switches" );
			status = train_switch_all( cmd. args[0] );
			assert( status == ERR_NONE );
			break;
		case PT:
			echo( "pressure test" );
			break;
		default:
			echo( "Invalid command" );
			break;
		}
	}

	//tell children to suicide
	status = train_module_suicide();
	assert( status == 0 );

	echo("so long");
	sync_responde( MyParentTid() );
	Exit();
}
