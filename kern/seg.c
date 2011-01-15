/*
 * Initialisation des differents segments
 */

#include <types.h>
#include "seg.h"


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