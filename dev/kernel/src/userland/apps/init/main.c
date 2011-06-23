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

void user_init()
{
	int tid;
	int status;

	/* Fixed launch order in order to obtain fixed tid for servers */
	/*
	  2      name server
	  3      time server
	  4, 5   console servers  (servers for each UART are created by the main console server)
	  6      display server
	*/
	tid = Create( SERVICE_PRIORITY, name_server_start );
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

	tid = Create( IDLE_SERVICE_PRIORITY, display_server );
	assert( tid > 0 );
	DEBUG_PRINT( DBG_USER, "display_server created with tid %d\n", tid );

	/* Driver initialization */
	status = uart_init( UART_1, 2400, 1, 1, 1 );
	assert( status == ERR_NONE );

	status = uart_init( UART_2, 115200, 1, 0, 0 );
	assert( status == ERR_NONE );

	DEBUG_NOTICE( DBG_USER, "uart init done\n" );

	status = display_init();
	assert( status == ERR_NONE );

	tid = Create( 5, train_control );
	
	sync_wait();

	PreShutdown();

	Shutdown();

	Exit();
}
