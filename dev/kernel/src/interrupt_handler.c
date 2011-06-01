#include <types.h>
#include <context.h>
#include <interrupt.h>
#include <ts7200.h>
#include <devices/clock.h>
#include <err.h>
#include <regopts.h>

typedef int (*Interrupt_handler)( Context* ctx );

static int interrupt_handler_init()
{
	Clock clk_1 = {0};

	HW_WRITE( VIC1_BASE, VIC_INT_ENABLE_OFFSET, 1 << INTERRUPT_SRC_TC1UI );

	/* For testing */
	/* Enable clock interrupt from clk_0 */
	clk_enable( &clk_1, CLK_1, CLK_MODE_INTERRUPT, CLK_SRC_2KHZ, 4000 );

	return ERR_NONE;
}

void interrupt_test()
{
	/* Clear the interrupt */
	HW_WRITE( TIMER1_BASE, TIMER_CLR_OFFSET, 0 );
}

int interrupt_init( Context* ctx )
{
	uint* interrupt_addr = ( uint* )0x38;

	*interrupt_addr = (uint)interrupt_trap;

	interrupt_handler_init();

	return ERR_NONE;
}

