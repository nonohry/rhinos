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

PUBLIC void proc_allocate(struct proc* pr, u32_t base, u32_t data_code, u32_t stack, u8_t priv)
{

  /* Cree les segments */
  init_code_seg(&(pr->ldt[LDT_CS_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_DS_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_ES_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_FS_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_GS_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_SS_INDEX]), base+data_code, stack, priv);

  /* Affecte les registres de segments */
  pr->context.cs = LDT_CS_SELECTOR;
  pr->context.ds = LDT_DS_SELECTOR;
  pr->context.es = LDT_ES_SELECTOR;
  pr->context.fs = LDT_FS_SELECTOR;
  pr->context.gs = LDT_GS_SELECTOR;
  pr->context.ss = LDT_SS_SELECTOR;

  return;
}
