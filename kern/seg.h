/*
 * Header de seg.c
 *
 */

#ifndef SEG_H
#define SEG_H

/*========================================================================
 * Constantes
 *========================================================================*/

/* Masques pour le champ attributes de seg_desc */

#define SEG_PRESENT     0x80 /* 10000000b = 0x80 - Segment Present en memoire */
#define SEG_DPL_0       0x00 /* 00000000b = 0x00 - Niveau de Privilege 0 */
#define SEG_DPL_1       0x20 /* 00100000b = 0x40 - Niveau de Privilege 1 */
#define SEG_DPL_2       0x40 /* 01000000b = 0x40 - Niveau de Privilege 2 */
#define SEG_DPL_3       0x60 /* 01100000b = 0x60 - Niveau de Privilege 3 */
#define SEG_DATA_CODE   0x10 /* 00010000b = 0x10 - Segment de code ou de donnees */

#define SEG_DPL_SHIFT   5    /* Permet de determiner le DPL a partir de 1,2 ou 3 */

#define SEG_RO          0x00 /* Type - cf Doc Intel */
#define SEG_RO_ACC      0x01 
#define SEG_RW          0x02
#define SEG_RW_ACC      0x03
#define SEG_RO_ED       0x04
#define SEG_RO_ED_ACC   0x05
#define SEG_RW_ED       0x06
#define SEG_RW_ED_ACC   0x07

#define SEG_EO          0x08
#define SEG_EO_ACC      0x09 
#define SEG_ER          0x0A
#define SEG_ER_ACC      0x0B
#define SEG_EO_ED       0x0C
#define SEG_EO_ED_ACC   0x0D
#define SEG_ER_ED       0x0E
#define SEG_ER_ED_ACC   0x0F

#define SEG_TSS         0x09
#define SEG_LDT         0x02

/* Masques pour le champs granularity de seg_desc */

#define SEG_LIMIT       0x0F /* 00001111b = 0x0F */
#define SEG_AVL         0x10 /* 00001000b = 0x10 */
#define SEG_D           0x40 /* 01000000b = 0x40 */
#define SEG_B           0x40 /* 01000000b = 0x40 */
#define SEG_GRANULAR    0x80 /* 10000000b = 0x40 */

#define SEG_GRANULAR_LIMIT    0xFFFFFL   /* Pas de granularite au dessous (L pour Long) */


/*========================================================================
 * Structures (cf Doc Intel)
 *========================================================================*/


/* Descripteur de Segment */

PUBLIC struct seg_desc 
{
  u16_t limit_low;
  u16_t base_low;
  u8_t base_middle;
  u8_t attributes;     /* |P|DPL|S| Type |            */
  u8_t granularity;    /* |G|D/B|0|AVL|Seg Lim 19:16| */
  u8_t base_high;
} __attribute__ ((packed));


/* Descripteur de Gate */

PUBLIC struct gate_desc
{
  u16_t offset_low;
  u16_t segment;
  u8_t zero;
  u8_t attributes;   /* |P|DPL|0111?| */
  u16_t offset_high;
} __attribute__ ((packed));


/*========================================================================
 * Prototypes
 *========================================================================*/

PUBLIC void init_code_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl);
PUBLIC void init_data_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl);
PUBLIC void init_int_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags);
PUBLIC void init_trap_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags);
PUBLIC void init_tss_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl);
PUBLIC void init_ldt_seg(struct seg_desc *desc, lineaddr_t base, u32_t size, u8_t dpl);

#endif
