#ifndef _USER_LIB_CURSOR_CONTROL_H_
#define _USER_LIB_CURSOR_CONTROL_H_

#include <types.h>

/* Being supplied by a cmd buffer, return the cmd size */
int cursor_control_cls( char* cmdbuf );
int cursor_control_save( char* cmdbuf );
int cursor_control_setposn( char* cmdbuf, int row, int col );
int cursor_control_restore( char* cmdbuf );
int cursor_control_scroll( char* cmdbuf, int low, int high );

#endif /* _USER_LIB_CURSOR_CONTROL_H_ */
