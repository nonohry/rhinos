/*
 * Code de demarrage en C 
 * Recharge GDT et IDT 
 *
 */


/*************
 * Includes 
 *************/

#include <types.h>
#include "klib.h"
#include "prot.h"
#include "start.h"


/*******************
 * Fonction cstart
 *******************/

PUBLIC void cstart(struct boot_info* binfo)
{ 
  u32_t i;
  struct boot_memmap_entry* entry;

  /* Recopie les informations de demarrage */
  bootinfo = *binfo;

  /* Initialise le mode protege (gdt, interruptions ...) */
  pmode_init();

  return;

}
