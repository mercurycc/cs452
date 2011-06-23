#include <types.h>
#include <lib/str.h>
#include <err.h>
#include <user/lib/parser.h>
#include <user/assert.h>

int parse_init( Parse* parse, char* str, char delim )
{
	assert( str );
	assert( parse );
	
	parse->str = str;
	parse->end = str + strlen( str );
	parse->token = 0;
	parse->delim = delim;

	return ERR_NONE;
}

int parse_token( Parse* parse )
{
	assert( parse );

	if( parse->str >= parse->end ){
		parse->token = 0;
		return ERR_NONE;
	}

	if( ! parse->delim ){
		parse->delim = ' ';
	}

	/* Skip all delimiters */
	while( parse->str[0] && parse->str[0] == parse->delim ){
		parse->str += 1;
	}

	/* Now we should either be hitting a null or a char */
	parse->token = parse->str;

	/* If we hit null, then return nothing */
	if( ! parse->token[0] ){
		parse->token = 0;
	}

	/* Set the next delimiter to null */
	while( parse->str[0] && parse->str[0] != parse->delim ){
		parse->str += 1;
	}
	parse->str[0] = 0;

	parse->str += 1;

	return ERR_NONE;
}

int parse_token_fill( Parse* parse, char** tokens, int size, int* filled )
{
	int i;

	for( i = 0; i < size; i += 1 ){
		parse_token( parse );
		if( parse->token ){
			tokens[ i ] = parse->token;
		} else {
			break;
		}
	}

	*filled = i;

	return ERR_NONE;
}

int parse_fill( char* str, char delim, char** tokens, int size, int* filled )
{
	Parse parse;

	parse_init( &parse, str, delim );

	return parse_token_fill( &parse, tokens, size, filled );
}
