
void train_clock() {
	int tid;
	int quit = 0;
	int status;
	uint ticks = 0;

	while ( !quit ) {
		ticks += 100 / CLOCK_COUNT_DOWN_MS_PER_TICK;
		status = DelayUntil(ticks);
		assert( status == 0 );

		status = train_update_time( ticks );
		if ( status == -1 ) {
			quit = 1;
		}
		else {
			assert( status == 0 );
		}
	}
	
	Exit();
}



