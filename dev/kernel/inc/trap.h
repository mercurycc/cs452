#ifndef _TRAP_H_
#define _TRAP_H_

#include <types.h>
#include <err.h>
#include <context.h>

/* Install trap_handler_begin */
int trap_init( Context* ctx );

/* Userland program will call into trap_handler_begin with swi */
void trap_handler_begin( uint reason, uint sp_usr );

/* Trap handler body */
void trap_handler( uint reason, uint sp_caller, uint mode );

/* Trap hander return to userland through this */
void trap_handler_end( uint reason, uint sp_usr );

#endif /* _TRAP_H_ */
