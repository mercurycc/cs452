#ifndef _TRAIN_WARNING_H_
#define _TRAIN_WARNING_H_

#include "config.h"
#include <user/display.h>

#define WAR_PRINT( fmt, arg... )			\
	{						\
		Region war_reg = WARNING_REGION;	\
		region_printf( &war_reg, fmt, arg );	\
	}

#define WAR_NOTICE( msg )				\
	{						\
		Region war_reg = WARNING_REGION;	\
		region_printf( &war_reg, msg );		\
	}

#endif /* _TRAIN_WARNING_H_  */
