#include <types.h>
#include <err.h>
#include <config.h>
#include <console.h>
#include <ui/ui.h>
#include <ui/status.h>
#include <bufio.h>
#include <regionio.h>

static Iobuf statusIobody;
static Iobuf* statusIo;
static Region statusbody;
static Region* statusbar;
static uchar buffersbody[ UI_STATUS_WIDTH * 2 ];
static uchar* buffers[ 2 ];

int ui_status_init( Console* cons )
{
	int status = 0;
	
	statusIo = &statusIobody;
	statusbar = &statusbody;
	buffers[ 0 ] = buffersbody;
	buffers[ 1 ] = buffersbody + UI_STATUS_WIDTH;

	status = iobuf_init( statusIo, cons );
	ASSERT( status == ERR_NONE );

	status = region_setup( statusbar, 0, UI_STATUS_COLUMN, UI_STATUS_ROW, UI_STATUS_WIDTH,
			       UI_STATUS_HEIGHT, UI_STATUS_HORIZONTAL_MARGIN, UI_STATUS_VERTICAL_MARGIN,
			       UI_STATUS_BOUNDRY, statusIo, buffers );	
	ASSERT( status == ERR_NONE );

	status = region_init( statusbar );
	ASSERT( status == ERR_NONE );
	
	return ERR_NONE;
}

int ui_status_write( char* msg )
{
	int status = 0;

	status = region_replace( statusbar );
	ASSERT( status == ERR_NONE );
	status = region_write( statusbar, msg );
	ASSERT( status == ERR_NONE );
	status = region_flush( statusbar );
	ASSERT( status == ERR_NONE );

	return ERR_NONE;
}

int ui_status_flush()
{
	return region_flush( statusbar );
}

