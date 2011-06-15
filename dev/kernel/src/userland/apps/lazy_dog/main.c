#include <types.h>
#include <user/syscall.h>
#include <user/clock_server.h>
#include <user/lib/sync.h>
#include <user/assert.h>

#define LAZY_MAGIC         0x11a27a61

typedef struct Lazy_dog_param_s {
	uint magic;
	uint delay_time;
	uint delay_count;
} Lazy_dog_param;

void lazy_lazy_dog()
{
	int i;
	uint request = LAZY_MAGIC;
	int status = 0;
	int my_tid = MyTid();
	Lazy_dog_param param;

	status = Send( MyParentTid(), ( char* )&request, sizeof( request ), ( char* )&param, sizeof( param ) );
	assert( status == sizeof( param ) );

	bwprintf( COM2, "task %d starts\n", my_tid );
	for( i = 0; i < param.delay_count; i += 1 ){
		status = Delay( param.delay_time );
		assert( status == 0 );
		bwprintf( COM2, "task %d has finished delay %d of %d ms\n", my_tid, i+1, param.delay_time * 10 );
	}

	sync_responde( MyParentTid() );
	Exit();
}

void lazy_dog()
{
	int i;
	int request;
	int tid;
	int status;
	Lazy_dog_param param;

	param.magic = LAZY_MAGIC;

	for( i = 0; i < 4; i += 1 ){
		switch( i ){
		case 0:
			param.delay_time = 10;
			param.delay_count = 20;
			break;
		case 1:
			param.delay_time = 23;
			param.delay_count = 9;
			break;
		case 2:
			param.delay_time = 33;
			param.delay_count = 6;
			break;
		case 3:
			param.delay_time = 71;
			param.delay_count = 3;
			break;
		}

		tid = Create( 3 + i, lazy_lazy_dog );
		assert( tid > 0 );

		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		assert( status == sizeof( request ) );
		assert( request == LAZY_MAGIC );
		status = Reply( tid, ( char* )&param, sizeof( param ) );
		assert( status == SYSCALL_SUCCESS );
	}

	for( i = 0; i < 4; i += 1 ){
		sync_wait();
	}
	
	sync_responde( MyParentTid() );

	Exit();
}
