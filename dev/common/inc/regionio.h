#ifndef _REGIONIO_H_
#define _REGIONIO_H_

#include <types.h>
#include <err.h>
#include <bufio.h>

/* The region i/o is designed under the assumption that all output to
   a region is meant to show the most recent activity of a state.
   Therefore by writting to futurebuf, although the old data would be
   overwritten, it would also be the most recent data. */
typedef struct Region_s {
	struct Region_s* parent;        /* NULL for terminal */
	uint pincol;                    /* Relative left position wrp parent */
	uint pinrow;                    /* Relative top position wrp parent */
	uint width;                     /* Width of region */
	uint height;                    /* Height of region */
	uint horizmargin;               /* Horizontal margin */
	uint vertimargin;               /* Vertical margin */
	uint boundry;                   /* If a boundry is needed */
	Iobuf* cons;                    /* Output port */
	uchar* curbuf;                  /* Buffer used by curcons */
	uint futureSize;                /* Size of futurebuf */
	uchar* futurebuf;               /* Buffer used by futurecons */
} Region;

/* Setup an i/o region.  Notice buffers has to be uchar*[2] */
int region_setup( Region* reg, Region* parent, uint pincol, uint pinrow, uint width, uint height, uint horizmargin, uint vertimargin, uint boundry, Iobuf* cons, uchar** buffers );

/* Initialize the region */
/* Clear the screen, print margin if needed */
int region_init( Region* reg );

/* Clear region */
void region_clear( Region* reg );

/* Force updated content to be displayed */
int region_replace( Region* reg );

/* Write to a region */
/* This routine will always copy to the futurebuf */
int region_write( Region* reg, char* str );

/* Flush */
/* Should be called periodically */
int region_flush( Region* reg );

/* Return 1 if region content is flushed, 0 o/w */
int region_flushed( Region* reg );

#endif /* _REGIONIO_H_ */
