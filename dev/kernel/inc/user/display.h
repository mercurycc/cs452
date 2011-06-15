#ifndef _USER_DISPLAY_H_
#define _USER_DISPLAY_H_

#include <types.h>
#include <config.h>
#include <err.h>
#include <lib/valist.h>

#define DISPLAY_WIDTH    80
#define DISPLAY_HEIGHT   24
#define DISPLAY_MAX_MSG  1024

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
int region_printf( Region* region, char* fmt, ... );
int region_clear( Region* region );

#endif /* _USER_DISPLAY_H_ */
