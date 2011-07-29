/*
 * Mode Protege
 * Mise en place des tables
 *
 */


/*========================================================================
 * Includes 
 *========================================================================*/

#include <types.h>
#include "klib.h"
#include "pic.h"
#include "seg.h"
#include "tables.h"


/*========================================================================
 * Initialisation de la GDT 
 *========================================================================*/
 
PUBLIC void gdt_init()
{
  
  bochs_print("Creating the new GDT...\n");

  /* Descripteur de GDT */

  gdt_desc.limit = sizeof(gdt) - 1;  /* la GDT commence a 0, d'ou le -1 */
  gdt_desc.base = (lineaddr_t) gdt;       /* Adresse de gdt dans l espace lineaire */

  /* Initialisation de la GDT */
  
  init_code_seg(&gdt[TABLES_CS_INDEX],(lineaddr_t) TABLES_KERN_BASE, TABLES_KERN_LIMIT_4G, TABLES_RING0);
  init_data_seg(&gdt[TABLES_XS_INDEX],(lineaddr_t) TABLES_KERN_BASE, TABLES_KERN_LIMIT_4G, TABLES_RING0);

  return;
}

/*========================================================================
 * Initialisation de l IDT 
 *========================================================================*/

PUBLIC void idt_init()
{

  /* Initialisation du PIC i8259 */
  pic_init();
  bochs_print("PIC i8259 initialized\n");

  bochs_print("Creating the IDT...\n");

  /* Descripteur de IDT */

  idt_desc.limit = sizeof(idt) - 1;  /* l IDT commence a 0, d'ou le -1 */
  idt_desc.base = (lineaddr_t) idt;       /* Adresse de idt dans l espace lineaire */

  /* Initialisation de l IDT - Exceptions */

  init_int_gate(&idt[0] , TABLES_CS_SELECTOR, (lineaddr_t)excep_00, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[1] , TABLES_CS_SELECTOR, (lineaddr_t)excep_01, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[2] , TABLES_CS_SELECTOR, (lineaddr_t)excep_02, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[3] , TABLES_CS_SELECTOR, (lineaddr_t)excep_03, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[4] , TABLES_CS_SELECTOR, (lineaddr_t)excep_04, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[5] , TABLES_CS_SELECTOR, (lineaddr_t)excep_05, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[6] , TABLES_CS_SELECTOR, (lineaddr_t)excep_06, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[7] , TABLES_CS_SELECTOR, (lineaddr_t)excep_07, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[8] , TABLES_CS_SELECTOR, (lineaddr_t)excep_08, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[9] , TABLES_CS_SELECTOR, (lineaddr_t)excep_09, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[10], TABLES_CS_SELECTOR, (lineaddr_t)excep_10, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[11], TABLES_CS_SELECTOR, (lineaddr_t)excep_11, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[12], TABLES_CS_SELECTOR, (lineaddr_t)excep_12, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[13], TABLES_CS_SELECTOR, (lineaddr_t)excep_13, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[14], TABLES_CS_SELECTOR, (lineaddr_t)excep_14, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[16], TABLES_CS_SELECTOR, (lineaddr_t)excep_16, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[17], TABLES_CS_SELECTOR, (lineaddr_t)excep_17, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[18], TABLES_CS_SELECTOR, (lineaddr_t)excep_18, SEG_PRESENT | SEG_DPL_0);

  /* Initialisation de l IDT - IRQ */
  
  init_int_gate(&idt[32], TABLES_CS_SELECTOR, (lineaddr_t)hwint_00, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[33], TABLES_CS_SELECTOR, (lineaddr_t)hwint_01, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[34], TABLES_CS_SELECTOR, (lineaddr_t)hwint_02, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[35], TABLES_CS_SELECTOR, (lineaddr_t)hwint_03, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[36], TABLES_CS_SELECTOR, (lineaddr_t)hwint_04, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[37], TABLES_CS_SELECTOR, (lineaddr_t)hwint_05, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[38], TABLES_CS_SELECTOR, (lineaddr_t)hwint_06, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[39], TABLES_CS_SELECTOR, (lineaddr_t)hwint_07, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[40], TABLES_CS_SELECTOR, (lineaddr_t)hwint_08, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[41], TABLES_CS_SELECTOR, (lineaddr_t)hwint_09, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[42], TABLES_CS_SELECTOR, (lineaddr_t)hwint_10, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[43], TABLES_CS_SELECTOR, (lineaddr_t)hwint_11, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[44], TABLES_CS_SELECTOR, (lineaddr_t)hwint_12, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[45], TABLES_CS_SELECTOR, (lineaddr_t)hwint_13, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[46], TABLES_CS_SELECTOR, (lineaddr_t)hwint_14, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[47], TABLES_CS_SELECTOR, (lineaddr_t)hwint_15, SEG_PRESENT | SEG_DPL_0);  

  return;
}


