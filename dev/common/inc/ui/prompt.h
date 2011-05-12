/* Prompt I/O */

#ifndef _PROMPT_H_
#define _PROMPT_H_

#include <types.h>
#include <err.h>
#include <console.h>

/* init will print "$ " on the screen. */
int ui_prompt_init( Console* cons );

/* read should be called periodically, and once it receive data it
   will return ERR_NONE and copy data to data.
   Conditon for end of input is '\n' */
int ui_prompt_read( uchar* buffer, uint* size );

#endif /* _PROMPT_H_ */
