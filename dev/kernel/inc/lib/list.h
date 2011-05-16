/* Simple doubly linked circular list implementation */
#ifndef _LIST_H_
#define _LIST_H_

#include <types.h>
#include <err.h>
#include <config.h>

struct List_s {
	struct* List_s prev;
	struct* List_s next;
};

int list_init( List* lst );
int list_empty( List* lst );
int list_add_tail( List* lst, List* elem );
List* list_remove_head( List* lst );

/* type: The type of struct including the list
   lst:  A pointer to the list contained in the struct
   elem: Name of the list to be used

   Return a pointer to the struct that contains the lst
*/
#define list_entry( type, lst, elem )				\
	( type* )( ( uchar* )lst - offsetof( type, elem ) )

#endif /* _LIST_H_ */
