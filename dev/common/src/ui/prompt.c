#include <types.h>
#include <err.h>
#include <ui/ui.h>
#include <console.h>
#include <ui/prompt.h>
#include <bufio.h>
#include <str.h>

static uchar input[ UI_PROMPT_CMD_LENGTH ];          /* Input buffer */
static uint pos;                                     /* Input count */
static Console* term;

int ui_prompt_init( Console* cons )
{
	int status = 0;

	term = cons;
	input[ 0 ] = '\0';
	pos = 0;

	status = console_cursor_setposn( cons, UI_PROMPT_ROW, UI_PROMPT_COLUMN );
	ASSERT( status == ERR_NONE );

	console_write_guarantee( cons, (uchar*)UI_PROMPT_PROMPT, UI_PROMPT_PROMPT_SIZE );

	return ERR_NONE;
}

int ui_prompt_read( uchar* buffer, uint* size )
{
	uchar ch = 0;
	int status = 0;
	
	status = console_read( term, &ch, 1 );
	ASSERT( status == ERR_NONE );
	
	if( term->last_read == 1 ){
		DEBUG_PRINT( DBG_PROMPT, "read character %c (%u)\n", ch, ch );
		if( ! ( ch == '\n' || ch == '\r' ) ){
			input[ pos ] = ch;
			/* TODO:
			   A queue'd IO class might needed, train.c use the same mechanism as well.
			*/
			status = console_write_guarantee( term, &ch, 1 );
			ASSERT( status == ERR_NONE );
		} else {
			status = console_cursor_remove( term, pos );
			ASSERT( status == ERR_NONE );
			input[ pos ] = '\0';
		}

		pos += 1;
		ASSERT( pos <= UI_PROMPT_CMD_LENGTH );

		if( ch == '\n' || ch == '\r' ){
			memcpy( buffer, input, pos );
			*size = pos;
			pos = 0;
			return ERR_NONE;
		}
	}
	
	return ERR_UI_PROMPT_NOT_DONE;
}
