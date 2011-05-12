 /*
 * iotest.c
 */

#include <bwio.h>
#include <ts7200.h>

int main( int argc, char* argv[] ) {
	char str[] = "Hello\n\r";
	int i = 0;
	
	bwsetfifo( COM2, OFF );
	bwputstr( COM2, str );
	bwputw( COM2, 10, '*', str );
	bwprintf( COM2, "Ze Long's Hello world.\n\r" );
	bwprintf( COM2, "%s world%u.\n\r", "Well, hello", 23 );
	bwprintf( COM2, "%d worlds for %u person.\n\r", -23, 1 );
	bwprintf( COM2, "%x worlds for %d people.\n\r", -23, 723 );
	
	bwsetfifo( COM1, ON );
	bwsetspeed( COM1, 2400 );
	bwprintf( COM1, "%c%c%c", 14, 20, 0x60 );
	bwprintf( COM1, "%c%c", 0x21, 0 );

	for( i = 0; i < 1024; i += 1 );

	bwprintf( COM1, "%c%c", 0x22, 0 );

	str[0] = bwgetc( COM2 );
	bwprintf( COM2, "%s", str );

	return 0;
}

