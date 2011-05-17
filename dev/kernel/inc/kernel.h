/* Platform independent kernel routines */
#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <types.h>
#include <err.h>
#include <context.h>

int kernel_init( Context* ctx );
int kernel_shutdown( ptr kernel_sp );

#endif /* _KERNEL_H_ */
