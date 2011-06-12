#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

#include <types.h>

int watchdog_init();
int watchdog_refresh();
int watchdog_deinit();

#endif /* _WATCHDOG_H_ */
