#include <types.h>
#include <err.h>
#include <lib/str.h>
#include <lib/hashtable.h>

static inline uint hash( char* str ){
	// djb hash
	uint h = 5381;
	char* c;
	for ( c = str; *c; c += 1 ) {
		h = ( ( h << 5 ) + h ) + *c;
	}
	return h;
}

int hashtable_init( Hashtable* hashtable, char** key_table, ptr* elem_table, uint table_size, uint maxstrlen ){
	hashtable->size = table_size;
	hashtable->key = key_table;
	hashtable->elem = elem_table;
	hashtable->strlen = maxstrlen;
	int i = 0;
	for ( i = 0; i < hashtable->size; i++ ){
		hashtable->key[i][0] = 0;
		hashtable->elem[i] = 0;
	}
	return ERR_NONE;
}

int hashtable_insert( Hashtable* hashtable, char* str, ptr elem ){
	if ( strlen(str) > hashtable->strlen ){
		return ERR_HASHTABLE_OVERLENGTH;
	}
	uint hash_value = hash( str ) % ( hashtable->size );
	uint i = hash_value;
	while ( hashtable->elem[i] ) {
		i = ( i + 1 ) % hashtable->size;
		if ( i == hash_value )
			return ERR_HASHTABLE_FULL;
	}
	memcpy( ( uchar* )hashtable->key[i], ( uchar* )str, strlen(str) );
	hashtable->elem[i] = elem;
	return ERR_NONE;
}

int hashtable_find( Hashtable* hashtable, char* str, ptr* elem ){
	DEBUG_NOTICE( DBG_HASH, "called\n" );
	if ( strlen(str) > hashtable->strlen ){
		return ERR_HASHTABLE_OVERLENGTH;
	}
	uint hash_value = hash( str ) % ( hashtable->size );
	uint i = hash_value;

	DEBUG_PRINT( DBG_HASH, "str: %s, hash: %u\n", str, hash_value );
	
	while ( strcmp( hashtable->key[i], str )  ) {
		i = ( i + 1 ) % hashtable->size;
		if ( ( i == hash_value ) || ( hashtable->key[i] == 0 ) )
			return ERR_HASHTABLE_NOTFOUND;
	}
	*elem = hashtable->elem[i];
	return ERR_NONE;
}

int hashtable_remove( Hashtable* hashtable, char* str ){
	if ( strlen(str) > hashtable->strlen ){
		return ERR_HASHTABLE_OVERLENGTH;
	}
	uint hash_value = hash( str ) % ( hashtable->size );
	uint i = hash_value;
	while ( strcmp( hashtable->key[i], str )  ) {
		i = ( i + 1 ) % hashtable->size;
		if ( ( i == hash_value ) || ( hashtable->key[i] == 0 ) )
			return ERR_HASHTABLE_NOTFOUND;
	}
	hashtable->key[i][0] = 0;
	hashtable->elem[i] = 0;
	return ERR_NONE;
}
