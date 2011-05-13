#include <types.h>
#include <err.h>
#include <bwio.h>
#include <ts7200.h>
#include <clock.h>
#include <regopts.h>

static uint clkLastVal[ CLK_COUNT ] = { 0 };

/* Obtain the base pointer to the specific clock */
static inline ptr clk_getbase( uint clk )
{
	ptr base = 0;

	ASSERT( clk < CLK_COUNT );
	
	switch( clk ){
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

static inline int clk_enable_direct( uint clk, uint ctrl )
{
	ptr base = clk_getbase( clk );

	HW_WRITE( base, TIMER_CTRL_OFFSET, ctrl );

	return ERR_NONE;
}

int clk_enable( uint clk, uint mode, uint clksrc, uint initial )
{
	ptr base = clk_getbase( clk );
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
	clkLastVal[ clk ] = initial;

	clk_enable_direct( clk, control );
	
	return ERR_NONE;
}

int clk_disable( uint clk )
{
	ptr base = clk_getbase( clk );

	HW_WRITE( base, TIMER_CTRL_OFFSET, 0 );

	clkLastVal[ clk ] = 0;

	return ERR_NONE;
}

int clk_value( uint clk, uint* val )
{
	ptr base = clk_getbase( clk );

	ASSERT( val );

	clkLastVal[ clk ] = HW_READ( base, TIMER_VAL_OFFSET );
	*val = clkLastVal[ clk ];

	// DEBUG_PRINT( "read clk %u\n", *val );

	return ERR_NONE;
}

int clk_diff_cycles( uint clk, uint* val )
{
	uint lastVal = clkLastVal[ clk ];
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

int clk_speed( uint clk, uint* speed )
{
	ptr base = clk_getbase( clk );
	uint srcSel = 0;
	uint clkSpeed = 0;

	srcSel = HW_READ( base, TIMER_CTRL_OFFSET ) & TIMER_CLKSEL_MASK;

	switch( srcSel ){
	case 0:
		// SEL unset, 2kHz clock
		clkSpeed = CLK_SRC_2KHZ_SPEED;
		break;
	case TIMER_CLKSEL_MASK:
		// SEL set, 508 kHz clock
		clkSpeed = CLK_SRC_508KHZ_SPEED;
		break;
	}

	*speed = clkSpeed;
	
	return ERR_NONE;
}

int clk_reset( uint clk, uint initial )
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

	return ERR_NONE;
}

int clk_clear( uint clk )
{
	ptr base = clk_getbase( clk );

	HW_WRITE( base, TIMER_CLR_OFFSET, 1 );

	return ERR_NONE;
}
