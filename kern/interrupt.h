/**
 
  interrupt.h
  ===========

  Header for interrupt.s
  Give access to hardware interrupts and exceptions assembly handlers

 **/


#ifndef INTERRUPT_H
#define INTERRUPT_H


/**

   Includes
   --------

   - types.h
   - const.h
   - thread.h      : struct thread needed
   
**/

#include <types.h>
#include "const.h"
#include "thread.h"


/**

   Prototypes
   ----------

   Assembly interrupt service routines

**/

EXTERN void hwint_00(void);
EXTERN void hwint_01(void);
EXTERN void hwint_02(void);
EXTERN void hwint_03(void);
EXTERN void hwint_04(void);
EXTERN void hwint_05(void);
EXTERN void hwint_06(void);
EXTERN void hwint_07(void);
EXTERN void hwint_08(void);
EXTERN void hwint_09(void);
EXTERN void hwint_10(void);
EXTERN void hwint_11(void);
EXTERN void hwint_12(void);
EXTERN void hwint_13(void);
EXTERN void hwint_14(void);
EXTERN void hwint_15(void);

EXTERN void swint_syscall(void);

EXTERN void excep_00(void);
EXTERN void excep_01(void);
EXTERN void excep_02(void);
EXTERN void excep_03(void);
EXTERN void excep_04(void);
EXTERN void excep_05(void);
EXTERN void excep_06(void);
EXTERN void excep_07(void);
EXTERN void excep_08(void);
EXTERN void excep_09(void);
EXTERN void excep_10(void);
EXTERN void excep_11(void);
EXTERN void excep_12(void);
EXTERN void excep_13(void);
EXTERN void excep_14(void);
EXTERN void excep_16(void);
EXTERN void excep_17(void);
EXTERN void excep_18(void);



/**

   Structure: struct int_node
   --------------------------

   Link all the first level interrupt handlers  sharing the same IRQ.
   Members are:

   - flih     : first level interrupt handler 
   - prev     : previous item in the linked list 
   - next     : next item in the linked list

**/
   

PUBLIC struct int_node
{
  void (*flih)(struct thread* th);
  struct int_node* prev;
  struct int_node* next;
};


#endif
