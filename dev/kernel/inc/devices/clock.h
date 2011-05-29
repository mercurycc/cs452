#ifndef _DEVICE_CLOCK_H_
#define _DEVICE_CLOCK_H_

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

struct Clock_s {
	uint clk_id;
	uint mode;
	uint clk_src;
	uint last_value;
};

/* In hz */
#define CLK_SRC_508KHZ_SPEED     508000
#define CLK_SRC_2KHZ_SPEED       2000

/* Clock operations */

/* Enable the clock */
int clk_enable( Clock* clk, uint clk_id, uint mode, uint clksrc, uint initial );

/* Disable the clock */
int clk_disable( Clock* clk );

/* Read the current clock value */
int clk_value( Clock* clk, uint* val );

/* Read the difference of clock cycles between clock reads */
int clk_diff_cycles( Clock* clk, uint* val );

/* Report the speed the clock is running at in Hz*/
int clk_speed( Clock* clk, uint* speed );

/* Reset the clock load register with new initial value */
int clk_reset( Clock* clk, uint initial );

/* Clear the interrupt supplied by the timer */
int clk_clear( Clock* clk );

#endif /* _DEVICE_CLOCK_H_ */
