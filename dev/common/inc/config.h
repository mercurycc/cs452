#ifndef _CONFIG_H_
#define _CONFIG_H_

#define USE_BWIO
#define DEBUG

/* Kernel parameters */
#define KERNEL_NUM_CONSOLES           2
#define KERNEL_MAX_NUM_TASKS          1024
#define KERNEL_PAGE_SIZE              4096
#define KERNEL_STACK_PAGE             3

/* Userland parameters */
#define USER_STACK_PAGE               1

/* Train control */
#define TRAIN_COMMAND_BUFFER_LENGTH   256
extern unsigned int TRAIN_COMMAND_GAP;    /* In milli-seconds */
#define TRAIN_MAX_TRAINS              80
#define TRAIN_SENSOR_QUERY_TIME_OUT   3000

/* Switches */
/* We have 22 switches: 1 - 18, 153 - 156 */
#define SWITCHES_COUNT                22

/* Sensor */
#define SENSOR_GROUPS                 5
#define SENSOR_COUNT_PER_GROUP        16
#define SENSOR_COUNT                  SENSOR_GROUPS * SENSOR_COUNT_PER_GROUP
#define SENSOR_DATA_SIZE              2

/* Time */
#define TIME_USE_CLK                  CLK_3
#define TIME_CLK_MODE                 CLK_MODE_FREE_RUN
#define TIME_CLK_SRC                  CLK_SRC_2KHZ
#define TIME_CLK_INITIAL              0xffffffff

/* ui_prompt */
#define UI_PROMPT_CMD_LENGTH          256

/* Command parser */
#define COMMAND_DELIMITER             ' '
#define COMMAND_TOKEN_SIZE            64
#define COMMAND_MAX_PARAM_COUNT       16

/* Constants for their meaning */
#define BITS_PER_BYTE                 8

#endif
