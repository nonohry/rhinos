#ifndef PROT_H
#define PROT_H

/*
 * Includes 
 *
 */

#include "types.h"

/*
 * Constantes
 *
 */


/* Indexes de la GDT */

#define NULL_INDEX     0
#define CS_INDEX       1
#define DS_INDEX       2
#define ES_INDEX       3
#define SS_INDEX       4
#define LDT_INDEX      5     /* Index de la premiere LDT */

/* Taille de la GDT */

#define GDT_SIZE       LDT_INDEX

/* Selecteurs de segment */

#define CS_SELECTOR	8    /* CS = 00000001  0  00   = (byte) 8  */
#define	DS_SELECTOR     16   /* DS = 00000010  0  00   = (byte) 16 */
#define	ES_SELECTOR	24   /* ES = 00000011  0  00   = (byte) 24 */
#define	SS_SELECTOR	32   /* SS = 00000100  0  00   = (byte) 32 */


/*
 * Structures (cf Doc Intel)
 *
 */


/* Descripteur de Segment */

struct seg_desc 
{
  u16_t limit_low;
  u16_t base_low;
  u8_t base_middle;
  u8_t attributes;     /* |P|DPL|S| Type |            */
  u8_t granularity;    /* |G|D/B|0|AVL|Seg Lim 19:16| */
  u8_t base_high;
}__attribute__ ((packed));


/* Descripteur de Table (GDT & LDT) */

struct table_desc
{
  u8_t limit;
  u32_t base;
}__attribute__ ((packed));

#endif
