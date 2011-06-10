#include <user/syscall.h>
#include <user/apps_entry.h>
#include <user/servers_entry.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <user/drivers_entry.h>
#include <user/devices/clock.h>
#include <user/devices/uart.h>
#include <user/time.h>
#include <user/lib/sync.h>
#include <bwio.h>
#include <err.h>

void user_init()
{
	int tid;
	uint data = 0;
	int status;

	/* Fixed launch order in order to obtain fixed tid for servers */
	/*
	  1      name server
	  2      time server
	  3 - 7  console servers  (servers for each UART are created by the main console server)
	*/
	tid = Create( 2, name_server_start );
	assert( tid > 0 );
	DEBUG_NOTICE( DBG_USER, "name server created\n" );

	tid = Create( SERVICE_PRIORITY, time_main );
	assert( tid > 0 );
	DEBUG_NOTICE( DBG_USER, "time server created\n" );

	/* We actually rely on the fact that the uart driver is at the highest priority */
	/* tid = Create( SLOW_DRIVER_PRIORITY, uart_driver ); */
	/* assert( tid > 0 ); */
	/* DEBUG_NOTICE( DBG_USER, "console server created\n" ); */
	
	/* tid = Create( SLOW_DRIVER_PRIORITY, uart_driver ); */
	/* assert( tid > 0 ); */
	/* DEBUG_NOTICE( DBG_USER, "console server created\n" ); */

	/* status = uart_init( UART_1,  */


	/* while( data != 'q' ){ */
	/* 	status = uart_getc(  */

	tid = Create( 1, lazy_dog );
	assert( tid > 0 );
	DEBUG_NOTICE( DBG_USER, "test case created\n" );

	
	
	sync_wait();
	
	DEBUG_NOTICE( DBG_USER, "Finishing...\n" );
	time_suicide( WhoIs("time") );
	DEBUG_NOTICE( DBG_USER, "clock server killed\n" );
	
	name_server_stop();
	DEBUG_NOTICE( DBG_USER, "name server killed\n" );

	Exit();
}
