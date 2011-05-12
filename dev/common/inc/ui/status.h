#ifndef _STATUS_H_
#define _STATUS_H_

#include <types.h>
#include <err.h>
#include <config.h>
#include <console.h>
#include <ui/ui.h>

/* Warning! This is slow. */
int ui_status_init( Console* cons );
int ui_status_write( char* msg );
int ui_status_flush();

#endif /* _STATUS_H_ */
