/*
 * vlist.c
 *
 * "void linked list" structure and helper functions
 *
 * (C)1999 Stefano Busti
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>

#if defined(__UNIX__)
#else
# include <windows.h>
#endif

#include "vlist.h"

/*
 * Initializes a list, 2nd parameter is a pointer to
 * a function which will be used to free the items
 * if the list is an owner list.
 * Call this once just after creating the list.
 */
int vlist_init(vlist_s *st, vlist_destroy_func_t destroyitem) 
{	
	st->head = NULL;	
	st->tail = NULL;
	st->count = 0;	
	st->owner = 1;	
	st->destroyitem = destroyitem;
	return 0;
}	
	
/*
 * Cleans up a list - call this when you've finished
 * with the list.
 */
void vlist_cleanup(vlist_s *st)                 
{	
	vlist_clear(st);	
}	

/*
 * Removes all the items in the list. If the list is
 * an owner list, item data is also freed.
 */
void vlist_clear(vlist_s *st)	
{	
	vlist_i *it = st->head;
	while(it)
	{
		vlist_delete(st, it);
		it = st->head;
	}
}	

vlist_i *vlist_addfirst(vlist_s *st, void *data)
{
	vlist_i *it = malloc(sizeof(vlist_i));

	if (!it)
		return NULL;
	
	it->data = data;
	it->prev = NULL;
	it->next = NULL;
	st->head = it;
	st->tail = it;
	st->count = 1;

	return it;
}

/*
 * Adds a new item after item it, using the data 'data'
 */
vlist_i *vlist_add(vlist_s *st, vlist_i *it, void *data)
{
	vlist_i *nextit;
	
	/* handle special case where list is empty */
	if (st->count == 0)
	{
		return vlist_addfirst(st, data);
	}

	/* grab a pointer to the next item */
	nextit = it->next;

	/* create our new item */
	it->next = malloc(sizeof(vlist_i));

	if (!it->next)
		return NULL;
	
	/* set up the new item */
	it->next->data = data;
	it->next->prev = it;
	it->next->next = nextit;
	
	if (nextit)
	{
		/* link the next item to the new one */
		nextit->prev = it->next;
	}
	else
	{
		/* update tail */
		st->tail = it->next;
	}
	
	/* add one to item count */
	st->count++;

	return it->next;
}

/*
 * Inserts a new item before item it, using the data 'data'
 */
vlist_i *vlist_insert(vlist_s *st, vlist_i *it, void *data)
{
	vlist_i *previt;
	
	/* handle special case where list is empty */
	if (st->count == 0)
	{
		return vlist_addfirst(st, data);
	}

	/* grab a pointer to the previous item */
	previt = it->prev;

	/* create our new item */
	it->prev = malloc(sizeof(vlist_i));

	if (!it->prev)
		return NULL;
	
	/* set up the new item */
	it->prev->data = data;
	it->prev->prev = previt;
	it->prev->next = it;
	
	if (previt)
	{
		/* link the previous item to the new one */
		previt->next = it->prev;
	}
	else
	{
		/* update head */
		st->head = it->prev;
	}
	
	/* add one to item count */
	st->count++;

	return it->prev;
}

/*
 * Removes the item 'it' from the list. If the list is
 * an owner list, item data is also freed.
 */
void vlist_delete(vlist_s *st, vlist_i *it)
{	
	/* call destroyitem() if we're the owner */
	if (st->owner) {
		st->destroyitem(it->data);
		it->data = NULL;	
	}
	if (it->next)	
		it->next->prev = it->prev;	
	else
		st->tail = it->prev;
		
	if (it->prev)	
		it->prev->next = it->next;
	else
		st->head = it->next;
		
	free(it);
	st->count--;	
}

/*
 * Returns the item data for item it
 */
void *vlist_get(vlist_i *it)
{
	return it->data;
}

/*
 * Searches the list forwards for 'data',
 * starting at item 'it'.
 * Returns NULL if not found.
 */
vlist_i *vlist_findfwd(vlist_i *it, void *data)
{
	vlist_i *item;
	
	/* iterate forwards searching for data */
	for (item = it; item; item = item->next)
		if (item->data == data)
			return item;

	/* not found */
	return NULL;
}

/*
 * Searches the list backwards for 'data',
 * starting at item 'it'.
 * Returns NULL if not found.
 */
vlist_i *vlist_findrev(vlist_i *it, void *data)
{
	vlist_i *item;

	/* iterate backwards searching for data */
	for (item = it; item; item = item->prev)
		if (item->data == data)
			return item;

	/* not found */
	return NULL;
}

void vlist_debug(vlist_s *st, FILE *stream)
{
	vlist_i *it;
	fprintf(stream, "list with %d items\n", st->count);
	fprintf(stream, "head = 0x%08lx\n", (unsigned int)st->head);
	fprintf(stream, "tail = 0x%08lx\n", (unsigned int)st->tail);
	fprintf(stream, "items are:\n{\n");

	for (it = st->head; it; it = it->next)
	{
		fprintf(stream, "\t0x%08lx << [ 0x%08lx ] >> 0x%08lx (data:0x%08lx)\n",
				(unsigned int)it->prev, (unsigned int)it,
				(unsigned int)it->next, (unsigned int)it->data);
	}

	fprintf(stream, "}\n\n");
}
