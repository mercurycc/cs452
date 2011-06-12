
typedef struct Train_event_s Train_event;
typedef struct Train_reply_s Train_reply;

enum Train_event_type {
	TRAIN_UPDATE_TIME,
	TRAIN_SET_SPEED,
	TRAIN_REVERSE,
	TRAIN_SWITCH,
	TRAIN_LAST_SWITCH,
	TRAIN_LAST_SENSOR,
	TRAIN_ALL_SENSORS,
	TRAIN_MODULE_SUICIDE
};

struct Train_event_s {
	uint event_type;
	uint args[2];
};

struct Train_reply_s {
	int result;
};

void train_module() {

	int tid;
	int quit = 0;
	int status;
	Train_event event;
	Train_event reply;

	while ( !quit ) {
		status = Receive( &tid, (char*)&event, sizeof(event) );
		assert( status == sizeof(event) );
		
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
		case TRAIN_GET_ALL_SENSOR:
			// send and receive all sensor data
			break;
		case TRAIN_MODULE_SUICIDE:
			if ( tid == MyParentTid() ) {
				quit = 1;
				reply.result = 0;
				status = Reply( tid, (char*)reply, sizeof( reply ) );
				assert( status == 0 );
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
	
	// wait to sell sensor and clock task to exit
	int i = 0;
	for ( i = 0; i < 2; i++ ) {
		status = Receive( &tid, (char*)&event, sizeof(event) );
		assert( status == sizeof(event) );
		reply.result = -1;
		status = Reply( tid, (char*)reply, sizeof( reply ) );
		assert( status == 0 );
	}
	
	// tell anything produced by this to exit
	Exit();

}

int train_event( uint type, int arg0, int arg1 ) {
	Train_event event;
	Train_reply reply;

	event.event_type = type;
	event.arg[0] = arg0;
	event.arg[1] = arg1;

	int status = Send( module_id, (char*)&event, sizeof( event ), (char*)&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );
	
	return reply.result;
}


int train_update_time( int ticks ){
	return train_event( TRAIN_UPDATE_TIME, ticks, 0 );
}

int train_set_speed( int speed ){
	return train_event( TRAIN_SET_SPEED, speed, 0 );
}

int train_reverse(){
	return train_event( TRAIN_REVERSE, 0, 0 );
}

int train_switch( int switch_id, int direction ){
	return train_event( TRAIN_SWITCH, switch_id, direction );
}

int train_last_switch(){
	return train_event( TRAIN_LAST_SWITCH, 0, 0 );
}

int train_last_sensor(){
	return train_event( TRAIN_LAST_SENSOR, 0, 0 );
}

int train_all_sensor(){
	return train_event( TRAIN_ALL_SENSORS, 0, 0 );
}

int train_module_suicide(){
	return train_event( TRAIN_MODULE_SUICIDE, 0, 0 );
}
