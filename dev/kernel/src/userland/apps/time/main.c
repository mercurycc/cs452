#include <bwio.h>
#include <err.h>
#include <types.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <user/name_server.h>
#include <user/time.h>
#include <user/syscall.h>
#include <devices/clock.h>
#include <user/devices/clock.h>
#include <user/assert.h>
#include <user/drivers_entry.h>
#include <user/servers_entry.h>

typedef struct Heap_node_s Heap_node;
typedef struct Heap_s Heap;

#define MAX_HEAP_SIZE 64

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

void swap_int( int *a, int *b ) {
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
		}
		else {
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
		if ( l <= heap->size ) {
			if (heap->data[i].key > heap->data[l].key) {
				swap_int( (int*)&(heap->data[i].value), (int*)&(heap->data[l].value) );
				swap_int( (int*)&(heap->data[i].key), (int*)&(heap->data[l].key) );
			}
		}
		else if ( r <= heap->size ){
			if (heap->data[i].key > heap->data[r].key) {
				swap_int( (int*)&(heap->data[r].value), (int*)&(heap->data[r].value) );
				swap_int( (int*)&(heap->data[r].key), (int*)&(heap->data[r].key) );
			}
		}
		else {
			i = heap->size;
		}
	}
	return ERR_HEAP_NONE;
}



struct Time_tid_node {
	uint time;
	int tid;
};



void time_main(){

	int tid;
	Time_request msg;
	Time_reply reply;
	int status;
	uint result = 0;
	int clock_tid;
	uint cur_time;
	int stop = 0;


	Heap heap;
	status = heap_init( &heap );
	assert( status == 0 );

	struct Time_tid_node node;

	
	clock_tid = Create( 0, clock_main );
	
	status = RegisterAs( "time" );
	assert( status == 0 );

	while ( !stop ) {
		status = Receive(&tid, (char*)&msg, sizeof(msg));
		assert( status == 0 );
		
		bwprintf ( COM2, "timer server received request 0x%x, interval %d\n", msg.request, msg.interval );
		
		switch ( msg.request ) {
		case TIME_ASK:
			status = clock_current_time( clock_tid, &result );
			assert( status == 0 );
			reply.result = result;
			status = Reply( tid, (char*)&reply, sizeof(reply));
			assert( status == sizeof(reply) );
			break;
		case TIME_DELAY:
			status = clock_current_time( clock_tid, &result );
			assert( status == 0 );

			if ( msg.interval <= 0 ) {
				reply.result = 0;
				status = Reply( clock_tid, (char*)&reply, sizeof(reply) );
				assert ( status == 0 );
				break;
			}

			node.time = result + msg.interval;
			node.tid = tid;

			heap_insert( &heap, (Heap_node*)&node );

			status = clock_count_down( clock_tid, node.time );
			assert( status == 0 );
			break;
		case TIME_SIGNAL:
			reply.result = 0;
			status = Reply( clock_tid, (char*)&reply, sizeof(reply) );
			assert( status == sizeof(reply) );

			// TODO find corresponding tid for the blocked task;
			heap_read_top( &heap, (Heap_node*)&node );
			cur_time = node.time;
			while ( node.time == cur_time ){
				status = Reply( node.tid, (char*)&reply, sizeof(reply) );
				assert( status == sizeof(reply) );
				status = heap_remove_top( &heap, (Heap_node*)&node );
				assert( status == 0 );
				heap_read_top( &heap, (Heap_node*)&node );
				assert( status == 0 );
			}
			
			status = clock_count_down( clock_tid, node.time );
			assert( status == 0 );
			break;
		default:
			bwprintf( COM2, "INVALID TIME MESSAGE" );
			break;
		}
	}

}

int time_request( int tid, uint request, int interval ){
	Time_request msg;
	Time_reply reply;

	msg.request = request;
	msg.interval = interval;

	Send( tid, (char*)&msg, sizeof(msg), (char*)&reply, sizeof(reply) );
	return reply.result;
}

int time_ask( int tid ){
	return time_request( tid, TIME_ASK, 0 );
}

int time_delay( int tid, int interval ){
	return time_request( tid, TIME_ASK, interval );
}

int time_signal( int tid ) {
	return time_request( tid, TIME_SIGNAL, 0 );
}



