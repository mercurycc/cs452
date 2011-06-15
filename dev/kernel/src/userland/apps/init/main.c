#include <user/syscall.h>
#include <user/apps_entry.h>
#include <user/servers_entry.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <user/drivers_entry.h>
#include <user/devices/clock.h>
#include <user/devices/uart.h>
#include <user/time.h>
#include <user/uart.h>
#include <user/display.h>
#include <user/lib/sync.h>
#include <bwio.h>
#include <err.h>
/*
void region_demo()
{
	Region regspec = {2, 1, 10, 20, 1, 1};
	Region subreg = {5, 8, 1, 1, 0, 0};
	int i = 0;

	region_init( &regspec );
	region_init( &subreg );

	region_printf( &regspec, "This is what you get %d, %x.  Newlines\n can be used.", 42, 16 );

	for( i = 0; i < 100; i += 1 ){
		Delay( 20 );
		region_printf( &regspec, "This is what you get %d, %d.  Newlines\n can be used.", 42, i );
		region_printf( &subreg, "%d", i );
	}
}
*/
void user_init()
{
	int tid;
	uint data = 0;
	int status;

	/* Fixed launch order in order to obtain fixed tid for servers */
	/*
	  2      name server
	  3      time server
	  4, 5   console servers  (servers for each UART are created by the main console server)
	  6      display server
	*/
	tid = Create( 2, name_server_start );
	assert( tid > 0 );
	DEBUG_NOTICE( DBG_USER, "name server created\n" );

	tid = Create( SERVICE_PRIORITY, time_main );
	assert( tid > 0 );
	DEBUG_NOTICE( DBG_USER, "time server created\n" );

	/* We actually rely on the fact that the uart driver is at the highest priority */
	tid = Create_drv( SLOW_DRIVER_PRIORITY, uart_driver );
	assert( tid > 0 );
	DEBUG_NOTICE( DBG_USER, "console server created\n" );
	
	tid = Create_drv( SLOW_DRIVER_PRIORITY, uart_driver );
	assert( tid > 0 );
	DEBUG_NOTICE( DBG_USER, "console server created\n" );

	tid = Create( SERVICE_PRIORITY, display_server );
	assert( tid > 0 );
	DEBUG_PRINT( DBG_USER, "display_server created with tid %d\n", tid );

	/* Driver initialization */
	status = uart_init( UART_1, 2400, 0, 1, 1 );
	assert( status == ERR_NONE );

	status = uart_init( UART_2, 115200, 1, 0, 0 );
	assert( status == ERR_NONE );

	DEBUG_NOTICE( DBG_USER, "uart init done\n" );


	tid = Create( 5, train_control );
	
	sync_wait();

	status = Putc( COM_2, '8' );
	assert( status == ERR_NONE );

	PreShutdown();
	
	DEBUG_NOTICE( DBG_USER, "Finishing...\n" );
	time_suicide( WhoIs("time") );
	DEBUG_NOTICE( DBG_USER, "clock server killed\n" );
	
	name_server_stop();
	DEBUG_NOTICE( DBG_USER, "name server killed\n" );

	display_quit();
	DEBUG_NOTICE( DBG_USER, "display killed\n" );

	Exit();
}
