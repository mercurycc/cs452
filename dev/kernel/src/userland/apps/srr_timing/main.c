#include <user/syscall.h>
#include <user/apps_entry.h>
#include <user/name_server.h>
#include <user/servers_entry.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <devices/clock.h>          /* Pretty hacky here, don't do this in a normal app */
#include <bwio.h>
#include <err.h>

static void srr_bench_sender()
{
	int status = 0;
	int tid = 0;
	uint cycle_count;
	char test_buffer[ 64 ];

	tid = WhoIs( "BenchRcv" );
	assert( tid > 0 );

	/* 64 bytes */
	
	status = Send( tid, test_buffer, 64, test_buffer, 64 );
	assert( tid > 0 );
	
	/* 4 bytes */
	
	status = Send( tid, test_buffer, 4, test_buffer, 4 );
	assert( tid > 0 );
	
}

static void srr_bench_receiver()
{
	int status = 0;
	int tid = 0;
	char test_buffer[ 64 ];
	
	status = RegisterAs( "BenchRcv" );
	assert( status == REGISTER_AS_SUCCESS );

	/* 64 bytes */
	status = Receive( &tid, test_buffer, 64 );
	assert( status == 64 );
	status = Reply( &tid, test_buffer, 64 );
	assert( status == 0 );

	/* 4 bytes */
	status = Receive( &tid, test_buffer, 4 );
	assert( status == 4 );
	status = Reply( &tid, test_buffer, 4 );
	assert( status == 0 );
}

void srr_bench()
{
	int tid;
	
	tid = Create( 2, srr_bench_sender );
	assert( tid > 0 );

	tid = Create( 2, srr_bench_receiver );
	assert( tid > 0 );

	Exit();
}
