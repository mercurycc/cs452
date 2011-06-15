#ifndef _CONFIG_H_
#define _CONFIG_H_

#define USE_BWIO
#define DEBUG

/* Kernel parameters */
#define KERNEL_NUM_CONSOLES           2
#define KERNEL_TID_SIZE               7   
#define KERNEL_MAX_NUM_TASKS          ( 1 << KERNEL_TID_SIZE ) / 2
#define KERNEL_STACK_PAGE             16
#define KERNEL_PAGE_SIZE              4096
#define KERNEL_HIGH_PRIORITY          0
#define KERNEL_LOW_PRIORITY           31
#define KERNEL_INIT_TASK_PRIORITY     0          /* Take the first user task as a service */
#define KERNEL_ENABLE_WATCHDOG

/* Userland parameters */
#define IPC_MAGIC                     1          /* Turn on magic check */
#define USER_STACK_PAGE               4
#define NAME_SERVER_HASH_ENTRIES      128
#define CLOCK_OPERATION_TICKS         4          /* Extra time for clock driver to operate if it receives a count down request that is close to current count down */
#define SLOW_DRIVER_PRIORITY          0
#define FAST_DRIVER_PRIORITY          1
#define SERVICE_PRIORITY              2
#define UART_TXBUF_SIZE               1024       /* How many bytes will the UART tx software buffer hold.  This number (2048) should be able to hold a whole screen */
#define DISPLAY_REFRESH_RATE          3          /* In ticks */

/* Trap */
#define TRAP_BASE                     0x20

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
#define TIME_CLK_SRC                  CLK_SRC_508KHZ
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
