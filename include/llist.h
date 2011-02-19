/*
 *
 * Macros Listes Doublement Chainees Circulaires
 *
 */


#ifndef LLIST_H
#define LLIST_H


/* Initialisation de la liste */
#define LLIST_HEAD(e) \
{ \
  (e)->next = e; \
  (e)->prev = e; \
}

/* Ajout a la liste */
#define LLIST_ADD(l,e) \
{ \
  (e)->next = (l)->next; \
  (e)->prev = (l); \
  ((l)->next)->prev = (e); \
  (l)->next = (e); \
}

/* Suppression */
#define LLIST_REMOVE(l,e) \
{ \
 ((e)->prev)->next = (e)->next;  \
 ((e)->next)->prev = (e)->prev; \
  if ((l)==(e)) \
  { \
   (l)=(l)->next; \
  } \
}


/* Next */
#define LLIST_NEXT(l,e) \
	(e)->next;
	
/* Previous */
#define LLIST_PREV(l,e) \
	(e)->prev;



#endif
