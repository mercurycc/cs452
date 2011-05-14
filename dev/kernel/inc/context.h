#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <types.h>
#include <err.h>
#include <devices/console.h>

typedef struct Context_s {
	Console* terminal;
} Context;

#endif /* _CONTEXT_H_ */
