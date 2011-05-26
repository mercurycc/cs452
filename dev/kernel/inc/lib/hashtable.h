#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <types.h>

struct Hashtable_s {
	uint size;
	uint strlen;
	char** key;
	ptr* elem;
};

int hashtable_init( Hashtable* hashtable, char** key_table, ptr* elem_table, uint table_size, uint maxstrlen );
int hashtable_insert( Hashtable* hashtable, char* str, ptr elem );
int hashtable_find( Hashtable* hashtable, char* str, ptr* elem );
int hashtable_remove( Hashtable* hashtable, char* str );

#endif

