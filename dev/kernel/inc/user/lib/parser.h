/* Simple parser */
/* The parser will mutate the input string, so caveat emptor */

#ifndef _USER_PARSER_H_
#define _USER_PARSER_H_
#include <types.h>

typedef struct Parse_s Parse;
struct Parse_s {
	char* str;
	char delim;             /* Delimiter, space by default */
	char* token;
	char* end;
};

int parse_init( Parse* parse, char* str, char delim );
int parse_token( Parse* parse );
int parse_token_fill( Parse* parse, char** tokens, int size, int* filled );
int parse_fill( char* str, char delim, char** tokens, int size, int* filled );

#endif /* _USER_PARSER_H_ */
