/* Command parser and issuer */
#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <types.h>
#include <err.h>

int command_init();
int command_execute( uchar* cmd );

#endif /* _COMMAND_H_ */
