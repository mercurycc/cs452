


void train_control() {
	
	int tid;
	int module_id;
	int view_id;
	int quit = 0;
	int state;
	
	module_id = Create( TRAIN_MODULE_PRIORITY, train_module );
	view_id = Create( TRAIN_VIEW_PRIORITY, train_view );
	
	
	while ( !quit ) {
		// await input
	}

	//tell children to suicide
	
	Exit();
}

