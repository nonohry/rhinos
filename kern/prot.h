#ifndef PROT_H
#define PROT_H

/*************
 * Includes 
 *************/

#include <types.h>

/**************
 * Constantes
 **************/


/* Indexes de la GDT */

#define NULL_INDEX     0
#define CS_INDEX       1
#define DS_INDEX       2
#define ES_INDEX       3
#define SS_INDEX       4
#define TSS_INDEX      5
#define LDT_INDEX      6     /* Index de la premiere LDT */
#define MAX_INDEX      8192  /* Nombre maximum d index d une GDT */

/* Taille de la GDT & IDT */

#define GDT_SIZE       MAX_INDEX-1  /* Debute a 0 */
#define IDT_SIZE       255

/* Selecteurs de segment */

#define CS_SELECTOR	8    /*  CS = 0000000000001  0  00   =  8  */
#define	DS_SELECTOR     16   /*  DS = 0000000000010  0  00   =  16 */
#define	ES_SELECTOR	24   /*  ES = 0000000000011  0  00   =  24 */
#define	SS_SELECTOR	32   /*  SS = 0000000000100  0  00   =  32 */
#define TSS_SELECTOR    40   /* TSS = 0000000000101  0  00   =  40 */

#define SHIFT_SELECTOR  3    /* INDEX << SHIFT_SELECTOR = SELECTOR */

/* Rings */

#define RING0   0
#define RING1   1
#define RING2   2
#define RING3   3

/* RPL pour les selecteurs de segments */

#define RPL_1   1
#define RPL_2   2
#define RPL_3   3

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

/* Limite des segments  */

#define GRANULAR_LIMIT    0xFFFFFL   /* Pas de granularite au dessous (L pour Long) */
#define KERN_LIMIT        0x0        /* Limite de l'espace Noyau */
#define KERN_TOP_STACK    0x7C00     /* ESP Noyau au boot */

/* IRQs */

#define IRQ_VECTORS     16

/*****************************
 * Structures (cf Doc Intel)
 *****************************/


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


/* Descripteur de Table (GDT & LDT) */

PUBLIC struct table_desc
{
  u16_t limit;
  u32_t base;
} __attribute__ ((packed));

/* TSS */

PUBLIC struct tss
{
  u16_t previous;
  u16_t zero_0;
  u32_t esp0;
  u16_t ss0;
  u16_t zero_1;
  u32_t esp1;
  u16_t ss1;
  u16_t zero_2;
  u32_t esp2;
  u16_t ss2;
  u16_t zero_3;
  u32_t cr3;
  u32_t eip;
  u32_t eflags;
  u32_t eax;
  u32_t ecx;
  u32_t edx;
  u32_t ebx;
  u32_t esp;
  u32_t ebp;
  u32_t esi;
  u32_t edi;
  u16_t es;
  u16_t zero_4;
  u16_t cs;
  u16_t zero_5;
  u16_t ss;
  u16_t zero_6;
  u16_t ds;
  u16_t zero_7;
  u16_t fs;
  u16_t zero_8;
  u16_t gs;
  u16_t zero_9;
  u16_t ldt;
  u16_t zero_10;
  u16_t debug;
  u16_t iomap;
} __attribute__ ((packed));

/* Liste chainee des handlers d'IRQs */

PUBLIC struct irq_chaine
{
  u8_t irq;                  /* IRQ */
  struct irq_chaine* next;   /* Chaine */
  u8_t (*handler)(void);     /* ISR */
  u32_t id;                  /* ID  */
} __attribute__ ((packed));

/* Alias pour le pointeur d'ISR */

typedef u8_t (*irq_handler_t)();

/******************
 * ISR assembleur
 ******************/

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


/**************
 * Prototypes 
 **************/

PUBLIC struct seg_desc gdt[GDT_SIZE];  /* GDT */
PUBLIC struct gate_desc idt[IDT_SIZE]; /* IDT */
PUBLIC struct tss tss;                 /* TSS */
PUBLIC struct table_desc gdt_desc;     /* Descripteur de la GDT */
PUBLIC struct table_desc idt_desc;     /* Descripteur de l'IDT */
PUBLIC struct irq_chaine* irq_handlers[IRQ_VECTORS];  /* Tableau des irq handlers */
PUBLIC u32_t  irq_active[IRQ_VECTORS];                /* Tableau des bitmaps d'irq actives */


PUBLIC void pmode_init();
PUBLIC void init_code_seg(struct seg_desc *desc, u32_t base, u32_t size, u8_t dpl);
PUBLIC void init_data_seg(struct seg_desc *desc, u32_t base, u32_t size, u8_t dpl);
PUBLIC void init_int_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags);
PUBLIC void init_trap_gate(struct gate_desc* gate, u16_t seg, u32_t off,u8_t flags);
PUBLIC void init_tss_seg(struct seg_desc *desc, u32_t base, u32_t size, u8_t dpl);
PUBLIC void init_ldt_seg(struct seg_desc *desc, u32_t base, u32_t size, u8_t dpl);

#endif
