/* Send/Receive protocals between call wrappers and server */
#ifndef _USER_PROTOCALS_H_
#define _USER_PROTOCALS_H_

#include <user/name_server.h>

/* Sync */
#define SYNC_MAGIC            0x5117c11e

/* Clock driver */
typedef struct Clock_request_s Clock_request;
typedef struct Clock_reply_s Clock_reply;

enum Clock_request_type {
	CLOCK_CURRENT_TICK,
	CLOCK_COUNT_DOWN,
	CLOCK_QUIT             /* Only parent can send */
};

struct Clock_request_s {
	unsigned int magic;
	unsigned int type;
	unsigned int data;
};

struct Clock_reply_s {
	unsigned int magic;
	unsigned int type;
	unsigned int data;
};

/* Name server */
#define NAME_SERVER_NAME      "NameServer"

typedef struct Name_server_request_s Name_server_request;
typedef struct Name_server_response_s Name_server_response;

enum Name_server_request_type {
	NAME_SERVER_REQUEST_REGISTER_AS,
	NAME_SERVER_REQUEST_WHO_IS,
	NAME_SERVER_DEINIT               /* Only parent can send */
};

struct Name_server_request_s {
	unsigned int magic;
	unsigned int type;
	char name[ NAME_SERVER_NAME_MAX_LENGTH ];
};

struct Name_server_response_s {
	unsigned int magic;
	unsigned int type;
	unsigned int status;
};

/* time server */

typedef struct Time_request_s Time_request;
typedef struct Time_reply_s Time_reply;

enum Time_request_type {
	TIME_ASK,
	TIME_DELAY,
	TIME_SIGNAL
};

struct Time_request_s {
	Time_request_type request;
	int interval;
};

struct Time_reply_s {
	int result;
};

#endif /* _USER_PROTOCALS_H_ */
