#include <types.h>
#include <err.h>
#include <regionio.h>
#include <bufio.h>
#include <console.h>
#include <str.h>

int region_setup( Region* reg, Region* parent, uint pincol, uint pinrow, uint width, uint height, uint horizmargin, uint vertimargin, uint boundry, Iobuf* cons, uchar** buffers )
{
	reg->parent = parent;
	reg->pincol = pincol;
	reg->pinrow = pinrow;
	reg->width = width;
	reg->height = height;
	reg->horizmargin = horizmargin;
	reg->vertimargin = vertimargin;
	reg->boundry = boundry;
	reg->cons = cons;
	reg->curbuf = buffers[ 0 ];
	reg->futureSize = 0;
	reg->futurebuf = buffers[ 1 ];

	return ERR_NONE;
}

static inline int region_set_cursor( Region* reg, uint offcol, uint offrow )
{
	uint col = reg->pincol + reg->horizmargin + offcol;
	uint row = reg->pinrow + reg->vertimargin + offrow;
	Region* parent = 0;
	int status = 0;

	for( parent = reg->parent; parent; parent = parent->parent ){
		col += parent->pincol + parent->horizmargin;
		row += parent->pinrow + parent->vertimargin;
	}

	status = console_cursor_setposn( reg->cons->cons, row, col );
	ASSERT( status == ERR_NONE );

	return ERR_NONE;
}

static inline int region_set_cursor_nomargin( Region* reg, uint offcol, uint offrow )
{
	return region_set_cursor( reg, offcol - reg->horizmargin, offrow - reg->vertimargin );
}

static inline void region_draw_boundry( Region* reg )
{
	uint row = 0;          /* Row */
	uint col = 0;          /* Col */
	int status = 0;

	status = console_cursor_save( reg->cons->cons );
	ASSERT( status == ERR_NONE );

	for( row = 0; row < reg->height; row += 1 ){
		status = region_set_cursor_nomargin( reg, 0, row );
		ASSERT( status == ERR_NONE );
		for( col = 0; col < reg->width; col += 1 ){
			if( col == 0 || col == reg->width - 1 ){
				if( row == 0 || row == reg->height - 1 ){
					console_write_guarantee( reg->cons->cons, (uchar*)"o", 1 );
				} else {
					console_write_guarantee( reg->cons->cons, (uchar*)"I", 1 );
				}
			} else {
				if( row == 0 || row == reg->height - 1 ) {
					console_write_guarantee( reg->cons->cons, (uchar*)"-", 1 );
				} else {
					console_write_guarantee( reg->cons->cons, (uchar*)" ", 1 );
				}
			}
		}
	}

	status = console_cursor_restore( reg->cons->cons );
	ASSERT( status == ERR_NONE );

	return;
}

int region_init( Region* reg )
{
	ASSERT( reg );

	if( reg->boundry ){
		region_draw_boundry( reg );
	}
	
	reg->curbuf[ 0 ] = '\0';
	reg->futurebuf[ 0 ] = '\0';

	return ERR_NONE;	
}

void region_clear( Region* reg )
{
	uint row = 0;          /* Row */
	uint col = 0;          /* Col */
	int status = 0;

	if( reg->boundry ){
		region_draw_boundry( reg );
	} else {
		status = console_cursor_save( reg->cons->cons );
		ASSERT( status == ERR_NONE );

		for( row = 0; row < reg->height; row += 1 ){
			status = region_set_cursor_nomargin( reg, 0, row );
			ASSERT( status == ERR_NONE );
			for( col = 0; col < reg->width; col += 1 ){
				console_write_guarantee( reg->cons->cons, (uchar*)" ", 1 );
			}
		}

		status = console_cursor_restore( reg->cons->cons );
		ASSERT( status == ERR_NONE );
	}

	return;
}

int region_write( Region* reg, char* str )
{
	uint length = strlen( str );
	
	memcpy( reg->futurebuf, (uchar*)str, length );

	DEBUG_PRINT( DBG_REGION, "%s: get %s, length %u\n", __func__, str, length );
	
	reg->futureSize = length;

	return ERR_NONE;
}

int region_replace( Region* reg )
{
	int status = 0;

	status = iobuf_init( reg->cons, reg->cons->cons );
	ASSERT( status == ERR_NONE );

	reg->curbuf[ 0 ] = 0;

	region_clear( reg );

	return ERR_NONE;
}

int region_flush( Region* reg )
{
	uint i = 0;
	uchar* swaptemp = 0;
	int status = 0;

	DEBUG_NOTICE( DBG_REGION, "flushing\n" ); 

	if( iobuf_flushed( reg->cons ) ){
		DEBUG_NOTICE( DBG_REGION, " flushed\n" ); 
		if ( reg->futureSize ){
			status = iobuf_write( reg->cons, reg->futurebuf, reg->futureSize );
			ASSERT( status == ERR_NONE );
			
			/* Exclude same characters */
			for( i = 0; i < reg->futureSize && reg->curbuf[ i ] == reg->futurebuf[ i ]; i += 1 );

			DEBUG_PRINT( DBG_REGION, " skip = %u\n", i );
			
			status = iobuf_skip( reg->cons, i );
			ASSERT( status == ERR_NONE );
			
			/* Swap 2 buffers */
			swaptemp = reg->futurebuf;
			reg->futurebuf = reg->curbuf;
			reg->curbuf = swaptemp;

			reg->futureSize = 0;
		}
	}
	
	if( ! iobuf_flushed( reg->cons ) ){
		status = console_cursor_save( reg->cons->cons );
		ASSERT( status == ERR_NONE );

		status = region_set_cursor( reg, iobuf_written( reg->cons ), 0 );
		ASSERT( status == ERR_NONE );

		status = iobuf_flush( reg->cons );
		ASSERT( status == ERR_NONE );

		DEBUG_PRINT( DBG_REGION, " flushing done, out = %u\n", reg->cons->cons->last_write );
		status = console_cursor_restore( reg->cons->cons );
		ASSERT( status == ERR_NONE );
	}

	return ERR_NONE;
}

/* Return 1 if region content is flushed, 0 o/w */
int region_flushed( Region* reg )
{
	return ( iobuf_flushed( reg->cons ) && ( reg->futureSize == 0 ) );
}

