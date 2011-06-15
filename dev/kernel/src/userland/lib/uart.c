#include <user/devices/uart.h>
#include <user/uart.h>
#include <user/protocals.h>
#include <user/assert.h>
#include <err.h>

int Getc( int channel )
{
	int data;
	int status;
	
	switch( channel ){
	case COM_1:
		channel = UART1_DRV_TID;
		break;
	case COM_2:
		channel = UART2_DRV_TID;
		break;
	default:
		return GETC_CHANNEL_INVALID;
	}

	status = uart_getc( channel, ( uint* )&data );
	assert( status == ERR_NONE );

	return data;
}

int Putc( int channel, char ch )
{
	int status;
	
	switch( channel ){
	case COM_1:
		channel = UART1_DRV_TID;
		break;
	case COM_2:
		channel = UART2_DRV_TID;
		break;
	default:
		return PUTC_CHANNEL_INVALID;
	}

	status = uart_putc( channel, ( uint )ch );
	assert( status == ERR_NONE );

	return PUTC_SUCCESS;
}
	

