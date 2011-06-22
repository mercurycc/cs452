#include <types.h>
#include <err.h>
#include <config.h>
#include <lib/str.h>

/* Modified from bwio bwprintf implementation */
char sc2x( char ch )
{
	if ( (ch <= 9) ) return '0' + ch;
	return 'a' + ch - 10;
}

static char* sputw( char* dst, int n, char fc, char *bf )
{
	char ch;
	char *p = bf;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) *(dst++) = fc;
	while( ( ch = *bf++ ) ) *(dst++) = ch;

	return dst;
}

static void sui2a( unsigned int num, unsigned int base, char *bf )
{
	int n = 0;
	int dgt;
	unsigned int d = 1;
	
	while( (num / d) >= base ) d *= base;
	while( d != 0 ) {
		dgt = num / d;
		num %= d;
		d /= base;
		if( n || dgt > 0 || d == 0 ) {
			*bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
			++n;
		}
	}
	*bf = 0;
}

static int sa2d( char ch ) {
	if( ch >= '0' && ch <= '9' ) return ch - '0';
	if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
	if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
	return -1;
}

static char sa2i( char ch, const char **src, int base, int *nump ) {
	int num, digit;
	const char *p;

	p = *src; num = 0;
	while( ( digit = sa2d( ch ) ) >= 0 ) {
		if ( digit > base ) break;
		num = num*base + digit;
		ch = *(p++);
	}
	*src = p; *nump = num;
	return ch;
}

static void si2a( int num, char *bf )
{
	if( num < 0 ) {
		num = -num;
		*bf++ = '-';
	}
	sui2a( num, 10, bf );
}

int sformat ( char* dst, const char *fmt, va_list va )
{
	char* orig = dst;
	char bf[12];
	char ch, lz;
	int w;
	
	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' ){
			*(dst++) = ch;
		} else {
			lz = 0; w = 0;
			ch = *(fmt++);
			if( ch == '0' ){
				lz = '0'; ch = *(fmt++);
			}
			switch ( ch ) {
			case '0':
				lz = 0;
				ch = *(fmt++);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				ch = sa2i( ch, &fmt, 10, &w );
				break;
			}
			switch( ch ) {
			case 0:
				fmt--;
				break;
			case 'c':
				*(dst++) = va_arg( va, char );
				break;
			case 's':
				dst = sputw( dst, w, 0, va_arg( va, char* ) );
				break;
			case 'u':
				sui2a( va_arg( va, unsigned int ), 10, bf );
				dst = sputw( dst, w, lz, bf );
				break;
			case 'd':
				si2a( va_arg( va, int ), bf );
				dst = sputw( dst, w, lz, bf );
				break;
			case 'x':
				sui2a( va_arg( va, unsigned int ), 16, bf );
				dst = sputw( dst, w, lz, bf );
				break;
			case '%':
				*(dst++) = ch;
				break;
			}
		}
	}

	*(dst++) = '\0';

	return dst - orig;
}

int sprintf( char* dst, const char *fmt, ... )
{
        va_list va;
	int size;

        va_start(va,fmt);
        size = sformat( dst, fmt, va );
        va_end(va);

	return size;
}
