#include <types.h>
#include <ts7200.h>
#include <regopts.h>
#include <err.h>
#include <watchdog.h>

#define WDT_FED_VAL    0x05

int watchdog_init()
{
	uchar* ctrl_addr = ( uchar* )HW_ADDR( WATCHDOG_CTRL, 0 );
	watchdog_refresh();

	/* Timeout 8 seconds */
	*ctrl_addr = 0x07;
	DEBUG_PRINT( DBG_WATCHDOG, "watchdog initialized to %c\n", ( uchar* )HW_READ( WATCHDOG_CTRL, 0 ) );

	return ERR_NONE;
}

int watchdog_refresh()
{
	uchar* fed_addr = ( uchar* )HW_ADDR( WATCHDOG_FED, 0 );
	*fed_addr = WDT_FED_VAL;
	
	return ERR_NONE;
}

int watchdog_deinit()
{
	uchar* ctrl_addr = ( uchar* )HW_ADDR( WATCHDOG_CTRL, 0 );
	watchdog_refresh();
	*ctrl_addr = 0x00;

	return ERR_NONE;
}
