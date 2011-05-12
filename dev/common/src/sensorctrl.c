#include <types.h>
#include <err.h>
#include <config.h>
#include <train.h>
#include <ui/sensor.h>
#include <ui/status.h>

#define HIGH_BIT_MASK  1 << 7

static uint sensor_recent_group;
static uint sensor_recent_id;
static uchar sensordata[ SENSOR_GROUPS * SENSOR_DATA_SIZE ];

int sensorctrl_init()
{
	sensor_recent_group = 0;
	sensor_recent_id = 0;
	
	return ERR_NONE;
}

int sensorctrl_update()
{
	int status = 0;
	int i = 0;

	status = train_sensor_query_all( sensordata );
	if( status == ERR_NONE ){
		DEBUG_PRINT( DBG_SENSORCTL, "%x %x %x %x %x %x %x %x %x %x\n",
			     sensordata[ 0 ], sensordata[ 1 ], sensordata[ 2 ], sensordata[ 3 ], sensordata[ 4 ],
			     sensordata[ 5 ], sensordata[ 6 ], sensordata[ 7 ], sensordata[ 8 ], sensordata[ 9 ] );
		status = ui_sensor_sensorctrl_update( sensordata );
		ASSERT( status == ERR_NONE );

		for( i = 0; i < SENSOR_COUNT; i += 1 ){
			if( sensordata[ ( i / BITS_PER_BYTE ) ] & HIGH_BIT_MASK >> ( i % BITS_PER_BYTE ) ){
				sensor_recent_group = i / SENSOR_COUNT_PER_GROUP;
				sensor_recent_id = i % SENSOR_COUNT_PER_GROUP;
				break;
			}
		}
	} else {
		return ERR_NOT_READY;
	}
	
	return ERR_NONE;
}

int sensorctrl_most_recent( uint* group, uint* id )
{
	*group = sensor_recent_group;
	*id = sensor_recent_id;

	return ERR_NONE;
}
