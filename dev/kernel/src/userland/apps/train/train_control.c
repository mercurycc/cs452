#include <types.h>
#include <err.h>
#include <lib/str.h>
#include <user/apps_entry.h>
#include <user/assert.h>
#include <user/display.h>
#include <user/syscall.h>
#include <user/uart.h>
#include <user/name_server.h>
#include <user/lib/sync.h>
#include <user/lib/parser.h>
#include "inc/track_node.h"
#include "inc/track_reserve.h"
#include "inc/train.h"
#include "inc/config.h"
#include "inc/sensor_data.h"
#include "inc/warning.h"

#define MAX_BUFFER_SIZE 128
#define MAX_SCREEN_SIZE 1024
#define PARSE_INT_FAIL  -1

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
	int switch_tid;
	int module_tid;
	int auto_tid;
	int tracking_ui_tid;
	int track_reserve_tid;
	int quit = 0;
	char data;
	char buf[MAX_BUFFER_SIZE] = {0};
	char* token_buf[ TRAIN_COMMAND_MAX_TOKEN ] = {0};
	int buf_i = 0;
	int token_filled;
	int i;
	Region title_reg = { 2, 1, 1, 14 - 2, 1, 0 };
	/* Prompt UI */
	Region prompt_titles = { 2, 20, 1, 11 - 2, 1, 0 };
	Region prompt_reg = { 2, 21, 1, 78 - 2, 1, 0 };
	Region result_reg = { 14, 22, 1, 78 - 14, 1, 0 };
	Region warning_reg = WARNING_REGION;
	// int prompt_width = prompt_reg.width - prompt_reg.margin - prompt_reg.margin;
	int fail;
	
	int status;

	status = RegisterAs( CONTROL_NAME );
	assert( status == REGISTER_AS_SUCCESS );

	status = region_init( &prompt_reg );
	assert( status == ERR_NONE );
	status = region_init( &result_reg );
	assert( status == ERR_NONE );
	status = region_init( &warning_reg );
	assert( status == ERR_NONE );
	status = region_init( &title_reg );
	assert( status == ERR_NONE );

	/* Print title */
	/* System */
	region_printf( &title_reg, "Pinball P2" );
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

	/* Prompt */
	region_printf( &prompt_reg, "$" );
	prompt_reg.col += 3;

	status = region_printf( &result_reg, "Please wait for the track to initialize\n" );
	assert( status == ERR_NONE );

	switch_tid = Create( TRAIN_UI_PRIORITY, switches_ui );
	assert( switch_tid > 0 );

	module_tid = Create( TRAIN_MODULE_PRIORITY, train_module );
	assert( module_tid > 0 );

	clock_tid = Create( TRAIN_UI_PRIORITY, clock_ui );
	assert( clock_tid > 0 );

	sensor_ui_tid = Create( TRAIN_UI_PRIORITY, sensor_ui );
	assert( sensor_ui_tid > 0 );

	sensor_tid = Create( TRAIN_SENSOR_PRIORITY, train_sensor );
	assert( sensor_tid > 0 );

	auto_tid = Create( TRAIN_AUTO_PRIROTY, train_auto );
	assert( auto_tid > 0 );

	tracking_ui_tid = Create( TRAIN_UI_PRIORITY, tracking_ui );
	assert( tracking_ui_tid > 0 );

	track_reserve_tid = Create( TRAIN_AUTO_PRIROTY, track_reserve );
	assert( track_reserve_tid > 0 );

	sync_wait();

	region_printf( &result_reg, "Which track am I using? (a/b)\n" );
	
	while( 1 ){
		data = Getc( COM_2 );

		if( data == 'a' || data == 'A' ){
			region_printf( &result_reg, "Track A selected\n" );
			train_auto_init( auto_tid, TRACK_A );
		} else if( data == 'b' || data == 'B' ){
			region_printf( &result_reg, "Track B selected\n" );
			train_auto_init( auto_tid, TRACK_B );
		} else {
			continue;
		}
		
		break;
	}

	train_auto_set_switch_all( auto_tid, 'C' );

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

			if( isalpha( data ) || isdigit( data ) || isspace( data ) || data == '-' ){
				region_printf( &prompt_reg, "%c", data );
				prompt_reg.col += 1;
				buf_i += 1;
			} else if( data == '\b' && buf_i ) {
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
			status = train_auto_query_sensor( auto_tid, &group, &id );
			assert( status == ERR_NONE );

			sensor_id2name( name, group, id );
			if( group < 0 ){
				region_printf( &result_reg, "No sensor has been triggered yet\n" );
			} else {
				region_printf( &result_reg, "Most recent sensor: %s\n", name );
			}
		} else if( ! strcmp( token_buf[ 0 ], "sall" ) ){
			train_switch_all( module_tid, 'S' );
			train_auto_set_switch_all( auto_tid, 'S' );
			region_printf( &result_reg, "Swap all switches to straight\n" );
		} else if( ! strcmp( token_buf[ 0 ], "call" ) ){
			train_switch_all( module_tid, 'C' );
			train_auto_set_switch_all( auto_tid, 'C' );
			region_printf( &result_reg, "Swap all switches to curve\n" );
		} else if( ! strcmp( token_buf[ 0 ], "tr" ) ){
			int args[ 2 ];
			if( token_filled == 3 ){
				args[ 0 ] = ( int )stou( token_buf[ 1 ] );
				args[ 1 ] = ( int )stou( token_buf[ 2 ] );
				train_set_speed( module_tid, args[ 0 ], args[ 1 ] );
				train_auto_set_speed( auto_tid, args[ 0 ], args[ 1 ] );
				region_printf( &result_reg, "Set train %d to speed level %d\n", args[ 0 ], args[ 1 ] );
			} else {
				region_printf( &result_reg, "tr <train id> <speed level (0 - 14)>\n" );
			}
		} else if( ! strcmp( token_buf[ 0 ], "rv" ) ){
			int arg;
			if( token_filled == 2 ){
				arg = ( int )stou( token_buf[ 1 ] );
				/* Reverse direction */
				train_reverse( module_tid, arg );
				train_auto_set_reverse( auto_tid, arg );

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
					train_auto_query_switch( auto_tid, switch_id, &i );
					direction = ( char )i;
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
					train_auto_set_switch( auto_tid, switch_id, direction );
					region_printf( &result_reg, "Set switch %d to %c\n", switch_id, direction );
				}
			} else {
				region_printf( &result_reg, "sw <switch id> <direction [C|S]>\n" );
			}
		} else if( ! strcmp( token_buf[ 0 ], "reg" ) ){
			int train_id;
			if( token_filled == 2 ){
				train_id = ( int )stou( token_buf[ 1 ] );
				train_auto_new_train( auto_tid, train_id );
				region_printf( &result_reg, "Register train %d\n", train_id );
			} else {
				region_printf( &result_reg, "%s <train id>\n", token_buf[ 0 ] );
			}
		} else if( ! strcmp( token_buf[ 0 ], "plan" ) ){
			int train_id;
			int group;
			int id;
			int dist_pass;
			if( token_filled == 4 ){
				train_id = ( int )stou( token_buf[ 1 ] );
				status = track_node_name2id( token_buf[ 2 ], &group, &id );
				if( status != ERR_NONE ){
					region_printf( &result_reg, "%s is not a valid point\n", token_buf[ 2 ] );
					fail = 1;
				}
				dist_pass = ( int )stoi( token_buf[ 3 ] );
				if( ! fail ){
					train_auto_plan( auto_tid, train_id, group, id, dist_pass );
					region_printf( &result_reg, "Plan to %s (%d, %d), %d for train %d\n", token_buf[ 2 ], group, id, dist_pass, train_id );
				}
			} else {
				region_printf( &result_reg, "%s <train id> <land mark> <distance pass the landmard>\n", token_buf[ 0 ] );
			}
		} else if( ! strcmp( token_buf[ 0 ], "sctime" ) ){
			int args[ 3 ];
			if( token_filled == 4 ){
				args[ 0 ] = ( int )stou( token_buf[ 1 ] );
				args[ 1 ] = ( int )stou( token_buf[ 2 ] );
				args[ 2 ] = ( int )stou( token_buf[ 3 ] );
				train_auto_set_train_sc_time( auto_tid, args[ 0 ], args[ 1 ], args[ 2 ] );
				region_printf( &result_reg, "Set train %d speed change time to %d - %d\n", args[ 0 ], args[ 1 ], args[ 2 ] );
			} else {
				region_printf( &result_reg, "sctime <train id> <min time(ticks)> <max time(ticks)>\n" );
			}
		} else {
			region_printf( &result_reg, "Unknown command %s\n", token_buf[ 0 ] );
		}
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

