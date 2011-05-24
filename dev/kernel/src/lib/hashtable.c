#include <types.h>
#include <err.h>
#include <lib/str.h>
#include <lib/hashtable.h>

uint hash( char* str ){
	// djb hash
	uint h = 5381;
	char* c;
	for ( c = str; *c; c += 1 ) {
		h = ( ( h << 5 ) + h ) + *c;
	}
	return h;
}

uint hashtable_init( Hashtable* hashtable, char** key_table, ptr* elem_table, uint table_size ){
	hashtable->size = table_size;
	hashtable->key = key_table;
	hashtable->elem = elem_table;
	int i = 0;
	for ( i = 0; i < hashtable->size; i++ ){
		hashtable->key[i] = 0;
		hashtable->elem[i] = 0;
	}
	return ERR_NONE;
}

uint hashtable_insert( Hashtable* hashtable, char* str, ptr elem ){
	uint hash_value = hash( str ) % ( hashtable->size );
	uint i = hash_value;
	while ( hashtable->elem[i] ) {
		i = ( i + 1 ) % hashtable->size;
		if ( i == hash_value )
			return ERR_HASHTABLE_FULL;
	}
	hashtable->key[i] = str;
	hashtable->elem[i] = elem;
	return ERR_NONE;
}

uint hashtable_find( Hashtable* hashtable, char* str, ptr* elem ){
	uint hash_value = hash( str ) % ( hashtable->size );
	uint i = hash_value;
	while ( !strcmp( hashtable->key[i], str )  ) {
		i = ( i + 1 ) % hashtable->size;
		if ( ( i == hash_value ) || ( hashtable->key[i] == 0 ) )
			return ERR_HASHTABLE_NOTFOUND;
	}
	*elem = hashtable->elem[i];
	return ERR_NONE;
}

uint hashtable_remove( Hashtable* hashtable, char* str ){
	uint hash_value = hash( str ) % ( hashtable->size );
	uint i = hash_value;
	while ( !strcmp( hashtable->key[i], str )  ) {
		i = ( i + 1 ) % hashtable->size;
		if ( ( i == hash_value ) || ( hashtable->key[i] == 0 ) )
			return ERR_HASHTABLE_NOTFOUND;
	}
	hashtable->key[i] = 0;
	hashtable->elem[i] = 0;
	return ERR_NONE;
}

