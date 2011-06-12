
void train_sensor() {
	int tid;
	int quit = 0;
	int status;

	while ( !quit ) {
		
		status = train_all_sensor();
		if ( status == -1 ) {
			quit = 1;
		}
		else {
			assert( status == 0 );
		}
	}
	
	Exit();
}



