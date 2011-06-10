

typedef struct Train_event_s Train_event;

enum Train_event_type {
	TRAIN_UPDATE_TIME,
	TRAIN_SET_SPEED,
	TRAIN_REVERSE,
	TRAIN_SWITCH,
	TRAIN_DISPLAY_LAST_SWITCH,
	TRAIN_DISPLAY_LAST_SENSOR,
	TRAIN_MODULE_SUICIDE
};


struct Train_event_s {
	uint event_type;
	uint args[2];
};



void train_module() {

	int tid;
	int quit = 0;
	int state;
	Train_event event;

	while ( !quit ) {
		state = Receive( &tid, (char*)&event, sizeof(event) );
		assert( state == sizeof(event) );
		
		switch (event.event_type) {
		case TRAIN_UPDATE_TIME:
			break;
		case TRAIN_SET_SPEED:
			// send to train control uart
			break;
		case TRAIN_REVERSE:
			// send to train control uart
			break;
		case TRAIN_SWITCH:
			// send to train control uart
			break;
		case TRAIN_DISPLAY_LAST_SWITCH:
			// send to train control uart
			break;
		case TRAIN_DISPLAY_LAST_SENSOR:
			// send to SENSOR uart
			break;
		case TRAIN_MODULE_SUICIDE:
			if ( tid == MyParentTid() ) {
				quit = 1;
				break;
			}
			// warning message
			break;
		default:
			// should not get to here
			// TODO: change to uart
			assert(0);
		}
	}
	
	// tell anything produced by this to exit
	Exit();

}
