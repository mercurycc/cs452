#ifndef _USER_EVENT_H_
#define _USER_EVENT_H_

#include <types.h>

enum Events {
	EVENT_SRC_COMMRX           = 2,
	EVENT_SRC_COMMTX           = 3,
	EVENT_SRC_TC1UI            = 4,
	EVENT_SRC_TC2UI            = 5,
	EVENT_SRC_AACINTR          = 6,
	EVENT_SRC_DMAM2P0          = 7,
	EVENT_SRC_DMAM2P1          = 8,
	EVENT_SRC_DMAM2P2          = 9,
	EVENT_SRC_DMAM2P3         = 10,
	EVENT_SRC_DMAM2P4         = 11,
	EVENT_SRC_DMAM2P5         = 12,
	EVENT_SRC_DMAM2P6         = 13,
	EVENT_SRC_DMAM2P7         = 14,
	EVENT_SRC_DMAM2P8         = 15,
	EVENT_SRC_DMAM2P9         = 16,
	EVENT_SRC_DMAM2M0         = 17,
	EVENT_SRC_DMAM2M1         = 18,
	EVENT_SRC_UART1RXINTR1    = 23,
	EVENT_SRC_UART1TXINTR1    = 24,
	EVENT_SRC_UART2RXINTR2    = 25,
	EVENT_SRC_UART2TXINTR2    = 26,
	EVENT_SRC_UART3RXINTR3    = 27,
	EVENT_SRC_UART3TXINTR3    = 28,
	EVENT_SRC_INT_KEY         = 29,
	EVENT_SRC_INT_TOUCH       = 30,
	EVENT_SRC_INT_EXT0        = 32,
	EVENT_SRC_INT_EXT1        = 33,
	EVENT_SRC_INT_EXT2        = 34,
	EVENT_SRC_TINTR           = 35,
	EVENT_SRC_WEINT           = 36,
	EVENT_SRC_INT_RTC         = 37,
	EVENT_SRC_INT_IRDA        = 38,
	EVENT_SRC_INT_MAC         = 39,
	EVENT_SRC_INT_PROG        = 41,
	EVENT_SRC_CLK1HZ          = 42,
	EVENT_SRC_V_SYNC          = 43,
	EVENT_SRC_INT_VIDEO_FIFO  = 44,
	EVENT_SRC_INT_SSP1RX      = 45,
	EVENT_SRC_INT_SSP1TX      = 46,
	EVENT_SRC_TC3UI           = 51,
	EVENT_SRC_INT_UART1       = 52,
	EVENT_SRC_SSPINTR         = 53,
	EVENT_SRC_INT_UART2       = 54,
	EVENT_SRC_INT_UART3       = 55,
	EVENT_SRC_USHINTR         = 56,
	EVENT_SRC_INT_PME         = 57,
	EVENT_SRC_INT_DSP         = 58,
	EVENT_SRC_GPIOINTR        = 59,
	EVENT_SRC_I2SINTR         = 60
};

/* tid should always be MyParentTid.  The callback should be designed that way. */
typedef int (*Event_callback)( int tid );

void event_handler();
int event_init( int tid, Event_callback callback );
int event_start( int tid );
int event_quit( int tid );


#endif /* _USER_EVENT_H_ */
