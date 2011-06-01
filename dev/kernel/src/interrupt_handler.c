#include <types.h>
#include <context.h>
#include <interrupt.h>
#include <ts7200.h>
#include <err.h>

typedef int (*Interrupt_handler)( Context* ctx );

static int interrupt_handler_init()
{
	
}

int interrupt_init( Context* ctx )
{
	uint* interrupt_addr = ( uint* )0x38;

	*interrupt_addr = (uint)interrupt_trap;

	return ERR_NONE;
}

