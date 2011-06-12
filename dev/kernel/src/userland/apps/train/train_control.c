
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
	int state;
	
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
			echo( "Invalid command, please check your " );
			break;
		}
	}

	//tell children to suicide
	event.event_type = TRAIN_MODULE_SUICIDE;
	status = Send( module_id, (char*)&event, sizeof( event ), (char*)&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );
	assert( reply.result == 0 );
	Exit();
}

