#ifndef PROC_H
#define PROC_H

#include "types.h"
#include "prot.h"

/**************
 * Constantes
 **************/

#define LDT_SIZE      6
#define PROC_NAME_LEN 32
#define PROC_NUM_MAX  8192-LDT_INDEX  /* Toute la GDT disponible */

/* Selecteur de segments de la LDT  
 *  Le RPL n'apparait pas ici !
 */

#define LDT_CS_SELECTOR	  4    /* CS = 0000000000000  1  00   =  4  */
#define	LDT_DS_SELECTOR   12   /* DS = 0000000000001  1  00   =  12 */
#define	LDT_ES_SELECTOR	  20   /* ES = 0000000000010  1  00   =  20 */
#define	LDT_SS_SELECTOR	  28   /* SS = 0000000000011  1  00   =  28 */
#define	LDT_FS_SELECTOR	  36   /* FS = 0000000000100  1  00   =  36 */
#define	LDT_GS_SELECTOR	  44   /* GS = 0000000000101  1  00   =  44 */


/* Index dans la LDT */

#define LDT_CS_INDEX       0
#define LDT_DS_INDEX       1
#define LDT_ES_INDEX       2
#define LDT_SS_INDEX       3
#define LDT_FS_INDEX       4
#define LDT_GS_INDEX       5


/***************
 * Structures
 ***************/

/* Structure de pile issue de hwint_ret */

PUBLIC struct stack_frame
{
  u16_t gs;
  u16_t fs;
  u16_t es;
  u16_t ds;
  u32_t edi;
  u32_t esi;
  u32_t ebp;
  u32_t ebx;
  u32_t edx;
  u32_t ecx;
  u32_t eax;
  u32_t ret_addr;   /* Adresse de retour empilee par hwint_save */
  u32_t eip;
  u16_t cs;
  u32_t eflags;
  u32_t esp;
  u16_t ss;
} __attribute__ ((packed));


/* Representation d'un processus */

PUBLIC struct proc
{
  struct stack_frame context;      /* Le contexte sauvegarde */
  u16_t index;                     /* Index dans la table de processus */
  u16_t ldt_selector;              /* Selecteur de la ldt dans la gdt */
  struct seg_desc ldt[LDT_SIZE];   /* LDT du processus */
  char name[PROC_NAME_LEN];        /* Nom du processus */
} __attribute__ ((packed));


/***************
 * Prototypes
 ***************/

PUBLIC struct proc proc_table[PROC_NUM_MAX];  /* Table des processus */

PUBLIC void proc_allocate(struct proc* pr, u32_t base, u32_t data_code, u32_t stack, u8_t priv);


#endif
