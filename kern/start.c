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

PUBLIC void cstart(struct boot_info* binfo)
{ 
  u16_t i;
  u64_t mem;


  /* Recopie les informations de demarrage */
  bootinfo = binfo;
  mem = 0;

  /* Calcul la taille de la memoire */
  if (bootinfo->mem_map_count)
    {
      /* Taille selon int 15/AX=E820 */
      struct boot_mmap_e820* entry;
      for(entry=(struct boot_mmap_e820*)bootinfo->mem_map_addr,i=0;i<bootinfo->mem_map_count;i++,entry++)
	{
	  mem += entry->size;
	}
    }
  else
    {
      /* On arrete la si pas de memory map  */
      bochs_print("Unable to get memory size !\n");
      while(1){}
    }

  /* Limite a 4G */
  if (mem > START_MEM_LIMIT)
    {
      bootinfo->mem_total = (u32_t)START_MEM_LIMIT;
    }
  else
    {
      bootinfo->mem_total = (u32_t)mem;
    }

  /* Initialise les tables du mode protege */
  gdt_init();
  idt_init();

  return;

}
