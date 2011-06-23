/* Train_auto receives sensor and control data to make judgements */
/* It is not implemented strictly as a administrator because incoming
   requests should be rare relative to its speed (sensor every 60 ms,
   control command every second, such and such) */

#include <types.h>
#include <err.h>
#include <lib/str.h>
#include "inc/train.h"
#include "inc/config.h"
#include "inc/sensor_data.h"
#include "inc/track_node.h"

typedef struct Train_stat_s {
	uint total_dist;
	uint total_time;
	uint avg_speed;
} Train_stat;

typedef struct Train_data_s {
	uint distance;
	uint speed;
	uint direction;                  /* Traveling with pick up at the front is forward, backward o/w */
	uint time_stamp;
	track_node* check_point;
} Train_data;

enum Train_auto_request_type {
	TRAIN_AUTO_INIT,
	TRAIN_AUTO_NEW_SENSOR_DATA,
	TRAIN_AUTO_NEW_TRAIN,
	TRAIN_AUTO_SET_TRAIN_SPEED,
	TRAIN_AUTO_SET_SWITCH_DIR,
	TRAIN_AUTO_QUERY_SWITCH_DIR,
	TRAIN_AUTO_QUERY_LAST_SENSOR
};

typedef struct Train_auto_request_s {
	uint type;
	union {
		struct {
			uint track_id;                 /* 0 for track a, 1 for track b */
		} init;
		Sensor_data sensor_data;
		struct {
			uint train_id;
			uint direction;
			uint previous_group;
			uint previous_id;
		} new_train;
		struct {
			uint train_id;
			uint speed_level;
		} set_speed;
		struct {
			uint switch_id;
			char direction;
		} set_switch;
		struct {
			uint switch_id
		} query_switch;
		struct {
			uint dummy;
		} query_sensor;
	} data;
};

void train_auto()
{
	Sensor_data sensor_data;
	uint last_sensor_group;
	uint last_sensor_id;
	int tid;
	track_node track_graph[ TRACK_NUM_NODES ];
	Train_data trains[ MAX_NUM_TRAINS ] = { { 0 } };

	
}
