#ifndef _STRING_H_
#define _STRING_H_

#include <types.h>
#include <lib/valist.h>

#define isalpha( ch )            ( ( ( ch >= 'a' ) && ( ch <= 'z' ) ) || ( ( ch >= 'A' ) && ( ch <= 'a' ) ) )
#define isdigit( ch )            ( ( ch >= '0' ) && ( ch <= '9' ) )
#define isspace( ch )            ( ( ch == '\n' ) || ( ch == '\r' ) || ( ch == ' ' ) || ( ch == '\t' ) )

/* Number-string conversion */
uint utos( uint num, char* str );
/* Fill the prefix with fill to form a string of length length */
uint utos_fill( uint num, char* buffer, uint length, uchar fill );
uint stou( const char* str );
uint itos( int num, char* str );
int stoi( const char* str );

uint strlen( const char* str );
int strcmp( const char* str1, const char* str2 );

int memcpy( uchar* dst, const uchar* src, uint size );

int sprintf( char* dst, const char* fmt, ... );
int sformat ( char* dst, const char *fmt, va_list va );

#endif
