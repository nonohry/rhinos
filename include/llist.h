/*
 *
 * Macros Listes Doublement Chainees Circulaires
 *
 */


#ifndef LLIST_H
#define LLIST_H


/* Initialisation de la liste */
#define LLIST_NULLIFY(__e)			\
  (__e) = (void*)0;


/* NULL ? */
#define LLIST_ISNULL(__e)				\
  ( (__e)==(void*)0 )


/* Tete de la liste (set) */
#define LLIST_SETHEAD(__e)			\
  {						\
    (__e)->next = (__e);			\
    (__e)->prev = (__e);			\
  }

/* Tete de la liste (get) */
#define LLIST_GETHEAD(__l)			\
  ( __l )


/* Liste singleton */
#define LLIST_ISSINGLE(__l)			\
  ( ((__l)->next == __l)&&((__l)->prev == __l) )		


/* Comparaison a la tete */
#define LLIST_ISHEAD(__l,__e)			\
  ( (__e)==(__l) )



/* Ajout a la liste */
#define LLIST_ADD(__l,__e)				\
  {						\
    if (LLIST_ISNULL(__l))			\
      {						\
	(__l)=(__e);				\
	LLIST_SETHEAD(__l);			\
      }						\
    else					\
      {						\
	(__e)->prev = (__l)->prev;			\
	(__e)->next = (__l);			\
	((__l)->prev)->next = (__e);		\
	(__l)->prev = (__e);			\
      }						\
}


/* Suppression */
#define LLIST_REMOVE(__l,__e)			\
  {						\
    ((__e)->prev)->next = (__e)->next;		\
    ((__e)->next)->prev = (__e)->prev;		\
    if (LLIST_ISHEAD(__l,__e))			\
    {						\
      if (LLIST_ISSINGLE(__l))			\
	{					\
	  LLIST_NULLIFY(__l);			\
	}					\
      else					\
	{					\
	  (__l)=(__l)->next;			\
	}					\
    }						\
  }


/* Next */
#define LLIST_NEXT(__l,__e)				\
  (__e)->next;


/* Previous */
#define LLIST_PREV(__l,__e)				\
  (__e)->prev;


#endif
