#ifndef _USER_LIB_HEAP_H_
#define _USER_LIB_HEAP_H_

typedef struct Heap_node_s Heap_node;
typedef struct Heap_s Heap;

#define MAX_HEAP_SIZE 256

enum HEAP_ERR {
	ERR_HEAP_NONE,
	ERR_HEAP_FULL,
	ERR_HEAP_EMPTY
};

struct Heap_node_s {
	uint key;
	int value;
};

struct Heap_s {
	Heap_node data[MAX_HEAP_SIZE];
	int size;
};

int heap_init( Heap* heap );
int heap_insert( Heap* heap, Heap_node* elem );
int heap_read_top( Heap* heap, Heap_node* elem );	//peak
int heap_remove_top ( Heap* heap, Heap_node* elem );

#endif
