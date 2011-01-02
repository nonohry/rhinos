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

  /* Recupere le memory map s'il existe */
  entry = (struct boot_memmap_entry*)bootinfo.mem_addr;

  for(i=0;i<bootinfo.mem_entry;i++)
    {
      bochs_print("%x%x | %x%x | %x\n",entry->base_addr_up, entry->base_addr_low, entry->size_up, entry->size_low, entry->type);
      entry ++;
    }

  /* Initialise le mode protege (gdt, interruptions ...) */
  pmode_init();

  return;

}
