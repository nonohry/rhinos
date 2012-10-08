/*
 * Mode Protege
 * Mise en place des tables
 *
 */


/*========================================================================
 * Includes 
 *========================================================================*/

#include <types.h>
#include "const.h"
#include "klib.h"
#include "pic.h"
#include "seg.h"
#include "interrupt.h"
#include "tables.h"


/*========================================================================
 * Initialisation de la GDT 
 *========================================================================*/
 
PUBLIC u8_t gdt_init()
{
  /* Descripteur de GDT */

  gdt_desc.limit = sizeof(gdt) - 1;  /* la GDT commence a 0, d'ou le -1 */
  gdt_desc.base = (lineaddr_t) gdt;       /* Adresse de gdt dans l espace lineaire */

  /* Initialisation de la GDT: segments noyau */
  
  init_code_seg(&gdt[TABLES_KERN_CS_INDEX],(lineaddr_t) TABLES_KERN_BASE, TABLES_KERN_LIMIT_4G, CONST_RING0);
  init_data_seg(&gdt[TABLES_KERN_XS_INDEX],(lineaddr_t) TABLES_KERN_BASE, TABLES_KERN_LIMIT_4G, CONST_RING0);

  /* Initialisation de la GDT: segments utilsateur */

  init_code_seg(&gdt[TABLES_USER_CS_INDEX],(lineaddr_t) TABLES_USER_BASE, TABLES_USER_LIMIT_4G, CONST_RING3);
  init_data_seg(&gdt[TABLES_USER_XS_INDEX],(lineaddr_t) TABLES_USER_BASE, TABLES_USER_LIMIT_4G, CONST_RING3);

  /* Initialisation de la GDT: TSS */

  init_tss_seg(&gdt[TABLES_TSS_INDEX], (lineaddr_t)&tss, sizeof(tss), CONST_RING0);

  return EXIT_SUCCESS;
}

/*========================================================================
 * Initialisation de l IDT 
 *========================================================================*/

PUBLIC u8_t idt_init()
{

  /* Initialisation du PIC i8259 */
  if (pic_init() != EXIT_SUCCESS)
    {
      return EXIT_FAILURE;
    }


  /* Descripteur de IDT */

  idt_desc.limit = sizeof(idt) - 1;  /* l IDT commence a 0, d'ou le -1 */
  idt_desc.base = (lineaddr_t) idt;       /* Adresse de idt dans l espace lineaire */

  /* Initialisation de l IDT - Exceptions */

  init_int_gate(&idt[0] , CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_00, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[1] , CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_01, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[2] , CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_02, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[3] , CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_03, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[4] , CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_04, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[5] , CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_05, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[6] , CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_06, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[7] , CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_07, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[8] , CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_08, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[9] , CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_09, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[10], CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_10, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[11], CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_11, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[12], CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_12, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[13], CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_13, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[14], CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_14, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[16], CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_16, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[17], CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_17, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[18], CONST_KERN_CS_SELECTOR, (lineaddr_t)excep_18, SEG_PRESENT | SEG_DPL_0);

  /* Initialisation de l IDT - IRQ */
  
  init_int_gate(&idt[32], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_00, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[33], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_01, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[34], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_02, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[35], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_03, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[36], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_04, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[37], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_05, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[38], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_06, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[39], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_07, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[40], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_08, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[41], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_09, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[42], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_10, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[43], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_11, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[44], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_12, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[45], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_13, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[46], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_14, SEG_PRESENT | SEG_DPL_0);
  init_int_gate(&idt[47], CONST_KERN_CS_SELECTOR, (lineaddr_t)hwint_15, SEG_PRESENT | SEG_DPL_0);

  init_int_gate(&idt[50], CONST_KERN_CS_SELECTOR, (lineaddr_t)swint_syscall, SEG_PRESENT | SEG_DPL_3);

  return EXIT_SUCCESS;
}


