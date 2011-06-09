/*
 * ts7200.h - definitions describing the ts7200 peripheral registers
 *
 * Specific to the TS-7200 ARM evaluation board
 *
 */

#ifndef _TS7200_H_
#define _TS7200_H_

#define TIMER1_BASE     0x80810000
#define TIMER2_BASE     0x80810020
#define TIMER3_BASE     0x80810080

#define TIMER_LDR_OFFSET        0x00000000      // 16/32 bits, RW
#define TIMER_VAL_OFFSET        0x00000004      // 16/32 bits, RO
#define TIMER_CTRL_OFFSET       0x00000008      // 3 bits, RW
        #define TIMER_ENABLE_MASK       0x00000080
        #define TIMER_MODE_MASK         0x00000040
        #define TIMER_CLKSEL_MASK       0x00000008
#define TIMER_CLR_OFFSET        0x0000000c      // no data, WO


#define LED_ADDRESS     0x80840020
        #define LED_NONE        0x0
        #define LED_GREEN       0x1
        #define LED_RED         0x2
        #define LED_BOTH        0x3

#define COM1    0
#define COM2    1

#define IRDA_BASE       0x808b0000
#define UART1_BASE      0x808c0000
#define UART2_BASE      0x808d0000

// All the below registers for UART1
// First nine registers (up to Ox28) for UART 2

#define UART_DATA_OFFSET        0x0     // low 8 bits
        #define DATA_MASK       0xff
#define UART_RSR_OFFSET         0x4     // low 4 bits
        #define FE_MASK         0x1
        #define PE_MASK         0x2
        #define BE_MASK         0x4
        #define OE_MASK         0x8
#define UART_LCRH_OFFSET        0x8     // low 7 bits
        #define BRK_MASK        0x1
        #define PEN_MASK        0x2     // parity enable
        #define EPS_MASK        0x4     // even parity
        #define STP2_MASK       0x8     // 2 stop bits
        #define FEN_MASK        0x10    // fifo
        #define WLEN_MASK       0x60    // word length
#define UART_LCRM_OFFSET        0xc     // low 8 bits
        #define BRDH_MASK       0xff    // MSB of baud rate divisor
#define UART_LCRL_OFFSET        0x10    // low 8 bits
        #define BRDL_MASK       0xff    // LSB of baud rate divisor
#define UART_CTRL_OFFSET        0x14    // low 8 bits
        #define UARTEN_MASK     0x1
        #define MSIEN_MASK      0x8     // modem status int
        #define RIEN_MASK       0x10    // receive int
        #define TIEN_MASK       0x20    // transmit int
        #define RTIEN_MASK      0x40    // receive timeout int
        #define LBEN_MASK       0x80    // loopback 
#define UART_FLAG_OFFSET        0x18    // low 8 bits
        #define CTS_MASK        0x1
        #define DCD_MASK        0x2
        #define DSR_MASK        0x4
        #define TXBUSY_MASK     0x8
        #define RXFE_MASK       0x10    // Receive buffer empty
        #define TXFF_MASK       0x20    // Transmit buffer full
        #define RXFF_MASK       0x40    // Receive buffer full
        #define TXFE_MASK       0x80    // Transmit buffer empty
#define UART_INTR_OFFSET        0x1c
        #define MIS_MASK        0x1     // Modem interrupt status
        #define RIS_MASK        0x2     // Receive interrupt status
        #define TIS_MASK        0x4     // Transmit interrupt status
        #define RTIS_MASK       0x8     // Receive timeout interrupt status
#define UART_DMAR_OFFSET        0x28

// Specific to UART1

#define UART_MDMCTL_OFFSET      0x100
        #define RTS_MASK        0x2
#define UART_MDMSTS_OFFSET      0x104
#define UART_HDLCCTL_OFFSET     0x20c
#define UART_HDLCAMV_OFFSET     0x210
#define UART_HDLCAM_OFFSET      0x214
#define UART_HDLCRIB_OFFSET     0x218
#define UART_HDLCSTS_OFFSET     0x21c


/* Interrupt source */
/* See EP93xx_User_Guide_UM1.pdf pg 165 for explanation */
#define INTERRUPT_SRC_COMMRX           2
#define INTERRUPT_SRC_COMMTX           3
#define INTERRUPT_SRC_TC1UI            4
#define INTERRUPT_SRC_TC2UI            5
#define INTERRUPT_SRC_AACINTR          6
#define INTERRUPT_SRC_DMAM2P0          7
#define INTERRUPT_SRC_DMAM2P1          8
#define INTERRUPT_SRC_DMAM2P2          9
#define INTERRUPT_SRC_DMAM2P3         10
#define INTERRUPT_SRC_DMAM2P4         11
#define INTERRUPT_SRC_DMAM2P5         12
#define INTERRUPT_SRC_DMAM2P6         13
#define INTERRUPT_SRC_DMAM2P7         14
#define INTERRUPT_SRC_DMAM2P8         15
#define INTERRUPT_SRC_DMAM2P9         16
#define INTERRUPT_SRC_DMAM2M0         17
#define INTERRUPT_SRC_DMAM2M1         18
#define INTERRUPT_SRC_UART1RXINTR1    23
#define INTERRUPT_SRC_UART1TXINTR1    24
#define INTERRUPT_SRC_UART2RXINTR2    25
#define INTERRUPT_SRC_UART2TXINTR2    26
#define INTERRUPT_SRC_UART3RXINTR3    27
#define INTERRUPT_SRC_UART3TXINTR3    28
#define INTERRUPT_SRC_INT_KEY         29
#define INTERRUPT_SRC_INT_TOUCH       30
#define INTERRUPT_SRC_INT_EXT0        32
#define INTERRUPT_SRC_INT_EXT1        33
#define INTERRUPT_SRC_INT_EXT2        34
#define INTERRUPT_SRC_TINTR           35
#define INTERRUPT_SRC_WEINT           36
#define INTERRUPT_SRC_INT_RTC         37
#define INTERRUPT_SRC_INT_IRDA        38
#define INTERRUPT_SRC_INT_MAC         39
#define INTERRUPT_SRC_INT_PROG        41
#define INTERRUPT_SRC_CLK1HZ          42
#define INTERRUPT_SRC_V_SYNC          43
#define INTERRUPT_SRC_INT_VIDEO_FIFO  44
#define INTERRUPT_SRC_INT_SSP1RX      45
#define INTERRUPT_SRC_INT_SSP1TX      46
#define INTERRUPT_SRC_TC3UI           51
#define INTERRUPT_SRC_INT_UART1       52
#define INTERRUPT_SRC_SSPINTR         53
#define INTERRUPT_SRC_INT_UART2       54
#define INTERRUPT_SRC_INT_UART3       55
#define INTERRUPT_SRC_USHINTR         56
#define INTERRUPT_SRC_INT_PME         57
#define INTERRUPT_SRC_INT_DSP         58
#define INTERRUPT_SRC_GPIOINTR        59
#define INTERRUPT_SRC_I2SINTR         60
#define INTERRUPT_COUNT               64

/* VIC registers */
#define VIC1_BASE                     0x800B0000
#define VIC2_BASE                     0x800C0000

#define VIC_IRQ_STATUS_OFFSET         0x0
#define VIC_FIQ_STATUS_OFFSET         0x4
#define VIC_RAW_INTR_OFFSET           0x8
#define VIC_INT_SELECT_OFFSET         0xC
#define VIC_INT_ENABLE_OFFSET         0x10
#define VIC_INT_EN_CLEAR_OFFSET       0x14
#define VIC_SOFT_INT_OFFSET           0x18
#define VIC_SOFT_INT_CLEAR_OFFSET     0x1C
#define VIC_PROTECTION_OFFSET         0x20
#define VIC_VECT_ADDR_OFFSET          0x30
#define VIC_DEF_VECT_ADDR_OFFSET      0x34
#define VIC_VECT_ADDR_OFFSET_OF( n )  ( 0x100 + 4 * ( n ) )
#define VIC_VECT_CNTL_OFFSET_OF( n )  ( 0x200 + 4 * ( n ) )
#define VIC_PERIPH_ID0_OFFSET         0x0FE0
#define VIC_PERIPH_ID1_OFFSET         0x0FE4
#define VIC_PERIPH_ID2_OFFSET         0x0FE8
#define VIC_PERIPH_ID3_OFFSET         0x0FEC

/* VIC register values */
#define VIC_VECT_CNTL_ENABLE          ( 1 << 5 )

/* VIC properties */
#define VIC_VECT_INT_MAX              16

/* CPU POWER CONTROL */
#define CPU_POWER_ADDR                0x80930000
#define CPU_HALT_OFFSET               0x08
#define CPU_STANDBY_OFFSET               0x0C
#define CPU_DEVICECFG_OFFSET          0x80
	#define CPU_SHENA_MASK        0x1
#define CPU_SYSSWLOCK_OFFSET          0xC0
	#define CPU_SWLOCK_MASK       0xAA

#endif /* _TS7200_H_ */
