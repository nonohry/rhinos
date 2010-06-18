#ifndef PROC_H
#define PROC_H

#include <types.h>
#include "prot.h"

/**************
 * Constantes
 **************/

#define LDT_SIZE      6
#define PROC_NAME_LEN 32
#define PROC_NUM_MAX  8192-LDT_INDEX  /* Toute la GDT disponible */
#define PROC_IF       0x200           /* 1000000000b */

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

/* Skip List */

#define SKIP_MAX_LEVEL     8

/* Etat des processus */

#define PROC_TERMINATED    0
#define PROC_BLOCKED       1
#define PROC_READY         2
#define PROC_RUNNING       3

/* Quantum temps */

#define PROC_QUANTUM       2     /* 20 ms de quantum d execution (on est a 100Hz, 2 = 20ms) */

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
  u32_t orig_esp;
  u32_t ebx;
  u32_t edx;
  u32_t ecx;
  u32_t eax;
  u32_t ret_addr;   /* Adresse de retour empilee par hwint_save */
  u32_t eip;
  u32_t cs;
  u32_t eflags;
  u32_t esp;
  u32_t ss;
} __attribute__ ((packed));


/* Structure d'une skip list */

PUBLIC struct skip_list
{
  u16_t level;                     /* Niveau courant de la liste */
  u32_t tickets;                   /* Somme courante des tickets */
  struct skip_node* header;        /* header */
  struct skip_node* update[SKIP_MAX_LEVEL]; /* Tableau pour insertion */
};

/* Structure d'un noeud de skip_list */

PUBLIC struct skip_node
{
  u32_t key;                       /* Cle de recherche (somme des tickets) */
  u32_t tickets;                   /* Tickets du process */
  u32_t index;                     /* Index dans la table des processus */
  struct skip_node* forwards[SKIP_MAX_LEVEL];  /* Successeurs par niveau */
};


/* Representation d'un processus */

PUBLIC struct proc
{
  struct stack_frame context;      /* Le contexte sauvegarde */
  u16_t ldt_selector;              /* Selecteur de la ldt dans la gdt */
  struct seg_desc ldt[LDT_SIZE];   /* LDT du processus */
  u8_t state;                      /* Etat du processus */
  u32_t quantum;                   /* Quantum temps */
  char name[PROC_NAME_LEN];        /* Nom du processus */
  struct skip_node node;           /* Noeud associe dans la skip list */
} __attribute__ ((packed));


/***************
 * Prototypes
 ***************/

PUBLIC struct proc proc_table[PROC_NUM_MAX];  /* Table des processus */
PUBLIC struct proc* proc_current;             /* Processus courant */
PUBLIC struct skip_list proc_ready;           /* Skip list des processus executables */
PUBLIC struct skip_node* NIL;                 /* Element NIL de la skip list */

PUBLIC void sched_init(struct skip_list* list);
PUBLIC void task_index(u32_t* index);
PUBLIC void task_init(struct proc* pr, u32_t index, u32_t base, u32_t size, u8_t priv, u32_t entry_point, u32_t tickets);
PUBLIC void task_schedule();

#endif
