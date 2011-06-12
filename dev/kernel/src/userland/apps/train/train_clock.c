
// train task priority, subject to change in future
#define TRAIN_MODULE_PRIORITY 5
#define TRAIN_VIEW_PRIORITY 5


typedef struct Train_paint_event_s Train_paint_event

enum Paint_event_type {
	PAINT_ECHO,
	PAINT_ACK,
	PAINT_CLOCK,
	PAINT_SWITCH_LIST,
	PAINT_SENSOR_LIST,
	PAINT_SUICIDE
};


struct Train_paint_event_s {
	uint paint_type;
	int args[10];
};

void train_view() {
	int tid;
	int quit = 0;
	int state;
	Train_paint_event event;
	
	while ( !quit ) {
		state = Receive( &tid, (char*)&event, sizeof(event) );
		assert( state == sizeof( event ) );
		
		switch ( event.paint_type ) {
		case PAINT_ECHO:
		case PAINT_ACK:
		case PAINT_CLOCK:
		case PAINT_SWITCH_LIST:
		case PAINT_SENSOR_LIST:
		case PAINT_SUICIDE:
			// paint stuff
			break;
		default:
			assert(0);
		}
		
	}
	
	Exit();
}



