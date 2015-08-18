#ifndef VLIST_H
#define VLIST_H

#include <stdio.h>

/* A single list item. */
struct vlist_i 
{							
	/* The item's data. */
	void *data;
	
	/* Pointers to previous and next items */
	struct vlist_i *prev, *next;							
};												

typedef struct vlist_i vlist_i;

/* A doubly-linked list structure */
struct vlist_s
{			
	/* First and last items in the list */
	vlist_i *head, *tail; 				

	/* Number of items in the list */
	int count;						

	/* 
	 * If this is non-zero, the list owns its items
	 * and will free data when any items are removed
	 */
	int owner;

	/*
	 * Pointer to the function used to free data
	 */
	void (*destroyitem)(void *);
};


typedef struct vlist_s vlist_s;

typedef void vlist_destroy_func_t (void *);

/* function declarations */
int vlist_init(vlist_s *st, vlist_destroy_func_t destroyitem);
void vlist_cleanup(vlist_s *st);
void vlist_clear(vlist_s *st);
vlist_i *vlist_add(vlist_s *st, vlist_i *it, void *data);
vlist_i *vlist_insert(vlist_s *st, vlist_i *it, void *data);	
void vlist_delete(vlist_s *st, vlist_i *it);			
void *vlist_get(vlist_i *it);
vlist_i *vlist_findfwd(vlist_i *it, void *data);
vlist_i *vlist_findrev(vlist_i *it, void *data);
void vlist_debug(vlist_s *st, FILE *stream);

#endif	/* VLIST_H */
