#include <user/syscall.h>
#include <user/apps_entry.h>
#include <user/servers_entry.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <user/drivers_entry.h>
#include <user/devices/clock.h>
#include <user/time.h>
#include <bwio.h>
#include <err.h>

static inline void K2()
{
	int tid;
	unsigned int buf;
	int status;

	/* For Kernel 2, launch the RPS game */
	tid = Create( 2, rps_game );
	assert( tid >= 0 );

	/* Synchronize with rps_game */
	status = Receive( &tid, ( char* )&buf, sizeof( buf ) );
	assert( status == sizeof( buf ) );
	status = Reply( tid, ( char* )&buf, sizeof( buf ) );
	assert( status == SYSCALL_SUCCESS );
}

static inline void clock_test( int clock_tid )
{
	uint time;

	clock_current_time( clock_tid, &time );
	bwprintf( COM2, "Received time %u\n", time );

	clock_count_down( clock_tid, 2000 );
	clock_count_down( clock_tid, 1000 );

	while( 1 ){
		clock_current_time( clock_tid, &time );
		if( time > 1500 && time < 2000 ){
			clock_count_down( clock_tid, 2000 );
		} else if( time > 2000 ){
			bwprintf( COM2, "Reached time %u\n", time );
			break;
		}
	}

	/* Expected: a interrupt should be generated after 2 seconds, rather than 10 seconds. */
}


static inline void time_test(){
	int tid = Create( 0, time_main );
	DEBUG_NOTICE( DBG_USER, "time server created\n" );

	int status;
/*
	status = time_ask( tid );
	bwprintf( COM2, "current time is 0x%x\n", status );

	int i;
	for ( i=0; i<10000; i++);
	status = time_ask( tid );
	bwprintf( COM2, "current time is 0x%x\n", status );

	status = time_delay( tid, 10 );
	assert( status == 0 );
*/
	status = time_ask( tid );
	bwprintf( COM2, "current time is 0x%x\n", status );

	status = time_suicide( tid );
	assert( status == 0 );
	DEBUG_NOTICE( DBG_USER, "time server killed\n" );

}


void user_init()
{
	int tid;
	int clock_tid;
	int status;
	
	tid = Create( 0, name_server_start );
	assert( tid > 0 );

	DEBUG_NOTICE( DBG_USER, "name server created\n" );
	
	/* Create device drivers 
	clock_tid = Create( 0, clock_main );
	assert( clock_tid > 0 );
	
	DEBUG_NOTICE( DBG_USER, "clock server created\n" );
	*/
//	clock_test( clock_tid );

//	DEBUG_NOTICE( DBG_USER, "clock_test finished\n" );

//	K2();

	time_test();

	DEBUG_NOTICE( DBG_USER, "killing name server\n" );
	name_server_stop();
	DEBUG_NOTICE( DBG_USER, "name server killed\n" );

	Exit();
}
