#include <types.h>
#include <err.h>
#include <config.h>
#include <lib/list.h>

int list_init( List* lst )
{
	ASSERT( lst );
	
	lst->prev = lst;
	lst->next = lst;

	return ERR_NONE;
}

int list_empty( List* lst )
{
	return !lst;
}

int list_add_tail( List* lst, List* elem )
{
	List* tail = lst->prev;

	elem->prev = tail;
	elem->next = lst;

	tail->next = elem;

	lst->prev = elem;

	return ERR_NONE;
}

int list_remove_head( List** lst, List** elem )
{
	ASSERT( lst && *lst && elem );

	List* tail = (*lst)->prev;
	int status = 0;

	*elem = *lst;

	if( tail == *lst ){
		*lst = 0;
	} else {
		tail->next = (*lst)->next;
		lst->next->prev = tail;
		*lst = (*lst)->next;
	}

	return ERR_NONE;
}
