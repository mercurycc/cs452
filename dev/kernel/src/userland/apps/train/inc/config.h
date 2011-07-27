#ifndef _TRAIN_CONFIG_H_
#define _TRAIN_CONFIG_H_

/* If we will caliberate speed change time */
// #define CALIB_SPEED_CHANGE

/* Train module transmission delays */
#define SWITCH_XMIT_DELAY 18
#define SWITCH_OFF_DELAY 20
#define TRAIN_XMIT_DELAY 20
#define SWITCH_ALL_DELAY 180

#define SENSOR_BYTE_COUNT    10
#define SENSOR_NAME_LENGTH    3
#define NUM_SWITCHES         22
#define MAX_NUM_TRAINS       5
#define MAX_TRAIN_ID         128
#define PATH_BUFFER_SIZE     128
#define TARGET_BUFFER_SIZE   20     /* First for current chk pnt, others for future */
#define PATH_LOOK_AHEAD_TIME    200
#define PATH_LOOK_AHEAD_LOW     6
#define PATH_LOOK_AHEAD_HIGH    10
#define PATH_LOOK_AHEAD_BUFFER  32
#define PATH_LOOK_AHEAD_DIST    1000
// #define PATH_LOOK_AHEAD_ADJUST_DIST 800
#define PATH_LOOK_AHEAD_DEFAULT_STOP    350
#define PLANNER_WAKE_UP      2      /* Length of time between each check when the train is stopping */

/* number of speed levels of train */
#define NUM_SPEED_LEVEL 30

/* number of ticks needed to change speed */
#define SPEED_CHANGE_TIME   350
#define CLOSE_SPEED_CHANGE_TIME 220
#define LOW_SPEED_BREAK_TIME    70
#define CLOSE_SPEED_CHANGE_GAP  8
#define START_CHANGE_TIME       350     /* Time for pick up speed from 0 */
#define STOP_SAFE_TIME          400
#define REPLAN_TIME_POOL_SIZE   16		/* replan time = random(0-15) * 10 ticks*/
#define PLAN_TIME               10
#define COMMAND_DELAY_TIME  7
#define SAFETY_DISTANCE     300     /* As defined in train_auto.c */
#define DEFAULT_MIN_SC_TIME 200
#define DEFAULT_MAX_SC_TIME 350

/* Track reservation */
#define TRACK_RESERVE_SAFE_DISTANCE         50
#define TRACK_RESERVE_SAFE_MODIFIER         160
#define TRACK_RESERVE_BRANCH_SAFE_DISTANCE  240
#define TRACK_RESERVE_INIT_DISTANCE         800

/* Train init */
#define TRAIN_AUTO_REG_SPEED_1  8
#define TRAIN_AUTO_REG_SPEED_2  12

#define TRAIN_TRAVEL_SPEED_1        12
#define TRAIN_TRAVEL_SPEED_2_LENGTH 2000
#define TRAIN_TRAVEL_SPEED_2        10
#define TRAIN_TRAVEL_SPEED_3_LENGTH 1000
#define TRAIN_TRAVEL_SPEED_3        6
#define TRAIN_TRAVEL_SPEED_4_LENGTH 500
#define TRAIN_TRAVEL_SPEED_4        3
#define TRAIN_TRAVEL_SPEED_5_LENGTH 20
#define TRAIN_TRAVEL_SPEED_5        1

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
#define RESERVE_NAME        "track_rsv"
#define PLANNER_UI_NAME     "planner_ui"
#define TRAIN_COMMAND_MAX_TOKEN  16

/* Timing, in ticks */
#define SENSOR_XMIT_TIME    8
#define TRAIN_AUTO_WAKEUP_PERIOD 2

#define WARNING_REGION       { 14, 23, 1, 78 - 14, 1, 0 }
#define SWID_TO_ARRAYID( i ) ( i < 19 ? i - 1 : i - 135 )
#define ARRAYID_TO_SWID( i ) ( i < 18 ? i + 1 : i + 135 )

#endif /* _TRAIN_CONFIG_H_ */
