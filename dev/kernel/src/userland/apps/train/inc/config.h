#ifndef _TRAIN_CONFIG_H_
#define _TRAIN_CONFIG_H_

/* Train module transmission delays */
#define SWITCH_XMIT_DELAY 18
#define SWITCH_OFF_DELAY 20
#define TRAIN_XMIT_DELAY 20
#define SWITCH_ALL_DELAY 180

#define SENSOR_BYTE_COUNT    10
#define SENSOR_NAME_LENGTH    3
#define NUM_SWITCHES         22
#define MAX_NUM_TRAINS       10
#define MAX_TRAIN_ID         128
#define PATH_BUFFER_SIZE     128
#define TARGET_BUFFER_SIZE   20     /* First for current chk pnt, others for future */
#define PATH_LOOK_AHEAD_TIME    200
#define PATH_LOOK_AHEAD_LOW     6
#define PATH_LOOK_AHEAD_HIGH    10
#define PATH_LOOK_AHEAD_BUFFER  32
#define PATH_LOOK_AHEAD_DIST    1000
// #define PATH_LOOK_AHEAD_ADJUST_DIST 800
#define PATH_LOOK_AHEAD_DEFAULT_STOP    250
#define PLANNER_WAKE_UP      2      /* Length of time between each check when the train is stopping */

/* number of ticks needed to change speed */
#define SPEED_CHANGE_TIME   260
#define CLOSE_SPEED_CHANGE_TIME 220
#define CLOSE_SPEED_CHANGE_GAP  4
#define START_CHANGE_TIME   400     /* Time for pick up speed from 0 */
#define STOP_SAFE_TIME      500
#define COMMAND_DELAY_TIME  7

#define TRAIN_AUTO_REG_SPEED_1  8
#define TRAIN_AUTO_REG_SPEED_2  12

#define TRAIN_TRAVEL_SPEED_1        12
#define TRAIN_TRAVEL_SPEED_2_LENGTH 2000
#define TRAIN_TRAVEL_SPEED_2        10
#define TRAIN_TRAVEL_SPEED_3_LENGTH 1000
#define TRAIN_TRAVEL_SPEED_3        5
#define TRAIN_TRAVEL_SPEED_4_LENGTH 500
#define TRAIN_TRAVEL_SPEED_4        3

#define TRAIN_AUTO_REG_RETRY      2
#define TRAIN_AUTO_REG_SPEED_CALIB_TIME   500

#define SENSOR_GROUP_COUNT      5
#define SENSOR_COUNT_PER_GROUP  16

/* Error bounds */
#define DIST_ERROR           40       /* in mm */
#define TIME_ERROR           0        /* TODO: Undefined yet */

/* Perf measurements */
#define SENSOR_AVERAGE_DELAY  8       /* in tickes */

/* UI */
#define SENSOR_UI_NAME      "sensor_ui"
#define TRAIN_AUTO_NAME     "train_auto"
#define TRAIN_SWITCH_NAME   "switch_ui"
#define TRAIN_MODULE_NAME   "train_mod"
#define SENSOR_QUERY_NAME   "sensor_qu"
#define TRACKING_UI_NAME    "tracking_ui"
#define CONTROL_NAME        "train_ctl"
#define TRAIN_COMMAND_MAX_TOKEN  16

/* Timing, in ticks */
#define SENSOR_XMIT_TIME    8
#define TRAIN_AUTO_WAKEUP_PERIOD 2

#define WARNING_REGION       { 14, 23, 1, 78 - 14, 1, 0 }
#define SWID_TO_ARRAYID( i ) ( i < 19 ? i - 1 : i - 135 )
#define ARRAYID_TO_SWID( i ) ( i < 18 ? i + 1 : i + 135 )

#endif /* _TRAIN_CONFIG_H_ */
