#include <types.h>
#include <bwio.h>


void abort_handle( uint pc )
{
	bwprintf( COM2, "FATAL ERROR: Prefech, Illegal instruction, or Data Abort\n" );
	bwprintf( COM2, "Faulting pc: 0x%x\n", pc );
	bwprintf( COM2, "Waiting for watchdog...\n" );

	while( 1 );
}
