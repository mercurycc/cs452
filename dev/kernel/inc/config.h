#ifndef _CONFIG_H_
#define _CONFIG_H_

#define USE_BWIO
// #define DEBUG

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
#define USER_STACK_PAGE               16
#define NAME_SERVER_HASH_ENTRIES      128
#define CLOCK_OPERATION_TICKS         4          /* Extra time for clock driver to operate if it receives a count down request that is close to current count down */
#define SLOW_DRIVER_PRIORITY          0
#define FAST_DRIVER_PRIORITY          1
#define SERVICE_PRIORITY              2
#define IDLE_SERVICE_PRIORITY         15
#define UART_TXBUF_SIZE               1024       /* How many bytes will the UART tx software buffer hold.  This number (2048) should be able to hold a whole screen */
#define DISPLAY_REFRESH_RATE          3          /* In ticks */

/* Trap */
#define TRAP_BASE                     0x20

/* Time */
#define TIME_USE_CLK                  CLK_3
#define TIME_CLK_MODE                 CLK_MODE_FREE_RUN
#define TIME_CLK_SRC                  CLK_SRC_508KHZ
#define TIME_CLK_INITIAL              0xffffffff

/* Constants for their meaning */
#define BITS_PER_BYTE                 8

#endif
