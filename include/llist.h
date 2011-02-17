/*
 *
 * Macros Listes Chainees
 *
 */


#ifndef LLIST_H
#define LLIST_H

#include <types.h>

/* Initialisation de la liste */
#define LLIST_HEAD(e) \
  e->next = NULL;

/* Ajout a la liste */
#define LLIST_ADD(l,e) \
{ \
  e->next = l->next; \
  e->prev = l; \
  (l->next)->prev = e; \
  l->next = e; \
} 

/* Suppression */
#define LLIST_DELETE(l,e) \
{ \
 if (l==e) \
 { \
   l=l->next; \
 } \
 else \
 { \
   (e->prev)->next = e->next;  \
   (e->next)->prev = e->prev; \
 } \
}

#endif
