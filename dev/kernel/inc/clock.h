#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <types.h>
#include <ts7200.h>

/* Enumeration of all clocks */
/* There are 3 clocks on EP9302 */
enum ClockEnum { CLK_1,
		 CLK_2,
		 CLK_3,
		 CLK_COUNT };

/* Clock modes */
enum ClockMode { CLK_MODE_FREE_RUN,
		 CLK_MODE_INTERRUPT,
		 CLK_MODE_COUNT };

/* Clock source */
enum ClockSrc { CLK_SRC_508KHZ,
		CLK_SRC_2KHZ,
		CLK_SRC_COUNT };

static const uint CLK_SRC_508KHZ_SPEED = 508000;
static const uint CLK_SRC_2KHZ_SPEED = 2000;

/* Clock operations */

/* Enable the clock */
int clk_enable( uint clk, uint mode, uint clksrc, uint initial );

/* Disable the clock */
int clk_disable( uint clk );

/* Read the current clock value */
int clk_value( uint clk, uint* val );

/* Read the difference of clock cycles between clock reads */
int clk_diff_cycles( uint clk, uint* val );

/* Report the speed the clock is running at in Hz*/
int clk_speed( uint clk, uint* speed );

/* Reset the clock load register with new initial value */
int clk_reset( uint clk, uint initial );

/* Clear the interrupt supplied by the timer */
int clk_clear( uint clk );

#endif /* _TS7200_CLOCK_H_ */
