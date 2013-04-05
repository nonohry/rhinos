/**
   
   llist.h
   =======

   Macros for linked list manipulation

**/


#ifndef LLIST_H
#define LLIST_H


/**

   LLIST_NULLIFY
   -------------

   Linked list initialization

**/


#define LLIST_NULLIFY(__e)			\
  (__e) = (void*)0;




/**

   LLIST_ISNULL
   ------------

   Test the existence of a linked list

**/

#define LLIST_ISNULL(__e)				\
  ( (__e)==(void*)0 )



/**

   LLIST_SETHEAD
   -------------

   Set a new first item

**/

#define LLIST_SETHEAD(__e)			\
  {						\
    (__e)->next = (__e);			\
    (__e)->prev = (__e);			\
  }


/**

   LLIST_GETHEAD
   -------------

   get the first item

**/

#define LLIST_GETHEAD(__l)			\
  ( __l )


/**

   LLIST_ISSINGLE
   --------------

   Test whether the linked list is a signleton

**/

#define LLIST_ISSINGLE(__l)			\
  ( ((__l)->next == __l)&&((__l)->prev == __l) )		


/**

   LLIST_ISHEAD
   ------------

   Test whether an item is the first element

**/

#define LLIST_ISHEAD(__l,__e)			\
  ( (__e)==(__l) )


/**

   LLIST_ADD
   ---------

   Add a new item. 
   The new item is placed at the head of the list

**/

#define LLIST_ADD(__l,__e)			\
  {						\
    if (LLIST_ISNULL(__l))			\
      {						\
	(__l)=(__e);				\
	LLIST_SETHEAD(__l);			\
      }						\
    else					\
      {						\
	(__e)->prev = (__l)->prev;		\
	(__e)->next = (__l);			\
	((__l)->prev)->next = (__e);		\
	(__l)->prev = (__e);			\
      }						\
}



/**

   LLIST_REMOVE
   ------------

   Remove an item. 

**/

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



/**

   LLIST_NEXT
   ----------

   Get the item following a given item in the list 

**/

#define LLIST_NEXT(__l,__e)				\
  (__e)->next;


/**

   LLIST_PREVIOUS
   --------------

   Get the item before a given item in the list 

**/

#define LLIST_PREV(__l,__e)				\
  (__e)->prev;


#endif
