/* Send/Receive protocals between call wrappers and server */
#ifndef _USER_PROTOCALS_H_
#define _USER_PROTOCALS_H_

#include <user/name_server.h>

/* Name server */
#define NAME_SERVER_MAGIC     0x11a111e0
#define NAME_SERVER_TID       0x1

typedef struct Name_server_request_s Name_server_request;
typedef struct Name_server_response_s Name_server_response;

enum Name_server_request_type {
	NAME_SERVER_REQUEST_REGISTER_AS,
	NAME_SERVER_REQUEST_WHO_IS,
	NAME_SERVER_DEINIT
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

#endif /* _USER_PROTOCALS_H_ */
