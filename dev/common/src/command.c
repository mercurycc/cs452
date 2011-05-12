#include <types.h>
#include <err.h>
#include <config.h>
#include <command.h>
#include <train.h>
#include <str.h>
#include <ui/switches.h>
#include <ui/status.h>
#include <sensorctrl.h>

/* TODO: Create a table mapping command to function calls */

static inline uchar* command_tokenize( uchar* cmd, uchar* buffer, uint size )
{
	/* Skip all the delimiter characters */
	while( *cmd && *cmd == (uchar)COMMAND_DELIMITER ){
		cmd += 1;
	}
	
	while( *cmd && *cmd != COMMAND_DELIMITER ){
		*(buffer++) = *(cmd++);
		size -= 1;
		ASSERT( size );
	}

	*buffer = '\0';
	
	return cmd;
}

int command_init()
{
	return ERR_NONE;
}

int command_execute( uchar* cmd )
{
	char cmdToIssue[ COMMAND_TOKEN_SIZE ] = { 0 };
	char params[ COMMAND_MAX_PARAM_COUNT ][ COMMAND_TOKEN_SIZE ] = { { 0 } };
	uint i = 0;
	int status = 0;

	cmd = command_tokenize( cmd, (uchar*)cmdToIssue, COMMAND_TOKEN_SIZE );
	ASSERT( status == ERR_NONE );
	ASSERT( *cmdToIssue );

	DEBUG_PRINT( DBG_CMD, "Get command %s\n", cmdToIssue );

	for( i = 0; i < COMMAND_MAX_PARAM_COUNT; i += 1 ){
		cmd = command_tokenize( cmd, (uchar*)params[ i ], COMMAND_TOKEN_SIZE );
		if( params[ i ][ 0 ] == '\0' ) break;
		DEBUG_PRINT( DBG_CMD, "Get param %s\n", params[ i ] );
	}

	DEBUG_PRINT( DBG_CMD, "PARAM count %u\n", i );

	/* TODO: Is there a better way to parse the commands? */
	if       ( ! strcmp( cmdToIssue, "tr" ) ){
		DEBUG_NOTICE( DBG_CMD, "tr captured\n" );
		if( i != 2 ){
			goto wrongparam;
		}
		uint train = stou( params[ 0 ] );
		uint speed = stou( params[ 1 ] );
		if( train < 1 || train > 80 ){
			goto wrongparam;
		}

		status = train_set_speed( train, speed );
		ASSERT( status == ERR_NONE );
	} else if( ! strcmp( cmdToIssue, "rv" ) ){
		DEBUG_NOTICE( DBG_CMD, "rv captured\n" );
		if( i != 1 ){
			goto wrongparam;
		}
		uint train = stou( params[ 0 ] );
		if( train < 1 || train > 80 ){
			goto wrongparam;
		}
		
		status = train_reverse_direction( train );
		ASSERT( status == ERR_NONE );
	} else if( ! strcmp( cmdToIssue, "sw" ) ){
		DEBUG_NOTICE( DBG_CMD, "sw captured\n" );
		if( i != 2 || params[ 1 ][ 1 ] != '\0' ){
			goto wrongparam;
		}
		uint swi = stou( params[ 0 ] );
		if( i < 1 || ( i > 18 && i < 152 ) || i > 156 ){
			goto wrongparam;
		}
		switch( params[ 1 ][ 0 ] ){
		case 'c':
		case 'C':
			status = train_switch_curve( swi );
		ASSERT( status == ERR_NONE );
		status = ui_switch_update( train_switch_swi_to_id( swi ), SWITCH_CURVE );
		break;
		case 's':
		case 'S':
			status = train_switch_straight( swi );
		ASSERT( status == ERR_NONE );
		status = ui_switch_update( train_switch_swi_to_id( swi ), SWITCH_STRAIGHT );
		break;
		}
		ASSERT( status == ERR_NONE );
	} else if( ! strcmp( cmdToIssue, "st" ) ){
		DEBUG_NOTICE( DBG_CMD, "st captured\n" );
		uchar state[] = "NNN";
		uint swi;
		if( i != 0 ){
			goto wrongparam;
		}
		train_switch_recent_switch( &swi );
		utos( swi, (char*)state );

		ui_status_write( (char*)state );
	} else if( ! strcmp( cmdToIssue, "wh" ) ){
		DEBUG_NOTICE( DBG_CMD, "wh captured\n" );
		uchar state[] = "ANN";
		uint group;
		uint id;
		if( i != 0 ){
			goto wrongparam;
		}
		sensorctrl_most_recent( &group, &id );
		state[ 0 ] = group + 'A';
		utos_fill( id + 1, (char*)(state + 1), 2, ' ' );

		ui_status_write( (char*)state );
	} else {
		return ERR_COMMAND_NOT_SUPPORTED;
	}

	return ERR_NONE;

 wrongparam:
	return ERR_COMMAND_WRONG_PARAMETER;
}
