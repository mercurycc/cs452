#include <types.h>
#include <bwio.h>


void abort_handle( uint pc )
{
	bwprintf( COM2, "FATAL ERROR: Prefech, Illegal instruction, or Data Abort\n" );
	bwprintf( COM2, "Faulting pc (corrected by sub -4): 0x%x\n", pc );
}

void abort_trace_begin( int fp )
{
	bwprintf( COM2, "* Trace begin with fp 0x%x *\n", fp );
}

void abort_trace_print( uint lr, uint fp )
{
	bwprintf( COM2, "< 0x%x (0x%x)", lr, fp );
}

void abort_trace_complete()
{
	bwprintf( COM2, "\n* Trace end *\n" );
}

void abort_trigger()
{
	int* data = 0xf0000000;

	*data = 0;
}
		
