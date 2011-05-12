#include <types.h>
#include <err.h>
#include <config.h>
#include <ui/ui.h>
#include <ui/switches.h>
#include <console.h>
#include <bufio.h>
#include <regionio.h>
#include <str.h>

/* Switch main frame */
static Iobuf switchFrameiobody;
static Iobuf* switchFrameio;
static Region switchFramebody;
static Region* switchFrame;
static uchar buffersbody[ UI_SWITCH_WIDTH * 2 ];
static uchar* buffers[ 2 ];

/* Switch states */
static Iobuf switchesIoBuf[ SWITCHES_COUNT ];
static Region switches[ SWITCHES_COUNT ];
static uchar switchesOutBuf[ SWITCHES_COUNT * 16 ];

static inline int ui_switch_names( Console* cons )
{
	/* Switch name */
	Iobuf switchNameIobody;
	Iobuf* switchNameIo;
	Region switchNamebody;
	Region* switchName;
	uchar switchNameBuffer[ UI_SWITCH_NAME_SIZE * 2 ];
	uchar* switchNamebuffers[ 2 ];

	int status = 0;
	int i = 1;
	int switchId = 1;
	uchar curname[ UI_SWITCH_NAME_SIZE ] = UI_SWITCH_NAME;
	uint convSize = 0;

	switchNameIo = &switchNameIobody;
	switchName = &switchNamebody;
	switchNamebuffers[ 0 ] = switchNameBuffer;
	switchNamebuffers[ 1 ] = switchNameBuffer + UI_SWITCH_NAME_SIZE;

	while( 1 ){
		status = iobuf_init( switchNameIo, cons );
		ASSERT( status == ERR_NONE );
	
		status = region_setup( switchName, switchFrame,
				       ( ( ( i - 1 ) % UI_SWITCH_NAME_PER_LINE ) * UI_SWITCH_NAME_SIZE ),
				       ( i - 1 ) / UI_SWITCH_NAME_PER_LINE + 2,
				       UI_SWITCH_NAME_SIZE, 1,
				       0, 0,
				       0, switchNameIo, switchNamebuffers );
		ASSERT( status == ERR_NONE );

		status = region_init( switchName );
		ASSERT( status == ERR_NONE );

		if( i > 18 ){
			switchId = 153 + i - 19;
		} else {
			switchId = i;
		}

		convSize = utos( switchId, (char*)(curname + UI_SWITCH_NAME_NUMBER_OFFSET) );

		switch( convSize ){
		case 1:
			curname[ UI_SWITCH_NAME_NUMBER_OFFSET + 2 ] = curname[ UI_SWITCH_NAME_NUMBER_OFFSET ];
			curname[ UI_SWITCH_NAME_NUMBER_OFFSET ] = ' ';
			curname[ UI_SWITCH_NAME_NUMBER_OFFSET + 1 ] = ' ';
			break;
		case 2:
			curname[ UI_SWITCH_NAME_NUMBER_OFFSET + 2 ] = curname[ UI_SWITCH_NAME_NUMBER_OFFSET + 1 ];
			curname[ UI_SWITCH_NAME_NUMBER_OFFSET + 1 ] = curname[ UI_SWITCH_NAME_NUMBER_OFFSET ];
			curname[ UI_SWITCH_NAME_NUMBER_OFFSET ] = ' ';
		}
		
		curname[ UI_SWITCH_NAME_NUMBER_OFFSET + 3 ] = ' ';
		curname[ UI_SWITCH_NAME_SIZE - 1 ] = '\0';

		status = region_write( switchName, (char*)curname );
		ASSERT( status == ERR_NONE );

		while( ! region_flushed( switchName ) ){
			status = region_flush( switchName );
			ASSERT( status == ERR_NONE );
		}

		i += 1;
		if( i > SWITCHES_COUNT ){
			break;
		}
	}

	return ERR_NONE;
}

static inline int ui_switch_state_init( Console* cons )
{
	uint i = 0;
	int status = 0;
	uchar* buffers[ 2 ] = { 0 };

	for( i = 0; i < SWITCHES_COUNT; i += 1 ){
		status = iobuf_init( switchesIoBuf + i, cons );
		ASSERT( status == ERR_NONE );

		buffers[ 0 ] = switchesOutBuf + i * 2;
		buffers[ 1 ] = switchesOutBuf + i * 2 + 1;

		region_setup( switches + i, switchFrame,
			      ( i % UI_SWITCH_NAME_PER_LINE ) * UI_SWITCH_NAME_SIZE + UI_SWITCH_STATE_OFFSET,
			      ( i / UI_SWITCH_NAME_PER_LINE ) + 2,
			      1, 1, 0, 0, 0, switchesIoBuf + i, buffers );

		region_init( switches + i );
	}
	
	return ERR_NONE;
}

int ui_switch_init( Console* cons )
{
	int status = 0;

	/* Initialize switchio body */
	/* Notice an Iobuf is only needed to point to the console.  No
	   io should be done in this frame */
	switchFrameio = &switchFrameiobody;
	status = iobuf_init( switchFrameio, cons );
	ASSERT( status == ERR_NONE );
	
	switchFrame = &switchFramebody;
	buffers[ 0 ] = buffersbody;
	buffers[ 1 ] = buffersbody + UI_SWITCH_WIDTH;
	
	status = region_setup( switchFrame, 0,
			       UI_SWITCH_COLUMN, UI_SWITCH_ROW,
			       UI_SWITCH_WIDTH, UI_SWITCH_HEIGHT,
			       UI_SWITCH_HORIZONTAL_MARGIN, UI_SWITCH_VERTICAL_MARGIN,
			       UI_SWITCH_BOUNDRY, switchFrameio, buffers );
	ASSERT( status == ERR_NONE );

	status = region_init( switchFrame );
	ASSERT( status == ERR_NONE );

	status = region_write( switchFrame, UI_SWITCH_TITLE );
	ASSERT( status == ERR_NONE );

	while( ! region_flushed( switchFrame ) ){
		status = region_flush( switchFrame );
		ASSERT( status == ERR_NONE );
	}

	/* Print switch names */
	status = ui_switch_names( cons );
	ASSERT( status == ERR_NONE );

	/* Init switch individual states */
	status = ui_switch_state_init( cons );
	ASSERT( status == ERR_NONE );

	return ERR_NONE;
}

int ui_switch_update( uint switchId, uint state )
{
	char* stateName = 0;

	switch( state ){
	case SWITCH_CURVE:
		stateName = "C";
		break;
	case SWITCH_STRAIGHT:
		stateName = "S";
		break;
	}
	
	region_write( switches + switchId, stateName );
	
	return ERR_NONE;
}

int ui_switch_flush()
{
	uint i = 0;
	
	for( i = 0; i < SWITCHES_COUNT; i += 1 ){
		region_flush( switches + i );
	}
	
	return ERR_NONE;
}
