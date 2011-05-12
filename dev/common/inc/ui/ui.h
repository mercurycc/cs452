#ifndef _UI_H_
#define _UI_H_

#include <types.h>
#include <err.h>
#include <ui/timer.h>
#include <console.h>

/*
  UI Definition

  The terminal should be at least 80 x 24 for the UI to work.  The UI will currently only occupy 80 x 24.

   0         1         2         3         4         5         6         7
 0  123456789A123456789A123456789A123456789A123456789A123456789A123456789A123456789A
   1
   2 o---------------------------------------------------------o o----------------o
   3 I Title                                                   I I DD:HH:MM:SS:MS I
   4 o---------------------------------------------------------o o----------------o
   5 o----------------------------------o o---------------------------------------o
   6 I           Switch States          I I            Sensor States              I
   7 I SW nnn [ ] SW nnn [ ] SW nnn [ ] I I Group 0 1 2 3 4 5 6 7 8 9 A B C D E F I
   8 I                                        A
   9 I                                        B
   A                                          C
 1 1                                          D
   2                                          E
   3
   4
   5
   6
   7
   8 > <commands here>
   9 o---------------------------------------------------------------------------o
   A I Command response                                                          I
 2 1
   2
   3
   4

*/
/* ui_timer */

/* DD:HH:MM:SS:MS, 14 characters */
#define UI_TIME_ROW                2
#define UI_TIME_COLUMN             62
#define UI_TIME_HORIZONTAL_MARGIN  2
#define UI_TIME_VERTICAL_MARGIN    1
#define UI_TIME_WIDTH              14 + UI_TIME_HORIZONTAL_MARGIN * 2
#define UI_TIME_HEIGHT             1 + UI_TIME_VERTICAL_MARGIN * 2
#define UI_TIME_BOUNDRY            1

/* ui_prompt */
#define UI_PROMPT_ROW              18
#define UI_PROMPT_COLUMN           2
#define UI_PROMPT_PROMPT           "$ "
#define UI_PROMPT_PROMPT_SIZE      2

/* ui_switch */
#define UI_SWITCH_ROW                5
#define UI_SWITCH_COLUMN             2
#define UI_SWITCH_HORIZONTAL_MARGIN  2
#define UI_SWITCH_VERTICAL_MARGIN    1
#define UI_SWITCH_WIDTH              37 - UI_SWITCH_COLUMN + 1
#define UI_SWITCH_HEIGHT             16 - 5 + 1
#define UI_SWITCH_BOUNDRY            1
#define UI_SWITCH_NAME               "SW nnn [ ]"   /* This piece causes hard coded code in switches.c */
#define UI_SWITCH_NAME_NUMBER_OFFSET 3
#define UI_SWITCH_STATE_OFFSET       8
#define UI_SWITCH_NAME_SIZE          11
#define UI_SWITCH_NAME_PER_LINE      3
#define UI_SWITCH_TITLE              "          Switch States"

/* ui_status */
#define UI_STATUS_ROW                19
#define UI_STATUS_COLUMN             2
#define UI_STATUS_HORIZONTAL_MARGIN  2
#define UI_STATUS_VERTICAL_MARGIN    1
#define UI_STATUS_WIDTH              79 - UI_STATUS_COLUMN + 1
#define UI_STATUS_HEIGHT             3
#define UI_STATUS_BOUNDRY            1

/* ui_sensor */
#define UI_SENSOR_ROW                5
#define UI_SENSOR_COLUMN             39
#define UI_SENSOR_HORIZONTAL_MARGIN  2
#define UI_SENSOR_VERTICAL_MARGIN    1
#define UI_SENSOR_WIDTH              79 - UI_SENSOR_COLUMN + 1
#define UI_SENSOR_HEIGHT             16 - 5 + 1
#define UI_SENSOR_BOUNDRY            1
#define UI_SENSOR_TITLE              "           Sensor States"
#define UI_SENSOR_HEADER_1           "Group 0 1 2 3 4 5 6 7 8 9 A B C D E F"
#define UI_SENSOR_HEADER_2           "A"
#define UI_SENSOR_HEADER_3           "B"
#define UI_SENSOR_HEADER_4           "C"
#define UI_SENSOR_HEADER_5           "D"
#define UI_SENSOR_HEADER_6           "E"
#define UI_SENSOR_HEADER_COUNT       6
#define UI_SENSOR_GROUP_HMARGIN      2
#define UI_SENSOR_STATUS_ROW         3
#define UI_SENSOR_STATUS_COLUMN      7

int ui_init( Console* cons );
int ui_flush();

#endif /* _UI_H_ */
