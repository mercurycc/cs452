#include <types.h>
#include <err.h>
#include <str.h>

uint utos( uint num, char* str )
{
	char conv[ 32 ];
	int i = 0;
	int j = 0;

	ASSERT( str );

	for( i = 0; 1; i += 1 ){
		conv[ i ] = num % 10;
		num /= 10;
		conv[ i ] += '0';
		if( ! num ){
			break;
		}
	}

	for( j = 0; i >= 0; j += 1, i -= 1 ){
		str[ j ] = conv[ i ];
	}

	str[ j ] = '\0';

	return j;
}

uint utos_fill( uint num, char* buffer, uint length, uchar fill )
{
	char conv[ 32 ];
	uint count = utos( num, conv );
	int diff = length;
	int i, j;

	diff -= count;
	ASSERT( diff >= 0 );

	for( i = 0; i < diff; i += 1 ){
		buffer[ i ] = fill;
	}

	for( j = 0; i < length; i += 1, j += 1 ){
		buffer[ i ] = conv[ j ];
	}

	buffer[ i ] = '\0';

	return i;
}

uint stou( const char* str )
{
	uint i = 0;
	uint res = 0;

	for( i = 0; str[ i ]; i += 1 ){
		res *= 10;
		res += str[ i ] - '0';
	}
	
	return res;
}

uint itos( int num, char* str )
{
	return 0;
}

int stoi( const char* str )
{
	return 0;
}

uint strlen( const char* str )
{
	uint i = 0;

	ASSERT( str );
	
	for( i = 0; str[ i ]; i += 1 );

	return i;
}

int strcmp( const char* str1, const char* str2 )
{
	ASSERT( str1 && str2 );
	
	while( *str1 && *str2 && *str1 == *str2 ){
		str1 += 1;
		str2 += 1;
	}

	if( *str1 || *str2 ){
		return 1;
	} else {
		return 0;
	}
}

int memcpy( uchar* dst, const uchar* src, uint size )
{
	uint i;
	
	ASSERT( dst && src );
	
	for( i = 0; i < size; i += 1 ){
		*dst = *src;
		dst++;
		src++;
	}

	return ERR_NONE;
}
