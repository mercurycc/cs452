#include <types.h>
#include <err.h>
#include <user/apps_entry.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/train.h>
#include <user/uart.h>


#define MAX_BUFFER_SIZE 256
#define PARSE_INT_FAIL -1

enum Command_type {
	TR,
	RV,
	SW,
	WH,
	ST,
	Q,
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
		
	module_id = Create( TRAIN_MODULE_PRIORITY, train_module );
	struct Command cmd;
	
	while ( !quit ) {
		// await input
		data = Getc( COM_2 );

		// parse input
		switch (data) {
		case '\r':
			// trigger
			if ( buf_i == 0 ) {
				/* empty line */
				cmd.command = N;
			}
			else if (( buf_i == 1 ) && ( buf[0] == 'q' )) {
				/* quit */
				cmd.command = Q;
			}
			else if ( buf_i == 2 ) {
				if (( buf[0] == 'S' )&&( buf[1] == 'T' )) {
					cmd.command = ST;
				}
				else if (( buf[0] == 'W' )&&( buf[1] == 'H' )) {
					cmd.command = WH;
				}
				else {
					cmd.command = X;
				}
			}
			else {
				cmd.command = X;
			}
			status = Putc( COM_2, '\n' );
			assert( status == ERR_NONE );
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
		case ST:
			echo("LAST SWITCH");
			break;
		case WH:
			echo("LAST SENSOR");
			break;
		case TR:
		case SW:
		case RV:
		default:
			echo( "Invalid command" );
			break;
		}
	}

	//tell children to suicide
	status = train_module_suicide();
	assert( status == 0 );

	Exit();
}
