#include <types.h>
#include <err.h>
#include <config.h>
#include <console.h>
#include <ui/ui.h>
#include <bufio.h>
#include <regionio.h>
#include <sensorctrl.h>
#include <ui/sensor.h>

/* Sensor main frame */
static Iobuf sensorFrameiobody;
static Iobuf* sensorFrameio;
static Region sensorFramebody;
static Region* sensorFrame;
static uchar buffer[ 256 ];       /* Workaround to the memory problem */
static uchar* buffers[ 2 ];

/* Sensor states */
static Iobuf sensorIoBuf[ SENSOR_COUNT ];
static Region sensor[ SENSOR_COUNT ];
static uchar sensorOutBuf[ SENSOR_COUNT * 20 ];

static void ui_sensor_header( Console* cons )
{
	Iobuf sensorNameiobody;
	Iobuf* sensorNameio;
	Region sensorNamebody;
	Region* sensorName;
	char* text;
	uint margin = UI_SENSOR_GROUP_HMARGIN;
	uint i = 0;
	uint status = 0;

	sensorNameio = &sensorNameiobody;
	sensorName = &sensorNamebody;
	buffers[ 0 ] = buffer;
	buffers[ 1 ] = buffer + UI_SENSOR_WIDTH;

	status = iobuf_init( sensorNameio, cons );
	ASSERT( status == ERR_NONE );

	for( i = 0; i < UI_SENSOR_HEADER_COUNT; i += 1 ){
		switch( i ){
		case 0:
			text = UI_SENSOR_HEADER_1;
			margin = 0;
			break;
		case 1:
			text = UI_SENSOR_HEADER_2;
			break;
		case 2:
			text = UI_SENSOR_HEADER_3;
			break;
		case 3:
			text = UI_SENSOR_HEADER_4;
			break;
		case 4:
			text = UI_SENSOR_HEADER_5;
			break;
		case 5:
			text = UI_SENSOR_HEADER_6;
			break;
		}

		status = region_setup( sensorName, sensorFrame, 0, 2 + i, UI_SENSOR_WIDTH - 2 * UI_SENSOR_HORIZONTAL_MARGIN,
				       1, margin, 0, 0, sensorNameio, buffers );
		ASSERT( status == ERR_NONE );

		status = region_write( sensorName, text );
		ASSERT( status == ERR_NONE );
		
		/* TODO: There was a memory corruption before, where the buffers are provided locally in this function */
	
		while( ! region_flushed( sensorName ) ){
			status = region_flush( sensorName );
			ASSERT( status == ERR_NONE );
		}
	}
}

static inline void ui_sensor_state_init( Console* cons )
{
	uint i = 0;
	int status = 0;
	uchar* buffers[ 2 ];

	for( i = 0; i < SENSOR_COUNT; i += 1 ){
		buffers[ 0 ] = sensorOutBuf + i * 2;
		buffers[ 1 ] = buffers[ 0 ] + 1;
		status = iobuf_init( sensorIoBuf + i, cons );
		ASSERT( status == ERR_NONE );

		status = region_setup( sensor + i, sensorFrame, ( i % SENSOR_COUNT_PER_GROUP ) * 2 + UI_SENSOR_STATUS_COLUMN - 1,
				       UI_SENSOR_STATUS_ROW + i / SENSOR_COUNT_PER_GROUP, 1, 1, 0, 0, 0, sensorIoBuf + i, buffers );
		ASSERT( status == ERR_NONE );
	}
}

int ui_sensor_init( Console* cons )
{
	int status = 0;

	uchar buffersbody[ UI_SENSOR_WIDTH * 2 ];
	uchar* buffers[ 2 ];
	
	sensorFrameio = &sensorFrameiobody;
	sensorFrame = &sensorFramebody;
	buffers[ 0 ] = buffersbody + UI_SENSOR_WIDTH;
	buffers[ 1 ] = buffersbody;

	status = iobuf_init( sensorFrameio, cons );
	ASSERT( status == ERR_NONE );

	status = region_setup( sensorFrame, 0, UI_SENSOR_COLUMN, UI_SENSOR_ROW,
			       UI_SENSOR_WIDTH, UI_SENSOR_HEIGHT, UI_SENSOR_HORIZONTAL_MARGIN,
			       UI_SENSOR_VERTICAL_MARGIN, UI_SENSOR_BOUNDRY,
			       sensorFrameio, buffers );
	ASSERT( status == ERR_NONE );

	status = region_init( sensorFrame );
	ASSERT( status == ERR_NONE );

	status = region_write( sensorFrame, UI_SENSOR_TITLE );
	ASSERT( status == ERR_NONE );

	while( ! region_flushed( sensorFrame ) ){
		status = region_flush( sensorFrame );
		ASSERT( status == ERR_NONE );
	}
	
	ui_sensor_header( cons );

	ui_sensor_state_init( cons );

	return ERR_NONE;
}

int ui_sensor_update( uint group, uint id, uint state )
{
	int status = 0;
	char* text = 0;

	if( state == SENSOR_TRIGGERED ){
		text = "#";
	} else {
		text = " ";
	}
	
	status = region_write( sensor + group * SENSOR_COUNT_PER_GROUP + id, text );
	ASSERT( status == ERR_NONE );	
			      
	return ERR_NONE;
}

int ui_sensor_sensorctrl_update( uchar* sensordata )
{
	uint sensor_group = 0;
	uint sensor_id = 0;
	uint i = 0;

	DEBUG_NOTICE( DBG_SENSORUI, "Received update request\n" );
	
	for( i = 0; i < SENSOR_COUNT; i += 1 ){
		sensor_group = i / SENSOR_COUNT_PER_GROUP;
		sensor_id = i % SENSOR_COUNT_PER_GROUP;
		if( sensordata[ ( i / BITS_PER_BYTE ) ] & HIGH_BIT_MASK >> ( i % BITS_PER_BYTE ) ){
			ui_sensor_update( sensor_group, sensor_id, SENSOR_TRIGGERED );
		} else {
			ui_sensor_update( sensor_group, sensor_id, SENSOR_UNTRIGGERED );
		}
	}

	DEBUG_NOTICE( DBG_SENSORUI, "handled update request\n" );

	return ERR_NONE;
}

int ui_sensor_flush()
{
	uint i = 0;

	DEBUG_NOTICE( DBG_SENSORUI, "Entering flushing\n" );
	for( i = 0; i < SENSOR_COUNT; i += 1 ){
		if( ( sensor + i )->cons->remain < 0 ){
			DEBUG_PRINT( DBG_SENSORUI, "%u hit, val %d\n", ( sensor + i )->cons->remain );
		}
		region_flush( sensor + i );
	}
	DEBUG_NOTICE( DBG_SENSORUI, "Exiting flushing\n" );

	return ERR_NONE;
}

