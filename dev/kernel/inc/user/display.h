#ifndef _USER_DISPLAY_H_
#define _USER_DISPLAY_H_

#include <types.h>
#include <config.h>
#include <err.h>
#include <lib/valish.h>

typedef struct Region_s {
	uint col;
	uint row;
	uint height;
	uint width;
	uint margin;
	uint boundary;
} Region;

int display_init();
int region_init( Region* region );
int region_printf( Region* region, char* msg, ... );
int region_clear( Region* region );

#endif /* _USER_DISPLAY_H_ */
