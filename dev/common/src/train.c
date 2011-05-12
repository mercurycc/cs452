#include <types.h>
#include <console.h>
#include <train.h>
#include <bufio.h>
#include <rbuf.h>
#include <err.h>
#include <str.h>
#include <time.h>
#include <sensorctrl.h>

#define SWITCH_STRAIGHT_CMD_0       33
#define SWITCH_CURVE_CMD_0          34
#define SWITCH_TURNOFF_SOLENOID_CMD 32
#define SENSOR_QUERY_GROUP_BASE     0x80

unsigned int TRAIN_COMMAND_GAP = 150;  /* Initial speed for initialization */

static Console cons;                /* COM1 */
static Rbuf cmdringbody;            /* Ring buffer for commands */
static Rbuf* cmdring;
static uchar cmdoutbuf[ TRAIN_COMMAND_BUFFER_LENGTH ];       /* Memory block for ring buffer */
static Iobuf panelbody;             /* Buffered I/O port */
static Iobuf* panel;
static uint trainSpeed[ TRAIN_MAX_TRAINS ];
static uint querySent;
static uint received;
static time_t sensorQueryLastFlushDone = 0;
static uint switchStates[ SWITCHES_COUNT ];
static uint mostRecentSwitch = 0;

enum Commands { TRAIN_SET_SPEED,
		TRAIN_STOP,
		TRAIN_REVERSE,
		SWITCH_STRAIGHT,
		SWITCH_CURVE,
		SWITCH_TURNOFF_SOLENOID,
		SENSOR_QUERY_ALL };

typedef struct Train_ctrl_s {
	uint id;      /* Address */
	uint cmd;     /* Command to execute */
	uint param1;  /* Parameter 1 */
	uint func;    /* TODO */
} Train_ctrl;

int train_init()
{
	int status = 0;
	uint i = 0;
	uint swi = 0;

	/* Global variable initialization */
	panel = &panelbody;
	cmdring = &cmdringbody;
	for( i = 0; i < TRAIN_MAX_TRAINS; i += 1 ){
		trainSpeed[ i ] = 0;
	}
	querySent = 0;
	received = 0;

	/* Initialize console */
	status = console_setup( &cons, CONSOLE_1, 2400, 0, 1, 1 );
	ASSERT( status == ERR_NONE );

	status = console_init( &cons );
	ASSERT( status == ERR_NONE );

	/* Initialize buffered I/O */
	status = iobuf_init( panel, &cons );
	ASSERT( status == ERR_NONE );

	/* Initialized command ring */
	status = rbuf_init( cmdring, cmdoutbuf, 2, TRAIN_COMMAND_BUFFER_LENGTH );
	ASSERT( status == ERR_NONE );

	/* Initialize track */
	for( i = 0; i < SWITCHES_COUNT; i += 1 ){
		switchStates[ i ] = SWITCH_CURVE;
		swi = train_switch_id_to_swi( i );		
		status = train_switch_curve( swi );
		ASSERT( status == ERR_NONE );
	}

	return ERR_NONE;
}

static inline uint train_command_get_size( const uchar* cmd )
{
	if( *cmd == 0x20 ) {
		/* Turn off solenoid */
		return 1;
	} else if( *cmd >= 0xc1 && *cmd <= 0xdf ){
		/* Read single sensor */
		return 1;
	} else if( *cmd >= 0x81 && *cmd <= 0x9f ){
		/* Read group of sensors */
		return 1;
	} else if( *cmd == 0x60 || *cmd == 0x61 ){
		/* Go or Stop*/
		return 1;
	} else {
		return 2;
	}
}

int train_command_flushed()
{
	return rbuf_empty( cmdring ) && iobuf_flushed( panel );
}

int train_command_flush()
{
	static uchar command[ 2 ];          /* Buffer for storing the command */
	static uint flashing = 0;
	static time_t lastFlushDone = 0;
	uint temp;
	int status = 0;

	// ENTERFUNC();
	
	if( iobuf_flushed( panel ) ){
		if( flashing ){
			flashing = 0;
			status = time_current( &temp, &lastFlushDone );
			ASSERT( status == ERR_NONE );
		}
		if( !( rbuf_empty( cmdring ) ) ){
			rbuf_get( cmdring, command );
			/* Note iobuf_write will not flush the buffer, so we are safe here */
			iobuf_write( panel, command, train_command_get_size( command ) );
			flashing = 1;
		}
	}

	if( flashing ){
		if( time_timer( lastFlushDone, TRAIN_COMMAND_GAP ) ){
			status = iobuf_flush( panel );
			ASSERT( status == ERR_NONE );
		}
	}

	return ERR_NONE;
}

static int train_control( Train_ctrl* param )
{
	uchar cmd[ 2 ];
	int status = 0;

	cmd[ 1 ] = param->id;

	switch( param->cmd ){
	case TRAIN_SET_SPEED:
		cmd[ 0 ] = param->param1;
		break;
	case TRAIN_STOP:
		cmd[ 0 ] = 0;
		break;
	case TRAIN_REVERSE:
		cmd[ 0 ] = TRAIN_SPEED_REVERSE;
		break;
	case SWITCH_STRAIGHT:
		cmd[ 0 ] = SWITCH_STRAIGHT_CMD_0;
		break;
	case SWITCH_CURVE:
		cmd[ 0 ] = SWITCH_CURVE_CMD_0;
		break;
	case SWITCH_TURNOFF_SOLENOID:
		cmd[ 0 ] = SWITCH_TURNOFF_SOLENOID_CMD;
		break;
	case SENSOR_QUERY_ALL:
		cmd[ 0 ] = SENSOR_QUERY_GROUP_BASE + SENSOR_GROUPS;
		break;
	}

	/* Record speed */
	switch( param->cmd ){
	case TRAIN_SET_SPEED:
	case TRAIN_STOP:
		trainSpeed[ cmd[ 1 ] ] = cmd[ 0 ];
		break;
	}

	status = rbuf_put( cmdring, cmd );
	ASSERT( status == ERR_NONE );

	/* status = console_write_guarantee( panel, cmd, 2 ); */
	/* ASSERT( status == ERR_NONE ); */

	return ERR_NONE;
}

int train_set_speed( uint train, uint speed )
{
	Train_ctrl ctrl;

	ctrl.id = train;
	ctrl.cmd = TRAIN_SET_SPEED;
	ctrl.param1 = speed;

	train_control( &ctrl );
	
	return ERR_NONE;
}

int train_reverse_direction( uint train )
{
	Train_ctrl ctrl;

	ctrl.id = train;
	ctrl.cmd = TRAIN_REVERSE;

	train_control( &ctrl );
	train_set_speed( train, trainSpeed[ train ] );
	
	return ERR_NONE;
}

int train_stop( uint train )
{
	Train_ctrl ctrl;

	ctrl.id = train;
	ctrl.cmd = TRAIN_STOP;

	train_control( &ctrl );
	
	return ERR_NONE;
}

uint train_switch_id_to_swi( uint id )
{
	ASSERT( id < SWITCHES_COUNT );
	if( id < 18 ){
		return id + 1;
	} else {
		return id - 18 + 153;
	}
}

uint train_switch_swi_to_id( uint swi )
{
	ASSERT( ( swi > 0 && swi <= 18 ) || ( swi >= 153 && swi <= 156 ) );
	if( swi <= 18 ){
		return swi - 1;
	} else {
		return swi - 153 + 19;
	}
}

/* Switch control */
static inline int train_switch_turnoff()
{
	Train_ctrl ctrl;

	ctrl.cmd = SWITCH_TURNOFF_SOLENOID;
	train_control( &ctrl );
	
	return ERR_NONE;
}

int train_switch_straight( uint swi )
{
	Train_ctrl ctrl;

	ctrl.id = swi;
	ctrl.cmd = SWITCH_STRAIGHT;

	if( switchStates[ train_switch_swi_to_id( swi ) ] == SWITCH_CURVE ){
		mostRecentSwitch = swi;		
	}

	train_control( &ctrl );
	train_switch_turnoff();
	
	return ERR_NONE;
}

int train_switch_curve( uint swi )
{
	Train_ctrl ctrl;

	ctrl.id = swi;
	ctrl.cmd = SWITCH_CURVE;

	if( switchStates[ train_switch_swi_to_id( swi ) ] == SWITCH_STRAIGHT ){
		mostRecentSwitch = swi;		
	}

	train_control( &ctrl );
	train_switch_turnoff();
	
	return ERR_NONE;
}

int train_switch_recent_switch( uint* swi )
{
	*swi = mostRecentSwitch;

	return ERR_NONE;
}

int train_sensor_query_one( uint sensor, uchar* data )
{
	return ERR_NONE;
}

int train_sensor_query_all( uchar* data )
{
	const uint expect = SENSOR_GROUPS * SENSOR_DATA_SIZE;
	Train_ctrl ctrl;
	int status = 0;
	uint wrap = 0;

	if( querySent && time_timer( sensorQueryLastFlushDone, TRAIN_SENSOR_QUERY_TIME_OUT ) ){
		DEBUG_NOTICE( DBG_TRAIN, "Timeout\n" );
		querySent = 0;
		received = 0;
		console_drain( &cons );
	}
	
	if( ! querySent ){
		DEBUG_NOTICE( DBG_TRAIN, "Query sent\n" );
		ctrl.cmd = SENSOR_QUERY_ALL;
		received = 0;
		train_control( &ctrl );
		querySent = 1;
		time_current( &wrap, &sensorQueryLastFlushDone );
	}

	if( querySent ){
		status = console_read( &cons, data + received, expect - received );
		ASSERT( status == ERR_NONE );

		if( cons.last_read ){
			DEBUG_PRINT( DBG_TRAIN, "Fresh %u at %u: %x %x %x %x %x %x %x %x %x %x\n",
				     cons.last_read, received,
				     data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ], data[ 4 ], data[ 5 ], data[ 6 ], data[ 7 ], data[ 8 ], data[ 9 ] );
		}

		received += cons.last_read;

		if( cons.last_read ){
			time_current( &wrap, &sensorQueryLastFlushDone );
		}

		ASSERT( received <= expect );

		if( received == expect ){
			querySent = 0;
			received = 0;
			return ERR_NONE;
		}
	}

	return ERR_NOT_READY;
}
