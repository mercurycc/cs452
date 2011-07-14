#include <types.h>
#include <err.h>
#include <user/apps_entry.h>
#include <user/assert.h>
#include <user/display.h>
#include <user/syscall.h>
#include <user/uart.h>
#include <user/name_server.h>
#include <user/lib/sync.h>
#include "inc/train.h"
#include "inc/config.h"
#include "inc/train.h"
#include "inc/warning.h"

void train_module_executor()
{
	int module_tid;
	int result;
	
	module_tid = WhoIs( TRAIN_MODULE_NAME );
	
	while(1){
		result = train_execute( module_tid );
		Delay( result );
	}
}

