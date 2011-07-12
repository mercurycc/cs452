#include <types.h>
#include <user/lib/heap.h>

static inline void swap_int( int *a, int *b ) {
	int temp = *a;
	*a = *b;
	*b = temp;
}

int heap_init( Heap* heap ){
	heap->size = 0;
	return ERR_HEAP_NONE;
}

int heap_insert( Heap* heap, Heap_node* elem ){
	if ( heap->size >= MAX_HEAP_SIZE ) {
		return ERR_HEAP_FULL;
	}

	(heap->data[heap->size]).key = elem->key;
	(heap->data[heap->size]).value = elem->value;

	int i = heap->size;
	int p;
	while (i) {
		p = (i - 1) / 2;
		if ( heap->data[i].key < heap->data[p].key ) {
			swap_int( (int*)&(heap->data[i].key), (int*)&(heap->data[p].key) );
			swap_int( (int*)&(heap->data[i].value), (int*)&(heap->data[p].value) );
			i = p;
		} else {
			i = 0;
		}
	}
	heap->size++;
	return ERR_HEAP_NONE;
}


int heap_read_top( Heap* heap, Heap_node* elem ){
	if ( heap->size == 0 ) {
		return ERR_HEAP_EMPTY;
	}

	elem->key = (heap->data[0]).key;
	elem->value = (heap->data[0]).value;
	return ERR_HEAP_NONE;
}

int heap_remove_top ( Heap* heap, Heap_node* elem ){
	int status = heap_read_top( heap, elem );
	if ( status != ERR_HEAP_NONE ){
		return status;
	}

	heap->size -= 1;
	if ( heap->size == 0 ) {
		return ERR_HEAP_NONE;
	}
	swap_int( (int*)&(heap->data[0].value), (int*)&(heap->data[heap->size].value) );
	swap_int( (int*)&(heap->data[0].key), (int*)&(heap->data[heap->size].key) );

	int i = 0;
	int l,r;
	while ( i < heap->size ) {
		l = i*2 + 1;
		r = i*2 + 2;
		
		if ( r < heap->size ){
			if (heap->data[l].key < heap->data[r].key) {
				swap_int( (int*)&(heap->data[i].value), (int*)&(heap->data[l].value) );
				swap_int( (int*)&(heap->data[i].key), (int*)&(heap->data[l].key) );
				i = l;
			}
            else if (heap->data[l].key > heap->data[r].key) {
				swap_int( (int*)&(heap->data[i].value), (int*)&(heap->data[r].value) );
				swap_int( (int*)&(heap->data[i].key), (int*)&(heap->data[r].key) );
				i = r;
			}
			else {
			     i = heap->size;
			}
		}
		else if ( l < heap->size ) {
			if (heap->data[i].key > heap->data[l].key) {
				swap_int( (int*)&(heap->data[i].value), (int*)&(heap->data[l].value) );
				swap_int( (int*)&(heap->data[i].key), (int*)&(heap->data[l].key) );
				i = l;
			}
			else {
			     i = heap->size;
			}
		}
		else {
			i = heap->size;
		}
	}
	return ERR_HEAP_NONE;
}
