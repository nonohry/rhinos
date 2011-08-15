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
      struct boot_mmap_e820* entry;

      /* Taille selon int 15/AX=E820 */
      for(entry=(struct boot_mmap_e820*)bootinfo->mem_map_addr,i=0;i<bootinfo->mem_map_count;i++,entry++)
	{
	  mem += entry->size;
	  
	  /* Evite la memoire au dela de 4G */
	  if ( (entry->addr >= START_MEM_LIMIT)&&(entry->type == START_E820_AVAILABLE) )
	    {
	      entry->type = START_E820_RESERVED;
	    }

	  /* Retrecit si la taille est au dela de 4G */
	  if ( (entry->addr < START_MEM_LIMIT)&&(entry->type == START_E820_AVAILABLE)&&(entry->addr+entry->size > START_MEM_LIMIT) )
	    {
	      entry->size = START_MEM_LIMIT - entry->addr;
	    }

	}

      /* Limite a 4G */
      bootinfo->mem_total = ( mem > START_MEM_LIMIT ? (u32_t)START_MEM_LIMIT : (u32_t)mem ); 

    }
  else if ( (bootinfo->mem_upper)&&(bootinfo->mem_lower) )
    {
      struct boot_mmap_e820* entry[3];

      /* Taille totale */
      bootinfo->mem_total = START_MEM_SIZE_0 + (bootinfo->mem_lower << 10) + (bootinfo->mem_upper << 16);

      /* Cree un faux memory map */
      bootinfo->mem_map_count = 3;

      entry[0] = (struct boot_mmap_e820*)bootinfo->mem_map_addr;
      entry[0]->addr = 0;
      entry[0]->size = 0x9FC00;
      entry[0]->type = START_E820_AVAILABLE;
      
      entry[1] = (struct boot_mmap_e820*)(bootinfo->mem_map_addr+sizeof(struct boot_mmap_e820));
      entry[1]->addr = 0x100000;
      entry[1]->size = bootinfo->mem_lower << 10;
      entry[1]->type = START_E820_AVAILABLE;

      entry[2] = (struct boot_mmap_e820*)(bootinfo->mem_map_addr+2*sizeof(struct boot_mmap_e820));
      entry[2]->addr = 0x1000000;
      entry[2]->size = bootinfo->mem_upper << 16;
      entry[2]->type = START_E820_AVAILABLE;
      
    }
  else if ( (bootinfo->mem_0x0)&&(bootinfo->mem_0x100000) )
    {
      struct boot_mmap_e820* entry[2];

      /* Taille totale */
      bootinfo->mem_total = (bootinfo->mem_0x0 + bootinfo->mem_0x100000) << 10;

      /* Cree un faux memory map */
      bootinfo->mem_map_count = 2;

      entry[0] = (struct boot_mmap_e820*)bootinfo->mem_map_addr;
      entry[0]->addr = 0;
      entry[0]->size = (bootinfo->mem_0x0 < 0x9FC00 ? bootinfo->mem_0x0 : 0x9FC00) ;
      entry[0]->type = START_E820_AVAILABLE;
      
      entry[1] = (struct boot_mmap_e820*)(bootinfo->mem_map_addr+sizeof(struct boot_mmap_e820));
      entry[1]->addr = 0x100000;
      entry[1]->size = bootinfo->mem_0x100000 << 10;
      entry[1]->type = START_E820_AVAILABLE;

    }
  else
    {
      /* Erreur memoire  */
      bochs_print("Unable to get memory size or not enough memory !\n");
      while(1){}
    }

  /* Initialise les tables du mode protege */
  gdt_init();
  idt_init();

  return;

}
