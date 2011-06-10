#include <types.h>
#include <ts7200.h>
#include <regopts.h>
#include <err.h>
#include <watchdog.h>

int watchdog_init()
{
	HW_WRITE( WATCHDOG_FED, 0, 0x05 );
	HW_WRITE( WATCHDOG_CTRL, 0, 0x02 );
	DEBUG_PRINT( DBG_WATCHDOG, "watchdog initialized to %u\n", HW_READ( WATCHDOG_CTRL, 0 ) );

	return ERR_NONE;
}

int watchdog_refresh()
{
	HW_WRITE( WATCHDOG_FED, 0, 0x05 );
	DEBUG_NOTICE( DBG_WATCHDOG, "watchdog fed\n" );

	return ERR_NONE;
}

