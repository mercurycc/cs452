#ifndef _USER_DEVICES_UART_H_
#define _USER_DEVICES_UART_H_

#include <types.h>
#include <config.h>

/* Driver main */
int uart_init( uint port, uint speed, uint fifo, uint flow_ctrl, uint dbl_stop );
int uart_getc( int tid, uint* data );
int uart_putc( int tid, uint data );
int uart_quit( int tid );

#endif /* _USER_DEVICES_UART_H_ */
