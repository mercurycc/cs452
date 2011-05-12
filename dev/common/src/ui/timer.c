#include <types.h>
#include <config.h>
#include <err.h>
#include <ui/ui.h>
#include <ui/timer.h>
#include <regionio.h>
#include <console.h>
#include <bufio.h>
#include <str.h>

static Iobuf iobody;
static Iobuf* iobuf;
static Region consbody;
static Region* cons;
static uchar buffersbody[ UI_TIME_WIDTH * 2 ];
static uchar* buffers[ 2 ];

int ui_timer_init( Console* outputCons )
{
	int status = 0;
	
	iobuf = &iobody;	
	status = iobuf_init( iobuf, outputCons );
	ASSERT( status == ERR_NONE );

	buffers[ 0 ] = buffersbody;
	buffers[ 1 ] = buffersbody + UI_TIME_WIDTH;

	cons = &consbody;
	status = region_setup( cons, 0, UI_TIME_COLUMN, UI_TIME_ROW, UI_TIME_WIDTH, UI_TIME_HEIGHT, UI_TIME_HORIZONTAL_MARGIN, UI_TIME_VERTICAL_MARGIN, UI_TIME_BOUNDRY, iobuf, buffers );
	ASSERT( status == ERR_NONE );

	status = region_init( cons );
	ASSERT( status == ERR_NONE );

	DEBUG_PRINT( DBG_TIMER, "%s: Done\n", __func__ );

	return ERR_NONE;
}

int ui_timer_update()
{
	static Timespec last = { 0 };
	Timespec current = { 0 };
	uchar timeOut[] = "DD:HH:MM:SS:MS";
	uint length = 0;
	int status = 0;

	status = time_convert( &current );
	ASSERT( status == ERR_NONE );

#define UI_TIMER_ASSIGN( field, index )		\
	do {					\
		length = utos( current.field, (char*)(timeOut + 3 * index) ); \
		if( length == 1 ){					\
			timeOut[ 3 * index + 1 ] = timeOut[ 3 * index ]; \
			timeOut[ 3 * index ] = '0';			\
		}							\
		timeOut[ 3 * index + 2 ] = ':';				\
	} while( 0 )

	last = current;
	UI_TIMER_ASSIGN( day, 0 );
	UI_TIMER_ASSIGN( hour, 1 );
	UI_TIMER_ASSIGN( min, 2 );
	UI_TIMER_ASSIGN( sec, 3 );
	UI_TIMER_ASSIGN( msec, 4 );
	timeOut[ 14 ] = '\0';

	region_write( cons, ( char* )timeOut );
	region_flush( cons );
	
	return ERR_NONE;
}



