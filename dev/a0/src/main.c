#include <types.h>
#include <config.h>
#include <err.h>
#include <clock.h>
#include <console.h>
#include <str.h>
#include <train.h>
#include <time.h>
#include <bwio.h>
#include <ui/ui.h>
#include <ui/prompt.h>
#include <ui/status.h>
#include <command.h>
#include <sensorctrl.h>

#define TRAIN 22

int main()
{
	uchar cmd[128] = {0};
	uint cmdsize = 0;
	uint execution = 1;
	Console termbody = {0};
	Console* term = &termbody;
	int status;

	/* Initialization */
	status = time_init();
	ASSERT( status == ERR_NONE );
	status = console_setup( term, CONSOLE_2, 115200, 0, 0, 0 );
	ASSERT( status == ERR_NONE );
	status = console_init( term );
	ASSERT( status == ERR_NONE );
	status = ui_init( term );
	ASSERT( status == ERR_NONE );

	ui_status_write( "Intializing..." );
	
	status = train_init();
	ASSERT( status == ERR_NONE );

	while( ! train_command_flushed() ){
		train_command_flush();
		ui_flush();
	}

	ui_status_write( "Completed." );

	/* For safetiness */
	TRAIN_COMMAND_GAP = 250;

	status = sensorctrl_init();
	ASSERT( status == ERR_NONE );

	while( execution ){
		status = ui_prompt_read( cmd, &cmdsize );
		if( status == ERR_NONE ){
			switch( *cmd ){
			case 'q':
			case 'Q':
				execution = 0;
				break;
			}
			/* In case input is '\r' */
			if( *cmd ){
				status = command_execute( cmd );
				if( status == ERR_COMMAND_NOT_SUPPORTED ) {
					ui_status_write( "Command not supported (persistent message)" );
				} else if( status == ERR_COMMAND_WRONG_PARAMETER ) {
					ui_status_write( "Incorrect parameter (persistent message)" );
				}
			}
		}

		status = ui_flush();
		ASSERT( status == ERR_NONE );

		sensorctrl_update();
		
		status = train_command_flush();
		ASSERT( status == ERR_NONE );
	}

	return 0;
}
