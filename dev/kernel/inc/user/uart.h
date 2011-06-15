#ifndef _USER_UART_H_
#define _USER_UART_H_

#include <types.h>

enum Uart_errors {
	/* Getc */
	GETC_CHANNEL_INVALID = -1,
	/* Putc */
	PUTC_SUCCESS = 0,
	PUTC_CHANNEL_INVALID = -1
};

enum Uart_channels {
	COM_1,
	COM_2,
};

int Getc( int channel );
int Putc( int channel, char ch );

#endif /* _USER_UART_H_ */
