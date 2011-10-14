/*
 *
 * Macros Listes Doublement Chainees Circulaires
 *
 */


#ifndef LLIST_H
#define LLIST_H


/* Initialisation de la liste */
#define LLIST_NULLIFY(e)			\
  (e) = (void*)0;


/* NULL ? */
#define LLIST_ISNULL(e)				\
  ( (e)==(void*)0 )


/* Tete de la liste (set) */
#define LLIST_SETHEAD(e)			\
  {						\
    (e)->next = e;				\
    (e)->prev = e;				\
  }

/* Tete de la liste (get) */
#define LLIST_GETHEAD(l)			\
  ( l )


/* Liste singleton */
#define LLIST_ISSINGLE(l)			\
  ( ((l)->next == l)&&((l)->prev == l) )		


/* Comparaison a la tete */
#define LLIST_ISHEAD(l,e)			\
  ( (e)==(l) )



/* Ajout a la liste */
#define LLIST_ADD(l,e)				\
  {						\
    if (LLIST_ISNULL(l))			\
      {						\
	l=e;					\
	LLIST_SETHEAD(l);			\
      }						\
    else					\
      {						\
	(e)->prev = (l)->prev;			\
	(e)->next = (l);			\
	((l)->prev)->next = (e);		\
	(l)->prev = (e);			\
      }						\
}


/* Suppression */
#define LLIST_REMOVE(l,e)			\
  {						\
    ((e)->prev)->next = (e)->next;		\
    ((e)->next)->prev = (e)->prev;		\
    if (LLIST_ISHEAD(l,e))			\
    {						\
      if (LLIST_ISSINGLE(l))			\
	{					\
	  LLIST_NULLIFY(l);			\
	}					\
      else					\
	{					\
	  (l)=(l)->next;			\
	}					\
    }						\
  }


/* Next */
#define LLIST_NEXT(l,e)				\
  (e)->next;


/* Previous */
#define LLIST_PREV(l,e)				\
  (e)->prev;


#endif
