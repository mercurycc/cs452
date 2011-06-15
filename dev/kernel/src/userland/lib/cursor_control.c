#include <types.h>
#include <err.h>
#include <config.h>
#include <lib/str.h>

#define CURSOR_CONTROL_ASSIGN_CMD( ch )				\
	do {							\
		cmdbuf[ cmdsize ] = ch;				\
		cmdsize += 1;					\
	} while( 0 )

enum { CURSOR_CONTROL_SAVE,
       CURSOR_CONTROL_RESTORE,
       CURSOR_CONTROL_SET_POS,
       CURSOR_CONTROL_REMOVE_LEFT,
       CURSOR_CONTROL_CLS,
       CURSOR_CONTROL_BACK,
       CURSOR_CONTROL_INSERT,
       CURSOR_CONTROL_COUNT };

static int cursor_control( uint op, uint op1, uint op2, uint op3, char* cmdbuf )
{
	int cmdsize = 0;

	CURSOR_CONTROL_ASSIGN_CMD( 27 );

	switch( op ){
		/* ESC group */
	case CURSOR_CONTROL_SAVE:
		CURSOR_CONTROL_ASSIGN_CMD( '7' );
		break;
	case CURSOR_CONTROL_RESTORE:
		CURSOR_CONTROL_ASSIGN_CMD( '8' );
		break;
		
		/* CSI group */
	case CURSOR_CONTROL_SET_POS:
	case CURSOR_CONTROL_CLS:
	case CURSOR_CONTROL_REMOVE_LEFT:
		CURSOR_CONTROL_ASSIGN_CMD( '[' );
		break;
	}

	/* CSI extra parameters */
	switch( op ){
	case CURSOR_CONTROL_CLS:
		CURSOR_CONTROL_ASSIGN_CMD( '2' );
		CURSOR_CONTROL_ASSIGN_CMD( 'J' );
		break;
	case CURSOR_CONTROL_REMOVE_LEFT:
		cmdsize += utos( op1, cmdbuf + cmdsize );
		CURSOR_CONTROL_ASSIGN_CMD( 'X' );
		break;
	case CURSOR_CONTROL_BACK:
		cmdsize += utos( op1, cmdbuf + cmdsize );
		CURSOR_CONTROL_ASSIGN_CMD( 'D' );
		break;
	case CURSOR_CONTROL_INSERT:
		cmdsize += utos( op1, cmdbuf + cmdsize );
		CURSOR_CONTROL_ASSIGN_CMD( '@' );
		break;
		/* CLS 2 parameters */
	case CURSOR_CONTROL_SET_POS:
		cmdsize += utos( op1, cmdbuf + cmdsize );
		CURSOR_CONTROL_ASSIGN_CMD( ';' );
		cmdsize += utos( op2, cmdbuf + cmdsize );
		CURSOR_CONTROL_ASSIGN_CMD( 'H' );
		break;
	}

	return cmdsize;
}

int cursor_control_cls( char* cmdbuf )
{
	return cursor_control( CURSOR_CONTROL_CLS, 0, 0, 0, cmdbuf );
}

int cursor_control_save( char* cmdbuf )
{
	return cursor_control( CURSOR_CONTROL_SAVE, 0, 0, 0, cmdbuf );
}

int cursor_control_setposn( char* cmdbuf, int row, int col )
{
	return cursor_control( CURSOR_CONTROL_SET_POS, row, col, 0, cmdbuf );
}

int cursor_control_restore( char* cmdbuf )
{
	return cursor_control( CURSOR_CONTROL_RESTORE, 0, 0, 0, cmdbuf );
}
