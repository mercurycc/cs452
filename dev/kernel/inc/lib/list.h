/* Simple doubly linked circular list implementation */
#ifndef _LIST_H_
#define _LIST_H_

#include <types.h>
#include <err.h>
#include <config.h>

#define offsetof(TYPE,MEMBER)   ((uint)&((TYPE*)0)->MEMBER)

struct List_s {
	struct List_s* prev;
	struct List_s* next;
};

/* Initialize a list */
int list_init( List* lst );
/* Test if a list is empty */
int list_empty( List* lst );
/* Add to the tail of the list */
int list_add_tail( List** lst, List* elem );
/* Remove the head of the list */
int list_remove_head( List** lst, List** elem );
/* Rotate a list so the list head is the next element in the list */
int list_rotate_head( List** lst );
/* Add to the head of the list */
int list_add_head( List** lst, List* elem );

/* type: The type of struct including the list
   lst:  A pointer to the list contained in the struct
   elem: Name of the list to be used

   Return a pointer to the struct that contains the lst
*/
#define list_entry( type, lst, elem )				\
	( type* )( ( uchar* )lst - offsetof( type, elem ) )

#endif /* _LIST_H_ */
