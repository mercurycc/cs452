#ifndef _SESSION_H_
#define _SESSION_H_

#include <types.h>
#include <err.h>
#include <config.h>
#include <context.h>

void session_start( Context* ctx, uchar* stackPointer );

#endif /* _SESSION_H_ */
