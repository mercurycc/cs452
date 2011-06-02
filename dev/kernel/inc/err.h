#ifndef _ERR_H_
#define _ERR_H_

#include <config.h>
#include <types.h>
#ifdef DEBUG
#include <bwio.h>
#endif

enum Debug_type {
	DBG_REGION,
	DBG_TIMER,
	DBG_BUFIO,
	DBG_CONS,
	DBG_TRAIN,
	DBG_PROMPT,
	DBG_APP,             /* For main.c */
	DBG_SENSORUI,
	DBG_SENSORCTL,
	DBG_TEMP,
	DBG_TMP,
	DBG_CMD,
	DBG_TRAP,
	DBG_KER,
	DBG_TASK,
	DBG_USER,
	DBG_COUNT,
	DBG_SCHED,
	DBG_NS,              /* Name server */
	DBG_HASH,
	DBG_INT,
	DBG_CLK_DRV
};

#ifdef DEBUG
// #define TRACE          /* Flag for function call trace */
static inline int DEBUG_ENABLE( uint x )
{
	switch( x ){
		/* Disabled debug */
	case DBG_REGION:
	case DBG_TIMER:
	case DBG_BUFIO:
	case DBG_CONS:
	case DBG_COUNT:
	case DBG_TRAIN:
	case DBG_SENSORCTL:
	case DBG_PROMPT:
	case DBG_SENSORUI:
	case DBG_TRAP:
	case DBG_KER:
	case DBG_TASK:
	case DBG_USER:
	case DBG_CMD:
	case DBG_SCHED:
	case DBG_NS:
	case DBG_HASH:
		// case DBG_INT:
		// case DBG_CLK_DRV:
		return 0;
	default:
		return 1;
	}
}

#define DEBUG_FORMAT         "%s (%d), %s: "
#define DEBUG_FORMAT_PARAM   __FILE__, __LINE__, __func__

#define DEBUG_PRINT( level, fmt, arg... )				\
	do {								\
		if( DEBUG_ENABLE( level ) )				\
			bwprintf( COM2, DEBUG_FORMAT fmt, DEBUG_FORMAT_PARAM, arg ); \
	} while( 0 )

#define DEBUG_NOTICE( level, msg )				\
	do {							\
		if( DEBUG_ENABLE( level ) )			\
			bwprintf( COM2, DEBUG_FORMAT msg, DEBUG_FORMAT_PARAM );	\
	} while( 0 )

/* Clear asserts for now */
#define ASSERT( cond )							\
	do {								\
		if( ! ( cond ) ){					\
			bwprintf( COM2, DEBUG_FORMAT "hit assert %s\n", DEBUG_FORMAT_PARAM, #cond ); \
			while( 1 );					\
		}							\
	} while( 0 )
#define ASSERT_M( cond, fmt, arg... )					\
	do {								\
		if( ! (cond ) ){					\
			bwprintf( COM2, DEBUG_FORMAT "hit assert %s: " fmt, DEBUG_FORMAT_PARAM, #cond, arg ); \
			while( 1 );					\
		}							\
	} while( 0 )
#define WORDBINDUMP( word )						\
	do {								\
		uint i;							\
		for( i = 0; i < 32; i += 1 ){				\
			bwprintf( COM2, "%d ", ( word >> ( 32 - i - 1 ) ) % 2 ); \
		}							\
		bwprintf( COM2, "\n" );					\
	} while( 0 )
#else
#define DEBUG_PRINT( level, fmt, arg... )
#define DEBUG_NOTICE( level, msg )
#define ASSERT( cond )
#define ASSERT_M( cond, msg, arg... )
#define WORDBINDUMP( word )
#endif /* DEBUG */

#ifdef TRACE
#define ENTERFUNC()							\
	do {								\
		bwprintf( COM2, "[TRACE/ENTER] %s (%d): %s\n", __FILE__, __LINE__, __func__ ); \
	} while( 0 )
#define LEAVEFUNC()				\
	do {								\
		bwprintf( COM2, "[TRACE/LEAVE] %s (%d): %s\n", __FILE__, __LINE__, __func__ ); \
	} while( 0 )
#else
#define ENTERFUNC()
#define LEAVEFUNC()
#endif


enum Error {
	ERR_NONE                           = 0,
	ERR_INVALID_ARGUMENT               = 1,
	ERR_OUT_OF_MEMORY                  = 2,
	ERR_CONSOLE_NOT_READY              = 3,
	ERR_NOT_READY                      = 3,
	ERR_RBUF_EMPTY                     = 4,
	ERR_RBUF_FULL                      = 5,
	ERR_UI_PROMPT_NOT_DONE             = 6,
	ERR_COMMAND_NOT_SUPPORTED          = 7,
	ERR_COMMAND_WRONG_PARAMETER        = 8,
	ERR_UNKNOWN                        = 9,
	ERR_INVALID_PRIORITY               = 10,
	ERR_OUT_OF_TASK_DESCRIPTOR         = 11,
	ERR_PARENT_EXIT                    = 12,
	ERR_HASHTABLE_FULL                 = 13,
	ERR_HASHTABLE_NOTFOUND             = 14,
	ERR_HASHTABLE_OVERLENGTH           = 15,
	ERR_INTERRUPT_ALREADY_REGISTERED   = 16,
	ERR_INTERRUPT_INVALID_INTERRUPT    = 17
};

#endif /* _ERR_H_ */
