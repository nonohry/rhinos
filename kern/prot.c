/*
 * Mode Protege
 * Mise en place des tables
 *
 */


/*************
 * Includes 
 *************/

#include "types.h"
#include "klib.h"
#include "i8259.h"
#include "prot.h"


/***********************
 * Fonction principale 
 ***********************/
 
PUBLIC void pmode_init()
{
  
  bochs_print("Creating the new GDT...\n");

  /* Initialisation du TSS */

  tss.ss0 = SS_SELECTOR;
  tss.esp0 = 0xC0000;

  /* Descripteur de GDT */

  gdt_desc.limit = sizeof(gdt) - 1;  /* la GDT commence a 0, d'ou le -1 */
  gdt_desc.base = (u32_t) gdt;       /* Adresse de gdt dans l espace lineaire */

  /* Initialisation de la GDT */
  
  init_code_seg(&gdt[CS_INDEX],(u32_t) 0, KERN_LIMIT, 0);
  init_data_seg(&gdt[DS_INDEX],(u32_t) 0, KERN_LIMIT, 0);
  init_data_seg(&gdt[ES_INDEX],(u32_t) 0, KERN_LIMIT, 0);
  init_data_seg(&gdt[SS_INDEX],(u32_t) 0, KERN_LIMIT, 0);
  init_tss_seg(&gdt[TSS_INDEX],(u32_t) &tss, sizeof(tss), 0);

  /* Initialisation du PIC i8259 */
  i8259_init();
  bochs_print("PIC i8259 initialized\n");

  bochs_print("Creating the IDT...\n");

  /* Descripteur de IDT */

  idt_desc.limit = sizeof(idt) - 1;  /* l IDT commence a 0, d'ou le -1 */
  idt_desc.base = (u32_t) idt;       /* Adresse de idt dans l espace lineaire */


  /* Initialisation de l IDT - Exceptions */

  init_int_gate(&idt[0], CS_SELECTOR, (u32_t)excep_00, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[1], CS_SELECTOR, (u32_t)excep_01, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[2], CS_SELECTOR, (u32_t)excep_02, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[3], CS_SELECTOR, (u32_t)excep_03, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[4], CS_SELECTOR, (u32_t)excep_04, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[5], CS_SELECTOR, (u32_t)excep_05, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[6], CS_SELECTOR, (u32_t)excep_06, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[7], CS_SELECTOR, (u32_t)excep_07, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[8], CS_SELECTOR, (u32_t)excep_08, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[9], CS_SELECTOR, (u32_t)excep_09, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[10], CS_SELECTOR, (u32_t)excep_10, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[11], CS_SELECTOR, (u32_t)excep_11, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[12], CS_SELECTOR, (u32_t)excep_12, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[13], CS_SELECTOR, (u32_t)excep_13, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[14], CS_SELECTOR, (u32_t)excep_14, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[16], CS_SELECTOR, (u32_t)excep_16, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[17], CS_SELECTOR, (u32_t)excep_17, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[18], CS_SELECTOR, (u32_t)excep_18, SEG_PRESENT | SEG_DPL_0);

  /* Initialisation de l IDT - IRQ */
  
  init_int_gate(&idt[32], CS_SELECTOR, (u32_t)hwint_00, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[33], CS_SELECTOR, (u32_t)hwint_01, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[34], CS_SELECTOR, (u32_t)hwint_02, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[35], CS_SELECTOR, (u32_t)hwint_03, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[36], CS_SELECTOR, (u32_t)hwint_04, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[37], CS_SELECTOR, (u32_t)hwint_05, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[38], CS_SELECTOR, (u32_t)hwint_06, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[39], CS_SELECTOR, (u32_t)hwint_07, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[40], CS_SELECTOR, (u32_t)hwint_08, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[41], CS_SELECTOR, (u32_t)hwint_09, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[42], CS_SELECTOR, (u32_t)hwint_10, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[43], CS_SELECTOR, (u32_t)hwint_11, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[44], CS_SELECTOR, (u32_t)hwint_12, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[45], CS_SELECTOR, (u32_t)hwint_13, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[46], CS_SELECTOR, (u32_t)hwint_14, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[47], CS_SELECTOR, (u32_t)hwint_15, SEG_PRESENT | SEG_DPL_0);  

  return;
}


/****************************************************
 * Initialisation de segments de code et de donnees
 ****************************************************/

PUBLIC void init_code_seg(struct seg_desc *desc, u32_t base, u32_t size, u8_t dpl)
{

  /* Définit l'adresse de base */

  desc->base_low = base;
  desc->base_middle = base >> 16;
  desc->base_high = base >> 24;

  /* Active la granularite pour les grandes tailles */
  size--;                         /* 0 signifie 4G */
  if (size > GRANULAR_LIMIT)      /* Le descripteur commence a 0, d'ou le -1 */
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

  desc->attributes = (dpl << SEG_DPL_SHIFT) | SEG_PRESENT | SEG_DATA_CODE | SEG_ER;  /* Attributes */

  return;

}

PUBLIC void init_data_seg(struct seg_desc *desc, u32_t base, u32_t size, u8_t dpl)
{
  /* Définit l'adresse de base */

  desc->base_low = base;
  desc->base_middle = base >> 16;
  desc->base_high = base >> 24;

  /* Active la granularite pour les grandes tailles */
  size--;                         /* 0 signifie 4G */
  if (size > GRANULAR_LIMIT)      /* Le descripteur commence a 0, d'ou le -1 */
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

  desc->attributes = (dpl << SEG_DPL_SHIFT) | SEG_PRESENT | SEG_DATA_CODE | SEG_RW; /* Attributes */

  return;

}

/**********************************************
 * Initialisation des interrupt et trap gates
 **********************************************/

PUBLIC void init_int_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags)
{
  u8_t magic = 14;  /* 14 = 00001110b */

  /* Offset */
  gate->offset_low = off;
  gate->offset_high = off >> 16;

  /* Segment du code */
  gate->segment = seg;

  /* 1 octet nul */
  gate->zero = 0;

  /* Attributes */
  gate->attributes = magic | flags;

  return;
}

PUBLIC void init_trap_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags)
{
  u8_t magic = 15;  /* 15 = 00001111b */

  /* Offset */
  gate->offset_low = off;
  gate->offset_high = off >> 16;

  /* Segment du code */
  gate->segment = seg;

  /* 1 octet nul */
  gate->zero = 0;

  /* Attributes */
  gate->attributes = magic | flags;

  return;
}

/***************************
 * Initialisation des TSS
 ***************************/

PUBLIC void init_tss_seg(struct seg_desc *desc, u32_t base, u32_t size, u8_t dpl)
{

  /* Définit l'adresse de base */

  desc->base_low = base;
  desc->base_middle = base >> 16;
  desc->base_high = base >> 24;

  /* Active la granularite pour les grandes tailles */
  size--;                         /* 0 signifie 4G */
  if (size > GRANULAR_LIMIT)      /* Le descripteur commence a 0, d'ou le -1 */
    {
      desc->limit_low = size >> 12;   /* Divise la taille par 4k (ie shift par 12) */
      desc->granularity = SEG_GRANULAR | size >> (16 + 12)  ; /* Partie haute de la limite */
    }
  else
    {
      desc->limit_low = size;          /* Bits 15:00 de la limite */
      desc->granularity = size >> 16;  /* Bits 19:16 de la limite */
    }

  desc->attributes = (dpl << SEG_DPL_SHIFT) | SEG_PRESENT | SEG_TSS;  /* Attributes */

  return;

}

/***************************
 * Initialisation des LDT
 ***************************/

PUBLIC void init_ldt_seg(struct seg_desc *desc, u32_t base, u32_t size, u8_t dpl)
{

  /* Définit l'adresse de base */

  desc->base_low = base;
  desc->base_middle = base >> 16;
  desc->base_high = base >> 24;

  /* Active la granularite pour les grandes tailles */
  size--;                         /* 0 signifie 4G */
  if (size > GRANULAR_LIMIT)      /* Le descripteur commence a 0, d'ou le -1 */
    {
      desc->limit_low = size >> 12;   /* Divise la taille par 4k (ie shift par 12) */
      desc->granularity = SEG_GRANULAR | size >> (16 + 12)  ; /* Partie haute de la limite */
    }
  else
    {
      desc->limit_low = size;          /* Bits 15:00 de la limite */
      desc->granularity = size >> 16;  /* Bits 19:16 de la limite */
    }

  desc->attributes = (dpl << SEG_DPL_SHIFT) | SEG_PRESENT | SEG_LDT;  /* Attributes */

  return;

}
