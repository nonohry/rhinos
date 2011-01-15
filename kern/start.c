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

PUBLIC void cstart(struct boot_info* binfo)
{ 
  /* Recopie les informations de demarrage */
  bootinfo = *binfo;

  /* Initialise les tables du mode protege */
  gdt_init();
  idt_init();

  return;

}
