#include <user/syscall.h>
#include <user/assert.h>
#include <ts7200.h>
#include <types.h>
#include <config.h>
#include <user/protocals.h>
#include <user/event.h>
#include <err.h>
#include <lib/rbuf.h>
#include <regopts.h>

#define UART_MAGIC       0x11a87017

typedef struct Uart_init_s Uart_init;
typedef struct Uart_request_s Uart_request;
typedef struct Uart_reply_s Uart_reply;

struct Uart_init_s {
#ifdef IPC_MAGIC
	uint magic;
#endif
	uint port;
	uint flow_ctrl;
	uint fifo;
	uint speed;
};

struct Uart_request_s {
#ifdef IPC_MAGIC
	uint magic;
#endif
	uint type;
	uint data;
};

struct Uart_reply_s {
#ifdef IPC_MAGIC
	uint magic;
#endif
	uint data;
};

enum Uart_request_type {
	UART_GET,
	UART_PUT,
	UART_TXRDY,
	UART_RXRDY,
	UART_GENERAL_INT,
	UART_QUIT
};

static int uart_txrdy( int tid );
static int uart_rxrdy( int tid );
static int uart_general( int tid );

static inline ptr uart_getbase( uint port )
{
	ptr base = 0;
	
	switch( port ){
	case UART_1:
		base = UART1_BASE;
		break;
	case UART_2:
		base = UART2_BASE;
		break;
	}

	return base;
}

#define uart_ready_read( flags )	( ! ( ( *flags ) & RXFE_MASK ) )
#define uart_ready_write( flags )	( ! ( ( *flags ) & TXFF_MASK ) )
#define uart_ready_cts( flags )         ( ( *flags ) & CTS_MASK )

static inline void uart_receive_config( Uart_init* config )
{
	Uart_reply reply;
	int tid;
	int status;

#ifdef IPC_MAGIC
	reply.magic = UART_MAGIC;
#endif
	status = Receive( &tid, ( char* )config, sizeof( Uart_init ) );
	assert( status == sizeof( Uart_init ) );
#ifdef IPC_MAGIC
	assert( config->magic == UART_MAGIC );
#endif

	status = Reply( tid, ( char* )&reply, sizeof( reply ) );
	assert( status == SYSCALL_SUCCESS );
}

static inline void uart_send_config( int tid, Uart_init* config )
{
	Uart_reply reply;
	int status;

	status = Send( tid, ( char* )config, sizeof( Uart_init ), ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );
#ifdef IPC_MAGIC
	assert( reply.magic == UART_MAGIC );
#endif
}

static inline void uart_receive_request( int* tid, Uart_request* request )
{
	int status;
	
	status = Receive( tid, ( char* )request, sizeof( Uart_request ) );
	assert( status == sizeof( Uart_request ) );
#ifdef IPC_MAGIC
	assert( request->magic == UART_MAGIC );
#endif
}

static inline void uart_set_interrupt( volatile uint* uart_ctrl, uint intr, uint on )
{
	if( on ){
		*uart_ctrl |= intr;
	} else {
		*uart_ctrl &= ( ~intr );
	}
}

static inline void uart_config_interrupt( Uart_init* config, volatile uint* uart_ctrl, uint type, uint state )
{
	uint control = 0;
	
	switch( type ){
	case UART_RXRDY:
		if( config->fifo ){
			control |= RTIEN_MASK;
		}
		control |= RIEN_MASK;
		break;
	case UART_TXRDY:
		/* Not used.  Finer control is needed so this is done inside the main thread */
		assert( 0 );
		if( config->flow_ctrl ){
			control |= MSIEN_MASK;
		}
		control |= TIEN_MASK;
	}

	uart_set_interrupt( uart_ctrl, control, state );
}

void uart_driver()
{
	ptr base;
	Uart_init config;
	Uart_request request;
	Uart_reply reply;
	int tid;
	int status;
	int txrdy_waiting = 0;
	int txrdy_handler_tid;
	int rxrdy_waiting = 0;
	int rxrdy_handler_tid;
	int general_waiting = 0;
	int general_handler_tid;
	int read_tid = 0;
	int can_reply;
	volatile uint* data;
	volatile uint* flags;
	volatile uint* intr;
	volatile uint* ctrl;
	Rbuf txbufbody = {0};
	Rbuf* txbuf = &txbufbody;
	uint txbuf_buffer[ UART_TXBUF_SIZE ];

	/* Receive config */
	uart_receive_config( &config );

	assert( MyTid() == ( ( config.port == UART_1 ) ? UART1_DRV_TID : UART2_DRV_TID ) );

	base = uart_getbase( config.port );

	data = ( uint* )HW_ADDR( base, UART_DATA_OFFSET );
	flags = ( uint* )HW_ADDR( base, UART_FLAG_OFFSET );
	intr = ( uint* )HW_ADDR( base, UART_INTR_OFFSET );
	ctrl = ( uint* )HW_ADDR( base, UART_CTRL_OFFSET );

	/* Initialized interrupt handler */
	txrdy_handler_tid = Create( 0, event_handler );
	assert( txrdy_handler_tid > 0 );
	status = event_init( txrdy_handler_tid, ( config.port == UART_1 ) ? EVENT_SRC_UART1TXINTR1 : EVENT_SRC_UART2TXINTR2, uart_txrdy );
	assert( status == ERR_NONE );
	
	rxrdy_handler_tid = Create( 0, event_handler );
	assert( rxrdy_handler_tid > 0 );
	status = event_init( rxrdy_handler_tid, ( config.port == UART_1 ) ? EVENT_SRC_UART1RXINTR1 : EVENT_SRC_UART2RXINTR2, uart_rxrdy );
	assert( status == ERR_NONE );
	
	general_handler_tid = Create( 0, event_handler );
	assert( general_handler_tid > 0 );
	status = event_init( general_handler_tid, ( config.port == UART_1 ) ? EVENT_SRC_INT_UART1 : EVENT_SRC_INT_UART2, uart_general );
	assert( status == ERR_NONE );

#ifdef IPC_MAGIC
	reply.magic = UART_MAGIC;
#endif

	/* Initialize txbuf */
	status = rbuf_init( txbuf, ( uchar* )txbuf_buffer, sizeof( uint ), UART_TXBUF_SIZE );
	assert( status == ERR_NONE );

	while( 1 ){
		/* By definition there should only be one task asking for input from any serial port */
		uart_receive_request( &tid, &request );

		DEBUG_PRINT( DBG_UART, "Received request: type %d, data 0x%x\n", request.type, request.data );

		can_reply = 0;

		/* Immediately reply if received interrupt notification to release event handler,
		   or if received put request */
		switch( request.type ){
		case UART_RXRDY:
			rxrdy_waiting = 0;
			break;
		case UART_TXRDY:
			txrdy_waiting = 0;
			break;
		case UART_GENERAL_INT:
			general_waiting = 0;
			break;
		default:
			break;
		}

		switch( request.type ){
		case UART_RXRDY:
		case UART_TXRDY:
		case UART_GENERAL_INT:
		case UART_PUT:
			status = Reply( tid, ( char* )&reply, sizeof( reply ) );
			tid = 0;
		default:
			break;
		}

		/* Process request, or translate request to proper interrupt */
		switch( request.type ){
		case UART_GET:
			if( read_tid ){
				assert( 0 );
			}
			read_tid = tid;
			request.type = UART_RXRDY;
			break;
		case UART_PUT:
			status = rbuf_put( txbuf, ( uchar* )&request.data );
			assert( status == ERR_NONE );
			request.type = UART_TXRDY;
			break;
		case UART_GENERAL_INT:
			if( ( *intr & MIS_MASK ) || ( *intr & TIS_MASK ) ){
				request.type = UART_TXRDY;
				
				/* Clear MIS */
				*intr = 0;
			} else if( ( *intr & RTIS_MASK ) || ( *intr & RIS_MASK ) ){
				request.type = UART_RXRDY;
			} else {
				assert( 0 );
			}
		default:
			break;
		}

		switch( request.type ){
		case UART_RXRDY:
			/* There is a race condition: since we are reading data in the user space driver, what if the
			   data here is already overwritten?  The solution to this problem is to wish that we will
			   eventually only need to turn fifo on */
			/* Disable interrupt */
			if( uart_ready_read( flags ) ){
				DEBUG_NOTICE( DBG_UART, "read ready\n" );
				/* Disable interrupt */
				uart_config_interrupt( &config, ctrl, UART_RXRDY, 0 );
				reply.data = *data;
				tid = read_tid;
				read_tid = 0;
				can_reply = 1;
			} else {
				DEBUG_NOTICE( DBG_UART, "read NOT ready\n" );
				/* Enable interrupt */
				uart_config_interrupt( &config, ctrl, UART_RXRDY, 1 );
				if( ! rxrdy_waiting ){
					DEBUG_NOTICE( DBG_UART, "waiting for rx interrupt\n" );
					uart_set_interrupt( ctrl, RIEN_MASK, 1 );
					event_start( rxrdy_handler_tid );
					rxrdy_waiting = 1;
				}
			}
			break;
		case UART_TXRDY:
			if( ! uart_ready_write( flags ) ){
				/* Enable txrdy interrupt */
				if( ! txrdy_waiting ){
					uart_set_interrupt( ctrl, TIEN_MASK, 1 );
					event_start( txrdy_handler_tid );
					txrdy_waiting = 1;
				}
			} else {
				/* txrdy has to be disabled otherwise it could be triggered many times
				   while we are actually waiting on CTS */
				uart_set_interrupt( ctrl, TIEN_MASK, 0 );
				
				if( config.flow_ctrl && ( ! uart_ready_cts( flags ) ) ){
					/* Enable MSI */
					uart_set_interrupt( ctrl, MSIEN_MASK, 1 );
				} else {
					/* Disable interrupt */
					uart_set_interrupt( ctrl, MSIEN_MASK, 0 );
					if( ! rbuf_empty( txbuf ) ){
						/* Transfer */
						status = rbuf_get( txbuf, ( uchar* )data );
						assert( status == ERR_NONE );
					}
				}
			}
			
			break;
		default:
			break;
		}

		if( ! general_waiting ){
			event_start( general_handler_tid );
			general_waiting = 1;
		}

		if( request.type == UART_QUIT ){
			/* Not implemented yet, given general event handler implementation, we need to generate an interrupt to wake up general_handler */
			assert( 0 );
			break;
		}

		if( can_reply ){
			status = Reply( tid, ( char* )&reply, sizeof( reply ) );
			assert( status == SYSCALL_SUCCESS );
		}

		
	}

	Exit();
}

int uart_init( uint port, uint speed, uint fifo, uint flow_ctrl, uint dbl_stop )
{
	ptr base = uart_getbase( port );
	Uart_init init;
	volatile uint* line_ctrl_high = ( uint* )HW_ADDR( base, UART_LCRH_OFFSET );
	volatile uint* line_ctrl_medium = ( uint* )HW_ADDR( base, UART_LCRM_OFFSET );
	volatile uint* line_ctrl_low = ( uint* )HW_ADDR( base, UART_LCRL_OFFSET );
	volatile uint* uart_ctrl = ( uint* )HW_ADDR( base, UART_CTRL_OFFSET );

#ifdef IPC_MAGIC
	init.magic = UART_MAGIC;
#endif
	init.port = port;
	init.speed = speed;
	init.fifo = fifo;
	init.flow_ctrl = flow_ctrl;

	if( fifo ){
		*line_ctrl_high |= FEN_MASK;
	} else {
		*line_ctrl_high &= ( ~FEN_MASK );
	}
	
	if( dbl_stop ){
		*line_ctrl_high |= STP2_MASK;
	}

	switch( speed ) {
	case 115200:
		*line_ctrl_medium = 0x0;
		*line_ctrl_low = 0x3;
		break;
	case 2400:
		*line_ctrl_medium = 0x0;
		*line_ctrl_low = 191;
		break;
	default:
		assert( 0 );
		break;
	}

	/* Turn off interrupt */
	uart_set_interrupt( uart_ctrl, MSIEN_MASK | RIEN_MASK | TIEN_MASK | RTIEN_MASK, 0 );

	uart_send_config( ( ( port == UART_1 ) ? UART1_DRV_TID : UART2_DRV_TID ), &init );
	
	return ERR_NONE;
}

static int uart_request( int tid, uint type, uint* data )
{
	Uart_request request;
	Uart_reply reply;
	int status;

#ifdef IPC_MAGIC
	request.magic = UART_MAGIC;
#endif
	request.type = type;
	request.data = *data;

	status = Send( tid, ( char* )&request, sizeof( request ), ( char* )&reply, sizeof( reply ) );
	assert( status == sizeof( reply ) );
#ifdef IPC_MAGIC
	assert( request.magic == UART_MAGIC );
#endif

	*data = reply.data;

	return ERR_NONE;
}

int uart_getc( int tid, uint* data )
{
	return uart_request( tid, UART_GET, data );
}

int uart_putc( int tid, uint data )
{
	return uart_request( tid, UART_PUT, &data );
}

int uart_quit( int tid )
{
	uint data;
	return uart_request( tid, UART_QUIT, &data );
}

/* Interrupt notifiers */
static int uart_txrdy( int tid )
{
	uint data;
	return uart_request( tid, UART_TXRDY, &data );
}

static int uart_rxrdy( int tid )
{
	uint data;
	return uart_request( tid, UART_RXRDY, &data );
}

static int uart_general( int tid )
{
	uint data;
	return uart_request( tid, UART_GENERAL_INT, &data );
}
