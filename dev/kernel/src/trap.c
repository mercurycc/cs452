#include <types.h>
#include <err.h>

/* Install trap_handler_begin */
int trap_init( Context* ctx );

/* Userland program will call into trap_handler_begin with swi */
void trap_handler_begin( uint reason, uint sp_usr );

/* Trap handler body */
void trap_handler( uint reason, uint sp_caller );

/* Trap hander return to userland through this */
void trap_handler_end( uint reason, uint sp_usr );
