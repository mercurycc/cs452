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
	uint col_append;
	uint row_append;
} Region;

int display_init();
int region_init( Region* region );
int region_printf( Region* region, char* fmt, ... );
int region_append( Region* region, char* fmt, ... );
int scroll_printf( char* fmt, ... );
int region_clear( Region* region );
int display_quit();

#endif /* _USER_DISPLAY_H_ */
