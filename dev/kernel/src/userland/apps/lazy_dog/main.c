#include <types.h>
#include <user/syscall.h>
#include <user/time.h>
#include <user/sync.h>
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
	Lazy_dog_param param;

	status = Send( MyParentTid(), ( char* )&request, sizeof( request ), ( char* )&param, sizeof( param ) );
	assert( status == sizeof( param ) );

	for( i = 0; i < param.delay_count; i += 1 ){
		status = Delay( param.delay_time );
		assert( status == 0 );
	}

	sync_responde( MyParentTid() );
}

void lazy_dog()
{
	int i;
	int request;
	int tid;
	Lazy_dog_param param;

	param.magic = LAZY_MAGIC;

	for( i = 0; i < 4; i += 1 ){
		switch( i ){
			/* Pinball uses 10 ms as a tick, so the delay
			   specified on the assignment need to be
			   modified */
		case 0:
			param.delay_time = 50;
			param.delay_count = 20;
			break;
		case 1:
			param.delay_time = 23 * 5;
			param.delay_count = 9;
			break;
		case 2:
			param.delay_time = 33 * 5;
			param.delay_count = 6;
			break;
		case 3:
			param.delay_time = 71 * 5;
			param.delay_count = 3;
			break;
		}

		tid = Create( 3 + i, lazy_lazy_dog );
		assert( tid > 0 );

		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		assert( status == sizeof( request ) );
		assert( request == LAZY_MAGIC );
		statis = Reply( tid, ( char* )&lazy_lazy_dog, sizeof( lazy_lazy_dog ) );
		assert( status == SYSCALL_SUCCESS );
	}

	for( i = 0; i < 4; i += 1 ){
		sync_wait();
	}
	
	sync_responde( MyParentTid() );

	Exit();
}