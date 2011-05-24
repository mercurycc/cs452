#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <types.h>

struct Hashtable_s {
	uint size;
	char** key;
	ptr* elem;
};

uint hash( char* str );

uint hashtable_init( Hashtable* hashtable, char** key_table, ptr* elem_table, uint table_size );
uint hashtable_insert( Hashtable* hashtable, char* str, ptr elem );
uint hashtable_find( Hashtable* hashtable, char* str, ptr* elem );
uint hashtable_remove( Hashtable* hashtable, char* str );

#endif

