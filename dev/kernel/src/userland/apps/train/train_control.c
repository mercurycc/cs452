#include <types.h>
#include <err.h>
#include <lib/str.h>
#include <user/apps_entry.h>
#include <user/assert.h>
#include <user/display.h>
#include <user/syscall.h>
#include <user/uart.h>
#include <user/lib/sync.h>
#include <user/lib/parser.h>
#include "inc/train.h"
#include "inc/config.h"
#include "inc/sensor_data.h"

#define MAX_BUFFER_SIZE 128
#define MAX_SCREEN_SIZE 1024
#define PARSE_INT_FAIL -1

typedef struct Screen_s {
	char line[5][MAX_BUFFER_SIZE];
	int head;
	char nextline[MAX_BUFFER_SIZE];
} Screen;

enum Command_status {
	CMD_SUCCESS,
	CMD_UNKNOWN,
	CMD_ERROR,
	CMD_HELP
};

int ack( Region* r, char* str, Screen* screen )
{
	// give the str to the print server
	int status;
	// = region_clear( r );
	// assert( status == ERR_NONE );

	screen->head = (screen->head + 4) % 5;
	int head = screen->head;
	status = sprintf( screen->line[head], "%s: %s", screen->nextline, str );
	assert( status );

	status = region_printf( r, "%s\n%s\n%s\n%s\n%s\n > \n", screen->line[(head+4)%5], screen->line[(head+3)%5], screen->line[(head+2)%5], screen->line[(head+1)%5], screen->line[head] );
	assert( status == ERR_NONE );
	return 0;
}

int ack_st( Region* r, int id, char state, Screen* screen )
{
	char str[64];

	sprintf( str, "switch %d is in state %c", id, state );

	return ack( r, str, screen);
}

int ack_wh( Region* r, int id, Screen* screen )
{
	char str[64];
	sprintf( str, "the last sensor triggered is %c%d", (id / 32 + 'A'), (id % 32));
	//assert( status < 64 );

	return ack( r, str, screen );
}

int echo( Region* r, char* str ) {
	// give the str to the print server
	int status = region_printf( r, " > %s ", str);
	assert( status == ERR_NONE );
	return 0;
}

/* Train control holds the prompt UI */
void train_control()
{
	int clock_tid;
	int sensor_tid;
	int sensor_ui_tid;
	int switch_tid;
	int module_tid;
	int quit = 0;
	char data;
	char buf[MAX_BUFFER_SIZE] = {0};
	char* token_buf[ TRAIN_COMMAND_MAX_TOKEN ] = {0};
	int buf_i = 0;
	int token_filled;
	int command_status;
	int i;
	Sensor_data sensor_data;
	/* Prompt UI */
	Region prompt_titles = { 2, 20, 1, 11 - 2, 1, 0 };
	Region prompt_reg = { 2, 21, 1, 78 - 2, 1, 0 };
	Region result_reg = { 14, 22, 1, 78 - 14, 1, 0 };
	int prompt_width = prompt_reg.width - prompt_reg.margin - prompt_reg.margin;
	
	int status;

	status = region_init( &prompt_reg );
	assert( status == ERR_NONE );
	status = region_init( &result_reg );
	assert( status == ERR_NONE );

	/* Print title */
	/* Console */
	status = region_init( &prompt_titles );
	assert( status == ERR_NONE );
	region_printf( &prompt_titles, "Console" );
	/* Result */
	prompt_titles.row = 22;
	status = region_init( &prompt_titles );
	assert( status == ERR_NONE );
	region_printf( &prompt_titles, "Result" );
	
	status = region_printf( &result_reg, "Please wait for the track to initialize\n" );
	assert( status == ERR_NONE );

	module_tid = Create( TRAIN_MODULE_PRIORITY, train_module );
	assert( module_tid > 0 );

	sync_wait();

	region_clear( &result_reg );

	// switch_tid = Create( TRAIN_SWITCH_PRIORITY, train_switches );
	// assert( switch_tid > 0 );

	clock_tid = Create( TRAIN_UI_PRIORITY, clock_ui );
	assert( clock_tid > 0 );

	sensor_ui_tid = Create( TRAIN_UI_PRIORITY, sensor_ui );
	assert( sensor_ui_tid > 0 );

	sensor_tid = Create( TRAIN_SENSOR_PRIORITY, train_sensor );
	assert( sensor_tid > 0 );
	
	while ( !quit ) {
		// await input
		while( 1 ){
			data = Getc( COM_2 );
			if( data == '\r' || data == '\n' ){
				buf[ buf_i ] = '\0';
				break;
			}
			
			if( isalpha( data ) || isdigit( data ) ){
				buf[ buf_i ] = data;
			} else if( isspace( data ) ) {
				buf[ buf_i ] = ' ';
			} else {
				buf_i -= 1;
			}

			buf_i += 1;
		}

		status = parse_fill( buf, ' ', token_buf, TRAIN_COMMAND_MAX_TOKEN, &token_filled );
		assert( status == ERR_NONE );

		command_status = CMD_SUCCESS;

		if( ! strcmp( token_buf[ 0 ], "q" ) ){
			quit = 1;
			region_printf( &result_reg, "Terminating...\n" );
		} else if( ! strcmp( token_buf[ 0 ], "wh" ) ){
			region_printf( &result_reg, "Not implemented\n" );
		} else if( ! strcmp( token_buf[ 0 ], "sall" ) ){
			region_printf( &result_reg, "Not implemented\n" );
		} else if( ! strcmp( token_buf[ 0 ], "call" ) ){
			region_printf( &result_reg, "Not implemented\n" );
		} else if( ! strcmp( token_buf[ 0 ], "tr" ) ){
			region_printf( &result_reg, "Not implemented\n" );
		} else if( ! strcmp( token_buf[ 0 ], "rv" ) ){
			region_printf( &result_reg, "Not implemented\n" );
		} else if( ! strcmp( token_buf[ 0 ], "st" ) ){
			region_printf( &result_reg, "Not implemented\n" );
		} else if( ! strcmp( token_buf[ 0 ], "sw" ) ){
			region_printf( &result_reg, "Not implemented\n" );
		}

		switch( command_status ){
		case CMD_SUCCESS:
		default:
			break;
		}
			
#if 0
		// parse input
		int start;
		int arg0;
		int arg1;
		
		switch (data) {
		case '\r':
			// trigger
			if ( buf_i == 0 ) {
				/* empty line */
				cmd.command = N;
			}
			else if ( buf_i == 1 ) {
				if ( buf[0] == 'q' ) {
					/* q: quit */
					cmd.command = Q;
				}
				else {
					cmd.command = X;
				}
			}
			else if ( buf_i == 2 ) {
				if (( buf[0] == 'w' )&&( buf[1] == 'h' )) {
					/* wh: the last sensor */
					cmd.command = WH;
				}
				else if (( buf[0] == 'p' )&&( buf[1] == 't' )) {
					/* PR: the last sensor */
					cmd.command = PT;
				}
				else {
					cmd.command = X;
				}
			}
			else if (( buf_i == 4 )&&( buf[0] == 's' )&&( buf[1] == 'a' )&&( buf[2] == 'l' )&&( buf[3] == 'l' )){
				cmd.command = SA;
				cmd.args[0] = 33;
			}
			else if (( buf_i == 4 )&&( buf[0] == 'c' )&&( buf[1] == 'a' )&&( buf[2] == 'l' )&&( buf[3] == 'l' )){
				cmd.command = SA;
				cmd.args[0] = 34;
			}
			else if (( buf[0] == 't' )&&( buf[1] == 'r' )) {
				/* TR: set train speed */
				arg0 = parse_int( buf, 3, buf_i, &start );
				arg1 = parse_int( buf, start+1, buf_i, &start );

				if (( arg0 == PARSE_INT_FAIL )||( arg1 == PARSE_INT_FAIL )||( start != buf_i )) {
					cmd.command = X;
				}
				else {
					cmd.command = TR;
					cmd.args[0] = arg0;
					cmd.args[1] = arg1;
				}
			}
			else if (( buf[0] == 'r' )&&( buf[1] == 'v' )){
				/* RV: rv train movement */
				arg0 = parse_int( buf, 3, buf_i, &start );

				if (( arg0 == PARSE_INT_FAIL )||( start != buf_i )) {
					cmd.command = X;
				}
				else {
					cmd.command = RV;
					cmd.args[0] = arg0;
				}
			}
			else if (( buf[0] == 's' )&&( buf[1] == 't' )){
				/* RV: rv train movement */
				arg0 = parse_int( buf, 3, buf_i, &start );

				if (( arg0 == PARSE_INT_FAIL )||( start != buf_i )) {
					cmd.command = X;
				}
				else {
					cmd.command = ST;
					cmd.args[0] = arg0;
				}
			}
			else if (( buf[0] == 's' )&&( buf[1] == 'w' )&&(( buf[buf_i-1] == 'S' )||( buf[buf_i-1] == 'C' )||( buf[buf_i-1] == 's' )||( buf[buf_i-1] == 'c' ))){
				/* SW: shift switch */
				arg0 = parse_int( buf, 3, buf_i, &start );

				if (( arg0 == PARSE_INT_FAIL )||( start != buf_i-2 )) {
					cmd.command = X;
				}
				else {
					cmd.command = SW;
					cmd.args[0] = arg0;
					if ( buf[buf_i-1] == 'S' || buf[buf_i-1] == 's' ){
						cmd.args[1] = 33;
					}
					else {
						cmd.args[1] = 34;
					}
				}
			}
			else {
				cmd.command = X;
			}
			echo( echo_region, " \n" );
			for ( i = 0; i < MAX_BUFFER_SIZE; i++ ){
				screen->nextline[i] = buf[i];
				if ( buf[i] == 0 ) break;
				buf[i] = 0;
			}
			buf_i = 0;
			break;
		case '\b':
			// undo
			if ( buf_i > 0 ){
				buf_i--;
				buf[buf_i] = 0;
				echo( echo_region, buf );
			}
			else {
				echo( echo_region, " \n" );
			}
			continue;
		default: 
			// echo
			buf[buf_i] = data;
			buf_i++;
			assert( buf_i < MAX_BUFFER_SIZE );
			echo( echo_region, buf );
			continue;
		}
		
		// do action
		switch ( cmd.command ) {
		case N:
			break;
		case Q:
			quit = 1;
			ack( ack_region, "Goodbye!", screen );
			break;
		case TR:
			ack( ack_region, "Train speed changes", screen );
			status = train_set_speed( cmd.args[0], cmd.args[1] );
			assert( status == ERR_NONE );
			break;
		case RV:
			ack( ack_region, "Train reverses", screen );
			status = train_reverse( cmd.args[0] );
			assert( status == ERR_NONE );
			break;
		case SW:
			ack( ack_region, "Switch shifts", screen );
			status = train_switch( cmd.args[0], cmd.args[1] );
			assert( status == ERR_NONE );
			break;
		case ST:
			status = train_check_switch( cmd.args[0] );
			ack_st( ack_region, cmd.args[0], (char)status, screen );
			break;
		case WH:
			status = train_last_sensor();
			if ( status ) {
				ack_wh( ack_region, status, screen );
			}
			break;
		case SA:
			ack( ack_region, "Shift all switches", screen );
			status = train_switch_all( cmd.args[0] );
			assert( status == ERR_NONE );
			break;
		case PT:
			ack( ack_region, "pressure test", screen );
			break;
		default:
			ack( ack_region, "Invalid command", screen );
			break;
		}
#endif /* if 0 */
	}

	// tell children to suicide
	status = train_module_suicide( module_tid );
	assert( status == 0 );

	Kill( clock_tid );
	Kill( sensor_tid );
	Kill( sensor_ui_tid );
	// Kill( switch_tid );

	sync_responde( MyParentTid() );

	DEBUG_NOTICE( DBG_USER, "quit!\n" );

	Exit();
}

