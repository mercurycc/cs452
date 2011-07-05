

int sensor_trustable( track_node* sensor ){
	return ( sensor->broken < SENSOR_NOT_TRUSTABLE );
}

int sensor_error( track_node* sensor ){
	sensor->broken += 1;
	return 0;
}

int sensor_trust( track_node* sensor ){
	if ( sensor->broken )
		sensor->broken -= 1;
	return 0;
}

int sensor_find_possible( track_node* sensor, int* switch_table, track_node** primary, track_node** secondary, track_node** tertiary ){
	*primary = 0;
	*secondary = 0;
	*tertiary = 0;

	track_node* temp = sensor->edge[DIR_AHEAD].dest;
	track_node* turnout = 0;
	int deadend = 0;

	while ( !(deadend || *secondary) ) {
		switch ( temp->type ) {
		case NODE_BRANCH:
			if ( !turnout ){
				turnout = temp;
			}

			if ( switch_table[SWID_TO_ARRAYID( temp->id + 1 )] == 'S' ) {
				temp = temp->edge[DIR_STRAIGHT].dest;
			} else {
				temp = temp->edge[DIR_CURVED].dest;
			}
			break;
		case NODE_MERGE:
			temp = temp->edge[DIR_AHEAD].dest;
			break;
		case NODE_SENSOR:
			if ( !(*primary) ) {
				*primary = temp;
			}
			else {
				*secondary = temp;
			}
			break;
		default:
			deadend = 1;
			break;
		}
	}

	if ( turnout ) {
		// find alternative
		if ( switch_table[SWID_TO_ARRAYID( temp->id + 1 )] == 'C' ) {
			temp = turnout->edge[DIR_STRAIGHT].dest;
		} else {
			temp = turnout->edge[DIR_CURVED].dest;
		}
		while ( temp->type != NODE_SENSOR ) {
			switch ( temp->type ){
			case NODE_BRANCH:
				if ( !turnout ){
					turnout = temp;
				}

				if ( switch_table[SWID_TO_ARRAYID( temp->id + 1 )] == 'S' ) {
					temp = temp->edge[DIR_STRAIGHT].dest;
				} else {
					temp = temp->edge[DIR_CURVED].dest;
				}
				break;
			case NODE_MERGE:
				temp = temp->edge[DIR_AHEAD].dest;
				break;
			default:
				return 0;
			}
		}
		tertiary = temp;
	}

	return 0;
}
