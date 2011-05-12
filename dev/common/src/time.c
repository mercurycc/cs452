#include <types.h>
#include <err.h>
#include <time.h>
#include <clock.h>

#define UP_CYCLE_MAX 0xffffffff

static uint upCycleWrap;
static uint upCycle;
static uint clkSpeed;

int time_init()
{
	int status = 0;

	upCycleWrap = 0;
	upCycle = 0;
	status = clk_enable( TIME_USE_CLK, TIME_CLK_MODE, TIME_CLK_SRC, 0xffffffff );
	ASSERT( status == ERR_NONE );

	status = clk_speed( TIME_USE_CLK, &clkSpeed );
	ASSERT( status == ERR_NONE );

	return ERR_NONE;
}

int time_current( uint* wrap, time_t* current )
{
	uint currentDiff = 0;
	int status;
	
	status = clk_diff_cycles( TIME_USE_CLK, &currentDiff );
	ASSERT( status == ERR_NONE );
	
	if( UP_CYCLE_MAX - upCycle < currentDiff ){
		upCycleWrap += 1;
		upCycle = currentDiff - ( UP_CYCLE_MAX - upCycle );
	} else {
		upCycle += currentDiff;
	}

	/* TODO: implement wrap */
		
	*wrap = 0;
	*current = upCycle * 1000 / clkSpeed;

	return ERR_NONE;
}

int time_timer( time_t last, uint duration )
{
	uint wrap = 0;
	time_t currentUp = 0;
	int status;

	status = time_current( &wrap, &currentUp );
	ASSERT( status == ERR_NONE );

	if( last + duration < currentUp ){
		return 1;
	}

	return 0;
}

int time_convert( Timespec* out )
{
	uint wrap = 0;
	time_t currentUp = 0;
	int status = 0;

	status = time_current( &wrap, &currentUp );
	ASSERT( status == ERR_NONE );

	/* TODO: implement wrap */
	out->msec = ( currentUp % 1000 ) / 10;
	currentUp /= 1000;
	out->sec = currentUp % 60;
	currentUp /= 60;
	out->min = currentUp % 60;
	currentUp /= 60;
	out->hour = currentUp % 24;
	currentUp /= 24;
	out->day = currentUp;

	return ERR_NONE;
}
