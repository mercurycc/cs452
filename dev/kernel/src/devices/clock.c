#include <types.h>
#include <err.h>
#include <context.h>
#include <bwio.h>
#include <ts7200.h>
#include <devices/clock.h>
#include <regopts.h>

/* Obtain the base pointer to the specific clock */
static inline ptr clk_getbase( Clock* clk )
{
	ptr base = 0;

	ASSERT( clk->clk_id < CLK_COUNT );
	
	switch( clk->clk_id ){
	case CLK_1:
		base = TIMER1_BASE;
		break;
	case CLK_2:
		base = TIMER2_BASE;
		break;
	case CLK_3:
		base = TIMER3_BASE;
		break;
	}

	return base;
}

static inline int clk_enable_direct( Clock* clk, uint ctrl )
{
	ptr base = clk_getbase( clk );

	HW_WRITE( base, TIMER_CTRL_OFFSET, ctrl );

	return ERR_NONE;
}

int clk_enable( Clock* clk, uint clk_id, uint mode, uint clksrc, uint initial )
{
	ptr base = 0;
	uint control = 0;

	ASSERT( mode < CLK_MODE_COUNT &&
		clksrc < CLK_SRC_COUNT );

	base = clk_getbase( clk );
	control = TIMER_ENABLE_MASK;

	switch( mode ){
	case CLK_MODE_FREE_RUN:
		// Default
		break;
	case CLK_MODE_INTERRUPT:
		control |= TIMER_MODE_MASK;
		break;
	}

	switch( clksrc ){
	case CLK_SRC_2KHZ:
		// Default
		break;
	case CLK_SRC_508KHZ:
		control |= TIMER_CLKSEL_MASK;
		break;
	}
	
	HW_WRITE( base, TIMER_LDR_OFFSET, initial );

	/* Initialize clock book keeping */
	clk->clk_id = clk_id;
	clk->mode = mode;
	clk->clk_src = clksrc;
	clk->last_value = initial;

	clk_enable_direct( clk, control );
	
	return ERR_NONE;
}

int clk_disable( Clock* clk )
{
	ptr base = clk_getbase( clk );

	HW_WRITE( base, TIMER_CTRL_OFFSET, 0 );

	clk->last_value = 0;

	return ERR_NONE;
}

int clk_value( Clock* clk, uint* val )
{
	ptr base = clk_getbase( clk );

	ASSERT( val );

	clk->last_value = HW_READ( base, TIMER_VAL_OFFSET );
	*val = clk->last_value;

	// DEBUG_PRINT( "read clk %u\n", *val );

	return ERR_NONE;
}

int clk_diff_cycles( Clock* clk, uint* val )
{
	uint lastVal = clk->last_value;
	uint newVal = 0;
	int status;

	ASSERT( val );

	status = clk_value( clk, &newVal );
	ASSERT( status == ERR_NONE );

	if( newVal < lastVal ){
		/* This might not work */
		*val = lastVal - newVal;
	} else {
		*val = newVal - lastVal;
	}

	return ERR_NONE;
}

int clk_speed( Clock* clk, uint* speed )
{
	uint clkSpeed = 0;

	switch( clk->clk_src ){
	case CLK_SRC_2KHZ:
		// SEL unset, 2kHz clock
		clkSpeed = CLK_SRC_2KHZ_SPEED;
		break;
	case CLK_SRC_508KHZ:
		// SEL set, 508 kHz clock
		clkSpeed = CLK_SRC_508KHZ_SPEED;
		break;
	}

	*speed = clkSpeed;
	
	return ERR_NONE;
}

int clk_reset( Clock* clk, uint initial )
{
	ptr base = clk_getbase( clk );
	uint ctrlVal = 0;
	int status;

	ctrlVal = HW_READ( base, TIMER_CTRL_OFFSET );

	status = clk_disable( clk );
	ASSERT( status == ERR_NONE );

	HW_WRITE( base, TIMER_LDR_OFFSET, initial );
	
	status = clk_enable_direct( clk, ctrlVal );
	ASSERT( status == ERR_NONE );

	clk->last_value = initial;

	return ERR_NONE;
}

int clk_clear( Clock* clk )
{
	ptr base = clk_getbase( clk );

	HW_WRITE( base, TIMER_CLR_OFFSET, 0 );

	return ERR_NONE;
}
