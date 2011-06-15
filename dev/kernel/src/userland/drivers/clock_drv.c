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
#include <config.h>

#define CLOCK_CLK_SRC                CLK_SRC_2KHZ
#define CLOCK_TICKS_PER_MS           ( CLK_SRC_2KHZ_SPEED / 1000 )
#define CLOCK_ACTUAL_TICKS_FACTOR    ( CLOCK_COUNT_DOWN_MS_PER_TICK *CLOCK_TICKS_PER_MS )
#define CLOCK_USER_TICK_TO_SYSTEM_TICK( user_ticks )   ( ( user_ticks ) * CLOCK_ACTUAL_TICKS_FACTOR )
#define CLOCK_SYSTEM_TICK_TO_USER_TICK( system_ticks )   ( ( system_ticks ) / CLOCK_ACTUAL_TICKS_FACTOR )

enum Clock_request_type {
	CLOCK_CURRENT_TICK,
	CLOCK_COUNT_DOWN,
	CLOCK_COUNT_DOWN_COMPLETE,
	CLOCK_QUIT             /* Only parent can send */
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

void clock_main()
{
	Clock clock_1 = {0};         /* For interrupt */
	// Clock clock_3 = {0};         /* For timer */
	Clock_request request;
	Clock_reply reply;
	int event_handler_tid = 0;
	int request_tid;
	int time_tid = MyParentTid();
	uint current_time = 0;
	uint current_cd = 0;
	uint event_handling = 0;
	int execute = 1;
	int status = 0;
	
	/* clk_reset to ensure the clock is fully reset */
	clk_enable( &clock_1, CLK_1, CLK_MODE_INTERRUPT, CLOCK_CLK_SRC, CLOCK_USER_TICK_TO_SYSTEM_TICK( 1 ) );
	clk_clear( &clock_1 );

	/* clk_enable( &clock_3, CLK_3, CLK_MODE_FREE_RUN, CLOCK_CLK_SRC, ~0 ); */
	/* clk_reset( &clock_3, ~0 ); */
	/* clk_clear( &clock_3 ); */

	event_handler_tid = Create_drv( 0, event_handler );
	assert( event_handler_tid > 0 );

	status = event_init( event_handler_tid, EVENT_SRC_TC1UI, clock_cd_complete );
	assert( status == ERR_NONE );

#ifdef IPC_MAGIC
	reply.magic = CLOCK_MAGIC;
#endif

	DEBUG_NOTICE( DBG_CLK_DRV, "launch\n" );

	event_always( event_handler_tid );

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

		/* Process request */
		switch( request.type ){
		case CLOCK_CURRENT_TICK:
			reply.data = current_time;
			break;
		case CLOCK_COUNT_DOWN:
			if( request.data < current_cd || current_cd == 0 ){
				current_cd = request.data;
			}
			break;
		case CLOCK_QUIT:
			status = event_quit( event_handler_tid );
			execute = 0;
			break;
		case CLOCK_COUNT_DOWN_COMPLETE:
			event_handling = 0;
			clk_clear( &clock_1 );
			current_time += 1;
			if( current_cd != 0 && current_time >= current_cd ){
				current_cd = 0;
				time_signal( time_tid );
				DEBUG_NOTICE( DBG_CLK_DRV, "cd notified\n" );
			}
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

	DEBUG_NOTICE( DBG_CLK_DRV, "About to send request\n" );
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
