
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
	int status = uart_putc( UART2_DRV_TID, '\n' );
	assert( status == 0 );
	char* c = str;
	while ( *c ) {
		status = uart_putc( UART2_DRV_TID, *c );
		assert( status == 0 );
	}
	return 0;
}


void train_control() {
	
	int tid;
	int module_id;
	int quit = 0;
	int status;
	char data;
	char buf[MAX_BUFFER_SIZE];
	int buf_i = 0;
	
	Train_event event;
	Train_reply reply;
	
	module_id = Create( TRAIN_MODULE_PRIORITY, train_module );
	struct Command cmd;
	
	while ( !quit ) {
		// await input
		status = uart_getc( UART2_DRV_TID, &data );
		assert( status == ERR_NONE );

		// parse input
		switch (data) {
		case '\n':
			// trigger
			if ( buf_i == 0 ) {
				cmd.command = N;
			}
			else if (( buf_i == 1 ) && ( buf[0] == q )) {
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
				status = uart_putc( UART2_DRV_TID, data );
				assert( status == ERR_NONE );
			}
			continue;
		default: 
			// echo
			buf[buf_i] = data;
			buf_i++;
			assert( buf_i < MAX_BUFFER_SIZE );
			status = uart_putc( UART2_DRV_TID, data );
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

