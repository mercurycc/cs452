/* Send/Receive protocals between call wrappers and server */
#ifndef _USER_PROTOCALS_H_
#define _USER_PROTOCALS_H_

#include <user/name_server.h>
#include <config.h>

/* Name server */
#define NAME_SERVER_NAME      "NameServer"

/* UART */
/* According to allocations in init */
#define UART1_DRV_TID          4
#define UART2_DRV_TID          5

enum Uart_ports {
	UART_1,
	UART_2
};

#endif /* _USER_PROTOCALS_H_ */
