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

#endif
