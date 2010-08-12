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
  /* Recopie les informations de demarrage */
  bootinfo = *binfo;

  /* Initialise le mode protege (gdt, interruptions ...) */
  pmode_init();

  return;

}
