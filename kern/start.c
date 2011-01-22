/*
 * Code de demarrage en C 
 * Recharge GDT et IDT 
 *
 */


/*************
 * Includes 
 *************/

#include "tables.h"
#include "start.h"


/*******************
 * Fonction cstart
 *******************/

PUBLIC void cstart()
{ 
  /* Recopie les informations de demarrage */
  bootinfo = (struct boot_info*)BOOTINFO_ADDR;

  /* Initialise les tables du mode protege */
  gdt_init();
  idt_init();

  return;

}
