
enum Command_type {
	TR,
	RV,
	SW,
	WH,
	ST,
	Q,
	X
};

struct Command {
	uint command;
	int args[2];
};

int echo( char* str ) {
	// give the str to the print server
	return 0;
}


void train_control() {
	
	int tid;
	int module_id;
	int quit = 0;
	int status;
	
	Train_event event;
	Train_reply reply;
	
	module_id = Create( TRAIN_MODULE_PRIORITY, train_module );
	struct Command cmd;
	
	while ( !quit ) {
		// await input
		
		// parse input
		
		// check number of arguments
		
		// do action
		switch ( cmd.command ) {
		case TR:
		case RV:
		case SW:
		case WH:
		case ST:
		case Q:
			quit = 1;
			echo( "Goodbye!" );
			break;
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

