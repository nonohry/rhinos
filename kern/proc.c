/*
 * Gestion des processus
 *
 */

#include "types.h"
#include "klib.h"
#include "prot.h"
#include "proc.h"

/***************************
 * Creation d un processus 
 ***************************/

PUBLIC void task_init(struct proc* pr, u32_t base, u32_t data_code, u32_t stack, u8_t priv, u32_t entry_point)
{

  /* Cree les segments */
  init_code_seg(&(pr->ldt[LDT_CS_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_DS_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_ES_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_FS_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_GS_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_SS_INDEX]), base+data_code, stack, priv);

  /* Affecte les registres de segments */
  pr->context.cs = LDT_CS_SELECTOR | priv;
  pr->context.ds = LDT_DS_SELECTOR | priv;
  pr->context.es = LDT_ES_SELECTOR | priv;
  pr->context.fs = LDT_FS_SELECTOR | priv;
  pr->context.gs = LDT_GS_SELECTOR | priv;
  pr->context.ss = LDT_SS_SELECTOR | priv;

  /* Copie le code au bon endroit */
  phys_copy(entry_point, base, data_code);
  
  /* Positionne les registres nécessaires */
  pr->context.esp = stack;   /* La pile */ 
  pr->context.eip = 0;       /* Pointeur d instructions */
  pr->context.eflags = 512;  /* Interrupt Enable Flag */

  /* Le selecteur de la LDT */
  init_ldt_seg(&gdt[LDT_INDEX],(u32_t) &(pr->ldt[0]), sizeof(pr->ldt), 0);
  pr->ldt_selector = LDT_INDEX << SHIFT_SELECTOR;

  return;
}
