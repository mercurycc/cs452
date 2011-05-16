#ifndef _SCHED_H_
#define _SCHED_H_

#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>

int sched_init( Context* ctx );
int sched_schedule( Context* ctx, Task** next );

#endif /* _SCHED_H_ */
