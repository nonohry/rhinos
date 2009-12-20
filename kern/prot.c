/*
 * Mode Protege
 * Mise en place des tables
 *
 */

#include "types.h"
#include "prot.h"

/****************************** 
 * Declarations preliminaires 
 *****************************/

EXTERN void bochs_print(char*);

/***********************
 * Fonction principale 
 ***********************/
 
PUBLIC void pmode_init()
{
  
  /* Descripteur de GDT */

  gdt_desc.limit = sizeof(gdt) - 1;  /* la GDT commence a 0, d'ou le -1 */
  gdt_desc.base = (u32_t) gdt;       /* Adresse de gdt dans l espace lineaire */

  /* Initialisation de la GDT */
  
  init_code_seg(&gdt[CS_INDEX],(u32_t) 0, KERN_LIMIT, SEG_DPL_0);
  init_data_seg(&gdt[DS_INDEX],(u32_t) 0, KERN_LIMIT, SEG_DPL_0);
  init_data_seg(&gdt[ES_INDEX],(u32_t) 0, KERN_LIMIT, SEG_DPL_0);
  init_data_seg(&gdt[SS_INDEX],(u32_t) 0, KERN_LIMIT, SEG_DPL_0);

  return;
}


/****************************************************
 * Initialisation de segments de code et de donnees
 ****************************************************/

PUBLIC void init_code_seg(struct seg_desc *desc, u32_t base, u32_t size, u32_t dpl)
{

  /* Définit l'adresse de base */

  desc->base_low = base;
  desc->base_middle = base >> 16;
  desc->base_high = base >> 24;

  /* Active la granularite pour les grandes tailles */

  if ((size-1) > GRANULAR_LIMIT)      /* Le descripteur commence a 0, d'ou le -1 */
    {
      desc->limit_low = size >> 12;   /* Divise la taille par 4k (ie shift par 12) */
      desc->granularity = SEG_GRANULAR | size >> (16 + 12)  ; /* Partie haute de la limite */
    }
  else
    {
      desc->limit_low = size;          /* Bits 15:00 de la limite */
      desc->granularity = size >> 16;  /* Bits 19:16 de la limite */
    }

  desc->granularity |= SEG_D;      /* Flag D */

  desc->attributes = dpl | SEG_PRESENT | SEG_DATA_CODE | SEG_ER;  /* Attributes */

  return;

}

PUBLIC void init_data_seg(struct seg_desc *desc, u32_t base, u32_t size, u32_t dpl)
{
  /* Définit l'adresse de base */

  desc->base_low = base;
  desc->base_middle = base >> 16;
  desc->base_high = base >> 24;

  /* Active la granularite pour les grandes tailles */

  if ((size-1) > GRANULAR_LIMIT)      /* Le descripteur commence a 0, d'ou le -1 */
    {
      desc->limit_low = size >> 12;   /* Divise la taille par 4k (ie shift par 12) */
      desc->granularity = SEG_GRANULAR | size >> (16 + 12)  ; /* Partie haute de la limite */
    }
  else
    {
      desc->limit_low = size;          /* Bits 15:00 de la limite */
      desc->granularity = size >> 16;  /* Bits 19:16 de la limite */
    }
  
  desc->granularity |= SEG_B;      /* Flag B */

  desc->attributes = dpl | SEG_PRESENT | SEG_DATA_CODE | SEG_RW; /* Attributes */

  return;

}
