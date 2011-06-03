#include <types.h>
#include <user/syscall.h>
#include <user/name_server.h>
#include <devices/clock.h>
#include <user/devices/clock.h>
#include <user/event.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <user/time.h>
#include <err.h>
#include <bwio.h>

#define CLOCK_CLK_SRC                CLK_SRC_2KHZ
#define CLOCK_TICKS_PER_MS           ( CLK_SRC_2KHZ_SPEED / 1000 )
#define CLOCK_ACTUAL_TICKS_FACTOR    ( CLOCK_COUNT_DOWN_MS_PER_TICK *CLOCK_TICKS_PER_MS )
#define CLOCK_USER_TICK_TO_SYSTEM_TICK( user_ticks )   ( ( user_ticks ) * CLOCK_ACTUAL_TICKS_FACTOR )
#define CLOCK_SYSTEM_TICK_TO_USER_TICK( system_ticks )   ( ( system_ticks ) / CLOCK_ACTUAL_TICKS_FACTOR )

enum Clock_request_internal {
	CLOCK_COUNT_DOWN_COMPLETE = CLOCK_QUIT + 1
};

#define CLOCK_MAGIC           0xc10c411e
#define CLOCK_EVENT_MAGIC     0xe11e1171

typedef struct Clock_event_s Clock_event_request;
typedef struct Clock_event_s Clock_event_reply;

struct Clock_event_s {
	uint magic;
	uint type;
};

enum Clock_event_type {
	CLOCK_EVENT_START_WAIT,
	CLOCK_EVENT_QUIT
};

/* Internal only */
static int clock_cd_complete( int tid );

static int clock_event_start( int tid )
{
	Clock_event_request request;
	Clock_event_reply reply;
	int status;

	request.magic = CLOCK_EVENT_MAGIC;

	status = Send( tid, ( char* )&request, sizeof( request ), ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );
	assert( reply.magic == CLOCK_EVENT_MAGIC );
	assert( reply.type == request.type );

	return ERR_NONE;
}

static int clock_evnet_stop( int tid ){
	Clock_event_request request;
	Clock_event_reply reply;
	int status;

	request.magic = CLOCK_EVENT_MAGIC;
	request.type = CLOCK_EVENT_QUIT;

	status = Send( tid, ( char* )&request, sizeof( request ), ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );
	assert( reply.magic == CLOCK_EVENT_MAGIC );
	assert( reply.type == request.type );

	return ERR_NONE;
}

static void clock_event_handler()
{
	Clock_event_request request;
	Clock_event_reply reply;
	int tid = 0;
	int clock_drv_tid = MyParentTid();
	int status = 0;

	reply.magic = CLOCK_EVENT_MAGIC;

	DEBUG_NOTICE( DBG_CLK_DRV, "launch\n" );
	
	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		assert( status == sizeof( request ) );
		assert( request.magic == CLOCK_EVENT_MAGIC );

		reply.type = request.type;

		status = Reply( tid, ( char* )&reply, sizeof( reply ) );
		assert( status == SYSCALL_SUCCESS );

		if( request.type == CLOCK_EVENT_QUIT ){
			DEBUG_NOTICE( DBG_CLK_DRV, "quitting\n" );
			break;
		}

		DEBUG_NOTICE( DBG_CLK_DRV, "received request\n" );
		
		AwaitEvent( EVENT_SRC_TC1UI );

		DEBUG_NOTICE( DBG_CLK_DRV, "received interrupt\n" );

		status = clock_cd_complete( clock_drv_tid );
		assert( status == ERR_NONE );
	}

	DEBUG_NOTICE( DBG_CLK_DRV, "quit!\n" );
	Exit();
}

void clock_main()
{
	Clock clock_1 = {0};         /* For interrupt */
	Clock clock_3 = {0};         /* For timer */
	Clock_request request;
	Clock_reply reply;
	int event_handler_tid = 0;
	int request_tid;
	int time_tid = MyParentTid();
	uint current_time = 0;
	uint current_cd = 0;
	uint event_handling = 0;
	uint cd_ticks;
	int execute = 1;
	int status = 0;

	/* clk_reset to ensure the clock is fully reset */
	clk_enable( &clock_1, CLK_1, CLK_MODE_INTERRUPT, CLOCK_CLK_SRC, ~0 );
	clk_reset( &clock_1, ~0 );
	clk_clear( &clock_1 );
	clk_enable( &clock_3, CLK_3, CLK_MODE_FREE_RUN, CLOCK_CLK_SRC, ~0 );
	clk_reset( &clock_3, ~0 );
	clk_clear( &clock_3 );

	event_handler_tid = Create( 0, clock_event_handler );
	assert( event_handler_tid > 0 );

	reply.magic = CLOCK_MAGIC;

	DEBUG_NOTICE( DBG_CLK_DRV, "launch\n" );

	while( execute ){
		status = Receive( &request_tid, ( char* )&request, sizeof( request ) );
		assert( status == sizeof( request ) );
		assert( request.magic == CLOCK_MAGIC );

		reply.type = request.type;

		DEBUG_PRINT( DBG_CLK_DRV, "received request, type %u\n", request.type );

		/* Assert message source */
#ifdef DEBUG
		switch( request.type ){
		case CLOCK_CURRENT_TICK:
		case CLOCK_COUNT_DOWN:
		case CLOCK_QUIT:
			assert( request_tid == time_tid );
			break;
		case CLOCK_COUNT_DOWN_COMPLETE:
			assert( request_tid == event_handler_tid );
			break;
		default:
			assert( 0 );
		}
#endif

		/* Update current time */
		status = clk_diff_cycles( &clock_3, &reply.data );
		assert( status == ERR_NONE );

		current_time += reply.data;

		/* Process request */
		switch( request.type ){
		case CLOCK_CURRENT_TICK:
			reply.data = CLOCK_SYSTEM_TICK_TO_USER_TICK( current_time );
			break;
		case CLOCK_COUNT_DOWN:
			status = clk_value( &clock_1, &current_cd );
			assert( status == ERR_NONE );

			cd_ticks = CLOCK_USER_TICK_TO_SYSTEM_TICK( request.data );
			if( cd_ticks < current_time ){
				cd_ticks = 1;
			} else {
				cd_ticks -= current_time;
			}
			
			/* Reset clock interrupt */
			if( ( ! event_handling ) || ( cd_ticks  + CLOCK_OPERATION_TICKS ) < current_cd ){
				status = clk_reset( &clock_1, cd_ticks );
				assert( status == ERR_NONE );
			}

			/* Send request to clock_event_start */
			if( ! event_handling ){
				status = clock_event_start( event_handler_tid );
				assert( status == ERR_NONE );
				event_handling = 1;
			}
			break;
		case CLOCK_QUIT:
			status = clock_evnet_stop( event_handler_tid );
			execute = 0;
			break;
		case CLOCK_COUNT_DOWN_COMPLETE:
			clk_clear( &clock_1 );
			event_handling = 0;
			time_signal( time_tid );
			break;
		}

		status = Reply( request_tid, ( char* )&reply, sizeof( reply ) );
		assert( status == SYSCALL_SUCCESS );

		DEBUG_NOTICE( DBG_CLK_DRV, "request responded\n" );
	}

	Exit();
}

static int clock_request( int tid, uint type, uint* data )
{
	Clock_request request;
	Clock_reply reply;
	int status = 0;

	request.magic = CLOCK_MAGIC;
	request.type = type;
	request.data = *data;

	status = Send( tid, ( char* )&request, sizeof( request ), ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );
	assert( reply.magic == CLOCK_MAGIC );
	assert( reply.type == request.type );

	*data = reply.data;

	return ERR_NONE;
}

/* time will be returned with the current ticks */
int clock_current_time( int tid, uint* time )
{
	return clock_request( tid, CLOCK_CURRENT_TICK, time );
}

int clock_count_down( int tid, uint ticks )
{
	return clock_request( tid, CLOCK_COUNT_DOWN, &ticks );
}

int clock_quit( int tid )
{
	uint buf;
	return clock_request( tid, CLOCK_QUIT, &buf );
}

static int clock_cd_complete( int tid )
{
	uint buf;
	return clock_request( tid, CLOCK_COUNT_DOWN_COMPLETE, &buf );
}
