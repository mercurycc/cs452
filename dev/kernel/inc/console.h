/* Generic console I/O routines */
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <types.h>

enum ConsoleChannel { CONSOLE_1,
		      CONSOLE_2,
		      CONSOLE_COUNT };

typedef struct Console_s {
	int channel;             /* Channel used */
	int speed;               /* Speed */
	int fifo;                /* Flag is fifo enabled */
	uint dbl_stop;           /* Flag if 2 stop bits are used */
	uint flow_control;       /* Flag if the console has flow control */
	uint blocking;           /* Flag if block to wait */
	uint write_bytes;        /* Total bytes written */
	uint read_bytes;         /* Total bytes read */
	uint last_write;         /* Bytes written in last write */
	uint last_read;          /* Bytes read in last read */
} Console;

/* Setup console handle */
int console_setup( Console* cons, uint channel, uint speed, uint blocking, uint flow_control, uint dbl_stop );

/* Initialization and deinitialization*/
int console_init( Console* cons );
int console_deinit( Console* cons );

/* Raw I/O */
int console_write( Console* cons, const uchar* data, uint size );
int console_read( Console* cons, uchar* buffer, uint size );
int console_write_guarantee( Console* cons, const uchar* data, uint size );
int console_read_guarantee( Console* cons, uchar* data, uint size );
/* Drain read buffer */
int console_drain( Console* cons );

/* Character I/O */
int console_putc( Console* cons, char c );
int console_getc( Console* cons, char* c );

/* String I/O */
int console_printf( Console* cons, char* format, ... );
int console_scanf( Console* cons, char* format, ... );

/* Cursor control */
int console_cls( Console* cons );
int console_cursor_save( Console* cons );
int console_cursor_setposn( Console* cons, int row, int col );
int console_cursor_restore( Console* cons );
int console_cursor_remove( Console* cons, uint count );

#endif  /* _CONSOLE_H_ */
