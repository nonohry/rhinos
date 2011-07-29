/*
 * Code de demarrage en C 
 * Recharge GDT et IDT 
 *
 */


/*========================================================================
 * Includes 
 *========================================================================*/

#include "klib.h"
#include "tables.h"
#include "start.h"


/*========================================================================
 * Fonction cstart
 *========================================================================*/

PUBLIC void cstart()
{ 
  u16_t i;

  /* Recopie les informations de demarrage */
  bootinfo = (struct boot_info*)START_BOOTINFO_ADDR;


  /* Calcul la taille de la memoire */
  if (bootinfo->mem_entry)
    {
      /* Taille selon int 15/AX=E820 */
      struct boot_mmap_e820* entry;
      for(entry=(struct boot_mmap_e820*)bootinfo->mem_addr,i=0;i<bootinfo->mem_entry;i++,entry++)
	{
	  bootinfo->mem_size += (u32_t)entry->size;
	}
    }
  else
    {
      /* On arrete la si la taille n est pas disponible */
      bochs_print("Unable to get memory size !\n");
      while(1){}
    }

  /* Initialise les tables du mode protege */
  gdt_init();
  idt_init();

  return;

}
