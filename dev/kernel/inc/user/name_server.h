#ifndef _NAME_SERVER_H_
#define _NAME_SERVER_H_

enum NAME_SERVER_TYPES {
	NAME_SERVER_REGISTER_AS,
	NAME_SERVER_WHO_IS,
	NAME_SERVER_NOT_AVAILABLE
};

typedef struct Name_server_request_s Name_server_request;
typedef struct Name_server_reply_s Name_server_reply;

struct Name_server_request_s {
	unsigned int request_type;
	char* data;
	unsigned int datalen;
	Name_server_reply* response;
};

struct Name_server_reply_s {
	unsigned int reply_type;
	char* data;
	unsigned int datalen;	
};

#endif /* _NAME_SERVER_H_ */
