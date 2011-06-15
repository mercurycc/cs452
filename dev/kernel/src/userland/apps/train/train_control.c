#include <types.h>
#include <err.h>
#include <user/apps_entry.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/train.h>
#include <user/uart.h>


#define MAX_BUFFER_SIZE 256

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
				cmd.command = N;
			}
			else if (( buf_i == 1 ) && ( buf[0] == 'q' )) {
				cmd.command = Q;
			}
			else {
				cmd.command = X;
				buf[buf_i] = '\n';
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
		case RV:
		case SW:
		case WH:
		case ST:
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
