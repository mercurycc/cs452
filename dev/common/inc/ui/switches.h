#ifndef _SWITCH_H_
#define _SWITCH_H_

#include <types.h>
#include <err.h>
#include <config.h>
#include <console.h>
#include <ui/ui.h>

enum SwitchState { SWITCH_CURVE,
		   SWITCH_STRAIGHT };

int ui_switch_init( Console* cons );
int ui_switch_update( uint switchId, uint state );
int ui_switch_flush();

#endif /* _SWITCH_H_ */
