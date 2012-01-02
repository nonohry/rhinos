/*
 * Interrupt.h
 * Header pour interrupt.s
 *
 */


#ifndef INTERRUPT_H
#define INTERRUPT_H

/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include "const.h"
#include "context_cpu.h"


/*========================================================================
 * ISR assembleur
 *========================================================================*/

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



/*========================================================================
 * Structure
 *========================================================================*/

/* Structure int_node */
PUBLIC struct int_node
{
  void (*flih)(struct context_cpu* ctx);      /* First Level Interrupt Handler */
  struct int_node* prev;   /* Noeud precedent */
  struct int_node* next;   /* Noeud suivant */
};


#endif
