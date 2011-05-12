#include <types.h>
#include <err.h>
#include <ui/ui.h>
#include <ui/timer.h>
#include <ui/prompt.h>
#include <ui/switches.h>
#include <ui/status.h>
#include <ui/sensor.h>
#include <console.h>

int ui_init( Console* cons )
{
	int status = 0;
	uint i = 0;
	
	status = console_cls( cons );
	ASSERT( status == ERR_NONE );

	/* Timer */
	status = ui_timer_init( cons );
	ASSERT( status == ERR_NONE );

	/* Switches */
	status = ui_switch_init( cons );
	ASSERT( status == ERR_NONE );

	for( i = 0; i < SWITCHES_COUNT; i += 1 ){
		status = ui_switch_update( i, SWITCH_CURVE );
		ASSERT( status == ERR_NONE );
	}

	/* Sensor */
	status = ui_sensor_init( cons );
	ASSERT( status == ERR_NONE );

	/* Status bar */
	status = ui_status_init( cons );
	ASSERT( status == ERR_NONE );

	/* Command prompt */
	status = ui_prompt_init( cons );
	ASSERT( status == ERR_NONE );

	return ERR_NONE;
}

int ui_flush()
{
	static uint start = 0;    /* start from this item */
	const uint count = 5;     /* Total number of items */

	switch( start ){
	case 0:
		ui_timer_update();
	case 1:
		ui_switch_flush();
	case 2:
		ui_sensor_flush();
	case 4:
		ui_status_flush();
	default:
		break;
	}
	
	switch( start ){
	case 4:
		ui_status_flush();
	case 3:
		ui_sensor_flush();
	case 2:
		ui_switch_flush();
	case 1:
		ui_timer_update();
		break;
	}

	start += 1;
	start %= count;

	return ERR_NONE;
}
