#include <types.h>
#include <user/syscall.h>
#include <user/name_server.h>
#include <devices/clock.h>
#include <user/devices/clock.h>
#include <user/event.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <user/time.h>
#include <user/courier.h>
#include <err.h>
#include <bwio.h>
#include <config.h>

#define CLOCK_CLK_SRC                CLK_SRC_2KHZ
#define CLOCK_TICKS_PER_MS           ( CLK_SRC_2KHZ_SPEED / 1000 )
#define CLOCK_ACTUAL_TICKS_FACTOR    ( CLOCK_COUNT_DOWN_MS_PER_TICK *CLOCK_TICKS_PER_MS )
#define CLOCK_USER_TICK_TO_SYSTEM_TICK( user_ticks )   ( ( user_ticks ) * CLOCK_ACTUAL_TICKS_FACTOR )
#define CLOCK_SYSTEM_TICK_TO_USER_TICK( system_ticks )   ( ( system_ticks ) / CLOCK_ACTUAL_TICKS_FACTOR )

enum Clock_request_type {
	CLOCK_CURRENT_TICK,
	CLOCK_COUNT_DOWN,
	CLOCK_QUIT,             /* Only parent can send */
	CLOCK_COUNT_DOWN_COMPLETE
};

#define CLOCK_MAGIC           0xc10c411e

typedef struct Clock_request_s Clock_request;
typedef struct Clock_reply_s Clock_reply;


struct Clock_request_s {
#ifdef IPC_MAGIC
	unsigned int magic;
#endif
	unsigned int type;
	unsigned int data;
};

struct Clock_reply_s {
#ifdef IPC_MAGIC
	unsigned int magic;
	unsigned int type;
#endif
	unsigned int data;
};

/* Internal only */
static int clock_cd_complete( int tid );

static int clock_signal_time( int tid, int dummy )
{
	time_signal( tid );

	return ERR_NONE;
}

void clock_main()
{
	Clock clock_1 = {0};         /* For interrupt */
	Clock clock_3 = {0};         /* For timer */
	Clock_request request;
	Clock_reply reply;
	int event_handler_tid;
	int courier_tid;
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

	/* Interrupt handler */
	event_handler_tid = Create_drv( 0, event_handler );
	assert( event_handler_tid > 0 );

	status = event_init( event_handler_tid, EVENT_SRC_TC1UI, clock_cd_complete );
	assert( status == ERR_NONE );

	/* Courier for communicating with time */
	courier_tid = Create( 1, courier );
	assert( status == ERR_NONE );

	status = courier_init( courier_tid, clock_signal_time );
	assert( status == ERR_NONE );

#ifdef IPC_MAGIC
	reply.magic = CLOCK_MAGIC;
#endif

	DEBUG_NOTICE( DBG_CLK_DRV, "launch\n" );

	while( execute ){
		status = Receive( &request_tid, ( char* )&request, sizeof( request ) );
		assert( status == sizeof( request ) );
#ifdef IPC_MAGIC
		assert( request.magic == CLOCK_MAGIC );
		reply.type = request.type;
#endif

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

			/* Send request to event_start */
			if( ! event_handling ){
				status = event_start( event_handler_tid );
				assert( status == ERR_NONE );
				event_handling = 1;
			}
			break;
		case CLOCK_QUIT:
			status = event_quit( event_handler_tid );
			execute = 0;
			break;
		case CLOCK_COUNT_DOWN_COMPLETE:
			clk_clear( &clock_1 );
			event_handling = 0;
			status = courier_go( courier_tid, time_tid, 0 );
			assert( status == ERR_NONE );
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

#ifdef IPC_MAGIC
	request.magic = CLOCK_MAGIC;
#endif
	request.type = type;
	request.data = *data;

	status = Send( tid, ( char* )&request, sizeof( request ), ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );
#ifdef IPC_MAGIC
	assert( reply.magic == CLOCK_MAGIC );
	assert( reply.type == request.type );
#endif

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
