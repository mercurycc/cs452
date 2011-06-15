#include <types.h>
#include <user/syscall.h>
#include <user/event.h>
#include <user/assert.h>
#include <user/protocals.h>
#include <err.h>
#include <config.h>

#define COURIER_MAGIC   0xc0091e87

void courier()
int courier_init( int courier_tid, int (*callback)( int tid, int arg ) )
int courier_go( int courier_tid, int tid, int arg )
int courier_quit( int courier_tid )
