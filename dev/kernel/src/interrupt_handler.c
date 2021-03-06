#include <types.h>
#include <context.h>
#include <interrupt.h>
#include <ts7200.h>
#include <devices/clock.h>
#include <err.h>
#include <regopts.h>
#include <task.h>

// TODO: VIC2 is not currently used

static inline void interrupt_init_one( uint base, uint priority, uint interrupt_id )
{
	HW_WRITE( base, VIC_VECT_CNTL_OFFSET_OF( priority ), VIC_VECT_CNTL_ENABLE | interrupt_id );
	HW_WRITE( base, VIC_VECT_ADDR_OFFSET_OF( priority ), interrupt_id );
}

static int interrupt_handler_init()
{
	/* VIC 1 */
	/* Clock 1 */
	interrupt_init_one( VIC1_BASE, 0, INTERRUPT_SRC_TC1UI );

	/* COM1 */
	interrupt_init_one( VIC1_BASE, 1, INTERRUPT_SRC_UART1RXINTR1 );
	interrupt_init_one( VIC1_BASE, 2, INTERRUPT_SRC_UART1TXINTR1 );
	
	/* COM2 */
	interrupt_init_one( VIC1_BASE, 3, INTERRUPT_SRC_UART2RXINTR2 );
	interrupt_init_one( VIC1_BASE, 4, INTERRUPT_SRC_UART2TXINTR2 );

	/* VIC 2 */
	/* COM */
	/* General interrupt must be lower priority than RX/TX */
	interrupt_init_one( VIC2_BASE, 0, INTERRUPT_SRC_INT_UART1 );
	interrupt_init_one( VIC2_BASE, 1, INTERRUPT_SRC_INT_UART2 );

	/* Reset VIC priority hardware */
	HW_WRITE( VIC1_BASE, VIC_VECT_ADDR_OFFSET, 0 );
	HW_WRITE( VIC2_BASE, VIC_VECT_ADDR_OFFSET, 0 );

	return ERR_NONE;
}

static inline ptr interrupt_get_vic_base( uint interrupt_id )
{
	if( interrupt_id > 31 ){
		return VIC2_BASE;
	} else {
		return VIC1_BASE;
	}
}

static inline int interrupt_disable( uint interrupt_id )
{
	ptr base = interrupt_get_vic_base( interrupt_id );

	/* EP9302 has 2 VICs, each one handle 32 interrupts */
	HW_WRITE( base, VIC_INT_EN_CLEAR_OFFSET, 1 << ( interrupt_id % 32 ) );
	
	return ERR_NONE;
}

static inline int interrupt_enable( uint interrupt_id )
{
	ptr base = interrupt_get_vic_base( interrupt_id );

	/* EP9302 has 2 VICs, each one handle 32 interrupts */
	HW_WRITE( base, VIC_INT_ENABLE_OFFSET, 1 << ( interrupt_id % 32 ) );
	
	return ERR_NONE;
}

int interrupt_deinit( Context* ctx )
{
	/* Disable all interrupts */
	HW_WRITE( VIC1_BASE, VIC_INT_EN_CLEAR_OFFSET, ~0 );
	HW_WRITE( VIC2_BASE, VIC_INT_EN_CLEAR_OFFSET, ~0 );

	return ERR_NONE;
}

int interrupt_init( Context* ctx )
{
	uint* interrupt_addr = ( uint* )0x38;
	int status = 0;

	/* Install IRQ exception entry */
	*interrupt_addr = (uint)interrupt_trap;

	status = interrupt_deinit( ctx );
	ASSERT( status == ERR_NONE );

	/* Set all interrupts to generate IRQ */
	HW_WRITE( VIC1_BASE, VIC_INT_SELECT_OFFSET, 0 );
	HW_WRITE( VIC2_BASE, VIC_INT_SELECT_OFFSET, 0 );
	
	/* Install all interrupt handlers */
	status = interrupt_handler_init();
	ASSERT( status == ERR_NONE );

	return ERR_NONE;
}

int interrupt_register( Context* ctx, Interrupt_mgr* int_mgr, Task* event_waiter, uint interrupt_id )
{
	int status = 0;
	Task** interrupt_handlers = int_mgr->interrupt_handlers;

	if( interrupt_id >= INTERRUPT_COUNT ){
		return ERR_INTERRUPT_INVALID_INTERRUPT;
	} else if( interrupt_handlers[ interrupt_id ] ){
		return ERR_INTERRUPT_ALREADY_REGISTERED;
	} else {
		interrupt_handlers[ interrupt_id ] = event_waiter;
		DEBUG_PRINT( DBG_INT, "task %d registerd as handler %d\n", event_waiter->tid, interrupt_id );
	}

	status = interrupt_enable( interrupt_id );
	ASSERT( status == ERR_NONE );
	
	return ERR_NONE;
}

int interrupt_handle( Context* ctx, Interrupt_mgr* int_mgr, Task** event_delivered )
{
	uint interrupt_id = 0;
	int status = 0;
	Task** interrupt_handlers = int_mgr->interrupt_handlers;

	interrupt_id = HW_READ( VIC1_BASE, VIC_VECT_ADDR_OFFSET );
	DEBUG_PRINT( DBG_INT, "handler read: %d\n", interrupt_id );

	status = interrupt_disable( interrupt_id );
	ASSERT( status == ERR_NONE );

	*event_delivered = interrupt_handlers[ interrupt_id ];

	ASSERT_M( event_delivered, "interrupt %d does not have a handler!\n", interrupt_id );
	
	interrupt_handlers[ interrupt_id ] = 0;

	/* End vectored ISR */
	HW_WRITE( VIC1_BASE, VIC_VECT_ADDR_OFFSET, 0 );

	return ERR_NONE;
}
