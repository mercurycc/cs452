#ifndef _NAME_SERVER_H_
#define _NAME_SERVER_H_

/* This is including the \0.  name[ NAME_SERVER_NAME_MAX_LENGTH - 1 ]
   = '\0' will be executed right after any string came into the name
   server. */
#define NAME_SERVER_NAME_MAX_LENGTH        16

enum NameServerErrors {
	/* RegisterAs */
	REGISTER_AS_SUCCESS = 0,
	REGISTER_AS_INVALID_NAMESERVER = -1,
	REGISTER_AS_FALSE_NAME_SERVER = -2,
	/* WhoIs */
	WHO_IS_INVALID_NAMESERVER = -1,
	WHO_IS_FALSE_NAME_SERVER = -2
};

int RegisterAs( char* name );
int WhoIs( char* name );

#endif /* _NAME_SERVER_H_ */
