#include <types.h>
#include <config.h>
#include <err.h>
#include <console.h>
#include <bwio.h>  /* For convenience */
#include <ts7200.h>
#include <regopts.h>
#include <str.h>

#define CURSOR_CONTROL_ASSIGN_CMD( ch )				\
	do {							\
		cmdbuf[ cmdsize ] = ch;				\
		cmdsize += 1;					\
	} while( 0 )

static inline ptr console_getbase( Console* cons )
{
	ptr base = 0;
	
	switch( cons->channel ){
	case CONSOLE_1:
		base = UART1_BASE;
		break;
	case CONSOLE_2:
		base = UART2_BASE;
		break;
	}

	return base;
}

static inline int console_ready_read( Console* cons )
{
	ptr base = console_getbase( cons );
	volatile uint* flags = HW_ADDR( base, UART_FLAG_OFFSET );

	return ( ! ( *flags & RXFE_MASK ) );
}

static inline int console_ready_write( Console* cons )
{
	ptr base = console_getbase( cons );
	volatile uint* flags = HW_ADDR( base, UART_FLAG_OFFSET );

	return ( ! ( *flags & TXFF_MASK ) ) && ( ( ! cons->flow_control ) || ( *flags & CTS_MASK ) );
}

int console_setup( Console* cons, uint channel, uint speed, uint blocking, uint flow_control, uint dbl_stop )
{
	ASSERT( channel < CONSOLE_COUNT );

	cons->channel = channel;
	cons->speed = speed;
	cons->fifo = ON;
	cons->write_bytes = 0;
	cons->read_bytes = 0;
	cons->blocking = blocking;
	cons->flow_control = flow_control;
	cons->dbl_stop = dbl_stop;

	return ERR_NONE;
}

int console_init( Console* cons )
{
	ptr base = 0;
	int status = 0;
	uint channel = 0;
	uint control = 0;
	
	base = console_getbase( cons );

	channel = cons->channel == CONSOLE_1 ? COM1 : COM2;
	/* Reset port first */
	cons->write_bytes = 0;
	cons->read_bytes = 0;
	status = bwsetspeed( channel, cons->speed );

	control = HW_READ( base, UART_LCRH_OFFSET );
	
	if( cons->fifo ){
		control |= FEN_MASK;
	} else {
		control &= ( ~FEN_MASK );
	}
	if( cons->dbl_stop ){
		control |= STP2_MASK;
	}
	
	HW_WRITE( base, UART_LCRH_OFFSET, control );

	ASSERT( status == ERR_NONE );

	/* Leave no garbage */
	console_drain( cons );
	
	return ERR_NONE;
}

int console_deinit( Console* cons )
{
	return ERR_NONE;
}

/* Send one byte through the UART port.
   Set block to 1 to busy wait until ready bit is set.
*/
static inline int console_transmit( Console* cons, uchar data, uint blocking )
{
	ptr base = console_getbase( cons );

	do {
		if( console_ready_write( cons ) ){
			HW_WRITE( base, UART_DATA_OFFSET, data );
			cons->write_bytes += 1;
			break;
		} else if( blocking ){
			continue;
		} else {
			return ERR_CONSOLE_NOT_READY;
		}
	} while( 1 );

	return ERR_NONE;
}

/* Receive one byte through the UART port.
   Set block to 1 to busy wait until ready bit is set.
*/
static inline int console_receive( Console* cons, uchar* data, uint blocking )
{
	ptr base = console_getbase( cons );

	do {
		if( console_ready_read( cons ) ){
			*data = (uchar)HW_READ( base, UART_DATA_OFFSET );
			cons->read_bytes += 1;
			break;
		} else if( blocking ){
			continue;
		} else {
			return ERR_CONSOLE_NOT_READY;
		}
	} while( 1 );

	return ERR_NONE;
}

int console_write( Console* cons, const uchar* data, uint size )
{
	uint i = 0;
	int status = 0;

	for( i = 0; i < size; i += 1 ) {
		status = console_transmit( cons, data[ i ], cons->blocking );
		if( status == ERR_CONSOLE_NOT_READY ){
			break;
		}
	}

	cons->last_write = i;

	return ERR_NONE;
}

int console_read( Console* cons, uchar* buffer, uint size )
{
	uint i = 0;
	int status = 0;
	
	for( i = 0; i < size; i += 1 ) {
		status = console_receive( cons, buffer + i, cons->blocking );
		if( status == ERR_CONSOLE_NOT_READY ){
			break;
		}
	}

	cons->last_read = i;

	return ERR_NONE;
}

int console_drain( Console* cons )
{
	uchar ch;
	
	while( console_receive( cons, &ch, 0 ) != ERR_CONSOLE_NOT_READY );

	return ERR_NONE;
}

int console_write_guarantee( Console* cons, const uchar* data, uint size )
{
	uint complete = 0;
	int status = 0;

	while( complete < size ) {
		status = console_write( cons, data + complete, size );
		if( status != ERR_NONE ){
			return status;
		}
		complete += cons->last_write;
	}

	return ERR_NONE;
}

int console_read_guarantee( Console* cons, uchar* data, uint size )
{
	uint complete = 0;
	int status = 0;

	while( complete < size ) {
		status = console_read( cons, data + complete, size );
		if( status != ERR_NONE ){
			return status;
		}
		complete += cons->last_read;
	}

	return ERR_NONE;
}

int console_putc( Console* cons, char c )
{
	return console_write( cons, (uchar*)&c, 1 );
}

int console_getc( Console* cons, char* c )
{
	uchar ch;
	int status;
	
	status = console_read_guarantee( cons, &ch, 1 );
	ASSERT( status == ERR_NONE );

	status = console_write_guarantee( cons, &ch, 1 );

	*c = ch;
	
	return ERR_NONE;
}

int console_printf( Console* cons, char* format, ... )
{
	return ERR_NONE;
}

int console_scanf( Console* cons, char* format, ... )
{
	return ERR_NONE;
}

enum { CURSOR_CONTROL_SAVE,
       CURSOR_CONTROL_RESTORE,
       CURSOR_CONTROL_SET_POS,
       CURSOR_CONTROL_REMOVE_LEFT,
       CURSOR_CONTROL_CLS,
       CURSOR_CONTROL_BACK,
       CURSOR_CONTROL_INSERT,
       CURSOR_CONTROL_COUNT };

static int console_ansi_cursor_control( Console* cons, uint op, uint op1, uint op2, uint op3 )
{
	char cmdbuf[ 32 ] = {0};
	int cmdsize = 0;
	int status = 0;

	CURSOR_CONTROL_ASSIGN_CMD( 27 );

	switch( op ){
		/* ESC group */
	case CURSOR_CONTROL_SAVE:
		CURSOR_CONTROL_ASSIGN_CMD( '7' );
		break;
	case CURSOR_CONTROL_RESTORE:
		CURSOR_CONTROL_ASSIGN_CMD( '8' );
		break;
		
		/* CSI group */
	case CURSOR_CONTROL_SET_POS:
	case CURSOR_CONTROL_CLS:
	case CURSOR_CONTROL_REMOVE_LEFT:
		CURSOR_CONTROL_ASSIGN_CMD( '[' );
		break;
	default:
		return ERR_INVALID_ARGUMENT;
	}

	/* CSI extra parameters */
	switch( op ){
	case CURSOR_CONTROL_CLS:
		CURSOR_CONTROL_ASSIGN_CMD( '2' );
		CURSOR_CONTROL_ASSIGN_CMD( 'J' );
		break;
	case CURSOR_CONTROL_REMOVE_LEFT:
		cmdsize += utos( op1, cmdbuf + cmdsize );
		CURSOR_CONTROL_ASSIGN_CMD( 'X' );
		break;
	case CURSOR_CONTROL_BACK:
		cmdsize += utos( op1, cmdbuf + cmdsize );
		CURSOR_CONTROL_ASSIGN_CMD( 'D' );
		break;
	case CURSOR_CONTROL_INSERT:
		cmdsize += utos( op1, cmdbuf + cmdsize );
		CURSOR_CONTROL_ASSIGN_CMD( '@' );
		break;
		/* CLS 2 parameters */
	case CURSOR_CONTROL_SET_POS:
		cmdsize += utos( op1, cmdbuf + cmdsize );
		CURSOR_CONTROL_ASSIGN_CMD( ';' );
		cmdsize += utos( op2, cmdbuf + cmdsize );
		CURSOR_CONTROL_ASSIGN_CMD( 'H' );
		break;
	}

	/* Send command to terminal */
	status = console_write_guarantee( cons, ( uchar* )cmdbuf, cmdsize );
	ASSERT( status == ERR_NONE );

	while( ! console_ready_write( cons ) );
	
	return ERR_NONE;	
}

int console_cls( Console* cons )
{
	return console_ansi_cursor_control( cons, CURSOR_CONTROL_CLS, 0, 0, 0 );
}

int console_cursor_save( Console* cons )
{
	return console_ansi_cursor_control( cons, CURSOR_CONTROL_SAVE, 0, 0, 0 );
}

int console_cursor_setposn( Console* cons, int row, int col )
{
	return console_ansi_cursor_control( cons, CURSOR_CONTROL_SET_POS, row, col, 0 );
}

int console_cursor_restore( Console* cons )
{
	return console_ansi_cursor_control( cons, CURSOR_CONTROL_RESTORE, 0, 0, 0 );
}

int console_cursor_remove( Console* cons, uint count )
{
	uint i = 0;
	uchar ch = '\b';
	int status = 0;

	/* Can be done with iobuf + flush */
	for( i = 0; i < count; i += 1 ){
		status = console_write_guarantee( cons, &ch, 1 );
		ASSERT( status == ERR_NONE );
	}

	ch = ' ';

	for( i = 0; i < count; i += 1 ){
		status = console_write_guarantee( cons, &ch, 1 );
		ASSERT( status == ERR_NONE );
	}

	ch = '\b';
	for( i = 0; i < count; i += 1 ){
		status = console_write_guarantee( cons, &ch, 1 );
		ASSERT( status == ERR_NONE );
	}
	
	return ERR_NONE;
}
