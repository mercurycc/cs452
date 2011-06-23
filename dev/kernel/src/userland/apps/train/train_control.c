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

/* Train control holds the prompt UI */
void train_control()
{
	int clock_tid;
	int sensor_tid;
	int sensor_ui_tid;
	int sensor_query_tid;
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
	Region warning_reg = WARNING_REGION;
	// int prompt_width = prompt_reg.width - prompt_reg.margin - prompt_reg.margin;
	int fail;
	
	int status;

	status = region_init( &prompt_reg );
	assert( status == ERR_NONE );
	status = region_init( &result_reg );
	assert( status == ERR_NONE );
	status = region_init( &warning_reg );
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
	/* Warning */
	prompt_titles.row = 23;
	status = region_init( &prompt_titles );
	assert( status == ERR_NONE );
	region_printf( &prompt_titles, "Warning" );

	status = region_printf( &result_reg, "Please wait for the track to initialize\n" );
	assert( status == ERR_NONE );

	module_tid = Create( TRAIN_MODULE_PRIORITY, train_module );
	assert( module_tid > 0 );

	/* Prompt */
	region_printf( &prompt_reg, "$" );
	prompt_reg.col += 3;

	switch_tid = Create( TRAIN_UI_PRIORITY, switches_ui );
	assert( switch_tid > 0 );

	clock_tid = Create( TRAIN_UI_PRIORITY, clock_ui );
	assert( clock_tid > 0 );

	sensor_ui_tid = Create( TRAIN_UI_PRIORITY, sensor_ui );
	assert( sensor_ui_tid > 0 );

	sensor_tid = Create( TRAIN_SENSOR_PRIORITY, train_sensor );
	assert( sensor_tid > 0 );

	sensor_query_tid = WhoIs( SENSOR_QUERY_NAME );
	assert( sensor_query_tid > 0 );

	sync_wait();
	region_clear( &result_reg );
	
	while ( !quit ) {
		/* Single character echo */
		prompt_reg.margin = 0;
		prompt_reg.width = 1;

		// await input
		buf_i = 0;
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
			}

			if( isalpha( data ) || isdigit( data ) || isspace( data ) ){
				region_printf( &prompt_reg, "%c", data );
				prompt_reg.col += 1;
				buf_i += 1;
			} else if( data == '\b' ) {
				buf_i -= 1;
				prompt_reg.col -= 1;
				region_printf( &prompt_reg, " " );
			}
		}

		/* Clear echo'ed command */
		prompt_reg.col -= buf_i;
		prompt_reg.width = buf_i;
		region_clear( &prompt_reg );
		
		status = parse_fill( buf, ' ', token_buf, TRAIN_COMMAND_MAX_TOKEN, &token_filled );
		assert( status == ERR_NONE );

		fail = 0;

		if( token_filled == 0 ){
			region_clear( &result_reg );
		} else if( ! strcmp( token_buf[ 0 ], "q" ) ){
			quit = 1;
			region_printf( &result_reg, "Terminating...\n" );
		} else if( ! strcmp( token_buf[ 0 ], "wh" ) ){
			int group;
			int id;
			char name[ SENSOR_NAME_LENGTH + 1 ];
			status = sensor_query_recent( sensor_query_tid, &group, &id );
			assert( status == ERR_NONE );

			sensor_id2name( name, group, id );
			if( group < 0 ){
				region_printf( &result_reg, "No sensor has been triggered yet\n" );
			} else {
				region_printf( &result_reg, "Most recent sensor: %s\n", name );
			}
		} else if( ! strcmp( token_buf[ 0 ], "sall" ) ){
			train_switch_all( module_tid, 'S' );
			region_printf( &result_reg, "Swap all switches to straight\n" );
		} else if( ! strcmp( token_buf[ 0 ], "call" ) ){
			train_switch_all( module_tid, 'C' );
			region_printf( &result_reg, "Swap all switches to curve\n" );
		} else if( ! strcmp( token_buf[ 0 ], "tr" ) ){
			int args[ 2 ];
			if( token_filled == 3 ){
				args[ 0 ] = ( int )stou( token_buf[ 1 ] );
				args[ 1 ] = ( int )stou( token_buf[ 2 ] );
				train_set_speed( module_tid, args[ 0 ], args[ 1 ] );
				region_printf( &result_reg, "Set train %d to speed level %d\n", args[ 0 ], args[ 1 ] );
			} else {
				region_printf( &result_reg, "tr <train id> <speed level (0 - 14)>\n" );
			}
		} else if( ! strcmp( token_buf[ 0 ], "rv" ) ){
			int arg;
			if( token_filled == 2 ){
				arg = ( int )stou( token_buf[ 1 ] );
				train_reverse( module_tid, arg );
				region_printf( &result_reg, "Reverse train %d\n", arg );
			} else {
				region_printf( &result_reg, "rv <train id>\n" );
			}
		} else if( ! strcmp( token_buf[ 0 ], "st" ) ){
			int switch_id;
			char direction;

			if( token_filled == 2 ){
				switch_id = ( int )stou( token_buf[ 1 ] );
				if( switch_id <= 0 || ( switch_id > 18 && switch_id < 153 ) || switch_id > 156 ){
					region_printf( &result_reg, "Switch %d is invalid\n", switch_id );
					fail = 1;
				}
				if( ! fail ){
					direction = train_check_switch( module_tid, switch_id );
					region_printf( &result_reg, "Switch %d is %s\n", switch_id, direction == 'C' ? "curve" : "straight" );
				}
			} else {
				region_printf( &result_reg, "st <switch id (1-18, 153-156)>\n" );
			}
		} else if( ! strcmp( token_buf[ 0 ], "sw" ) ){
			int switch_id;
			char direction;
			if( token_filled == 3 ){
				switch_id = ( int )stou( token_buf[ 1 ] );
				direction = token_buf[ 2 ][ 0 ];
				if( direction == 'c' || direction == 's' ){
					direction -= ( int )'a' - ( int )'A';
				} else if( ! ( direction == 'C' || direction == 'S' ) ){
					region_printf( &result_reg, "Direction can only be C or S\n" );
					fail = 1;
				}
				if( switch_id <= 0 || ( switch_id > 18 && switch_id < 153 ) || switch_id > 156 ){
					region_printf( &result_reg, "Switch %d is invalid\n", switch_id );
					fail = 1;
				}
				if( ! fail ){
					assert( direction == 'C' || direction == 'S' );
					train_switch( module_tid, switch_id, direction );
					region_printf( &result_reg, "Set switch %d to %c\n", switch_id, direction );
				}
			} else {
				region_printf( &result_reg, "sw <switch id> <direction [C|S]>\n" );
			}
		} else {
			region_printf( &result_reg, "Unknown command %s\n", token_buf[ 0 ] );
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
	Kill( switch_tid );

	sync_responde( MyParentTid() );

	DEBUG_NOTICE( DBG_USER, "quit!\n" );

	Exit();
}

