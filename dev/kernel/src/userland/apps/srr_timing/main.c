#include <user/syscall.h>
#include <user/apps_entry.h>
#include <user/name_server.h>
#include <user/servers_entry.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <devices/clock.h>          /* Pretty hacky here, don't do this in a normal app */
#include <context.h>                /* So damn hacky, I can't go to heaven */
#include <bwio.h>
#include <err.h>

static void srr_bench_sender()
{
	int status = 0;
	int tid = 0;
	uint cycle_count;
	char test_buffer[ 64 ];
	Context* ctx;
	Clock* clk;

	tid = WhoIs( "BenchRcv" );
	assert( tid > 0 );

	/* Obtain kernel context */
	KernelContext( (void**)&ctx );

	clk = ctx->timer_clk;

	/* 64 bytes */
	/* Update clock value */
	clk_value( clk, &cycle_count );
	status = Send( tid, test_buffer, 64, test_buffer, 64 );
	clk_diff_cycles( clk, &cycle_count );
	assert( tid > 0 );

	bwprintf( COM2, "64 bytes Send took %u ticks, which is equivalent to %u ms\n", cycle_count, cycle_count / 2 );
	
	/* 4 bytes */
	/* Update clock value */
	clk_value( clk, &cycle_count );
	status = Send( tid, test_buffer, 4, test_buffer, 4 );
	clk_diff_cycles( clk, &cycle_count );
	assert( tid > 0 );

	bwprintf( COM2, "4 bytes Send took %u ticks, which is equivalent to %u ms\n", cycle_count, cycle_count / 2 );

	/* Signal parent */
	status = Send( MyParentTid(), ( char* )&tid, sizeof( tid ), ( char* )&tid, sizeof( tid ) );
	assert( status == sizeof( tid ) );

	Exit();
}

static void srr_bench_receiver()
{
	int status = 0;
	int tid = 0;
	char test_buffer[ 64 ];
	
	status = RegisterAs( "BenchRcv" );
	assert( status == REGISTER_AS_SUCCESS );

	/* 64 bytes */
	status = Receive( &tid, (char*)test_buffer, 64 );
	assert( status == 64 );
	status = Reply( tid, (char*)test_buffer, 64 );
	assert( status == 0 );

	/* 4 bytes */
	status = Receive( &tid, (char*)test_buffer, 4 );
	assert( status == 4 );
	status = Reply( tid, (char*)test_buffer, 4 );
	assert( status == 0 );

	Exit();
}

void srr_bench()
{
	int tid,tid0;
	int status = 0;
	
	tid = Create( 2, srr_bench_sender );
	assert( tid > 0 );

	tid = Create( 2, srr_bench_receiver );
	assert( tid > 0 );

	/* Synchronize with srr_bench_sender */
	status = Receive( &tid, ( char* )&tid0, sizeof( tid0 ) );
	assert( status == sizeof( tid ) );
	status = Reply( tid, ( char* )&tid, sizeof( tid ) );
	assert( status == SYSCALL_SUCCESS );

	/* Signal parent */
	status = Send( MyParentTid(), ( char* )&tid, sizeof( tid ), ( char* )&tid, sizeof( tid ) );
	assert( status == sizeof( tid ) );

	Exit();
}
