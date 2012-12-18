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
 * Macros 
 *========================================================================*/


#define MIN(_x,_y)     ((_x)<(_y)?(_x):(_y))
#define MAX(_x,_y)     ((_x)>(_y)?(_x):(_y))


/*========================================================================
 * Declarations Private 
 *========================================================================*/


PRIVATE u8_t start_e820_sanitize(struct boot_info* bootinfo);
PRIVATE u8_t start_e820_truncate32b(struct boot_info* bootinfo);
PRIVATE u8_t start_e801_generate(struct boot_info* bootinfo);
PRIVATE u8_t start_e88_generate(struct boot_info* bootinfo);


/*========================================================================
 * Fonction start_main
 *========================================================================*/


PUBLIC void start_main(u32_t magic, physaddr_t mbi_addr)
{ 

  u8_t i;
  struct multiboot_info* mbi;

  /* Initialise le port serie pour la sortie noyau */
  klib_serial_init();

  klib_printf("Booting with magic 0x%x and mbi addr 0x%x\n",magic,mbi_addr);
  mbi = (struct multiboot_info*)mbi_addr;

  if (mbi->flags & START_MULTIBOOT_FLAG_MEMORY)
    {
      klib_printf("Mem lower : %d Ko - Mem upper : %d Ko\n",mbi->mem_lower,mbi->mem_upper);
    }

  /* DEBUG */
  while(1){}

  /* Recopie les informations de demarrage */
  bootinfo = (struct boot_info*)mbi;

  /* Genere un memory map si besoins */
  if (!bootinfo->mem_map_count)
    {
      if ( (bootinfo->mem_upper)&&(bootinfo->mem_lower) )
	{      
	  if (start_e801_generate(bootinfo) != EXIT_SUCCESS)
	    {
	      goto err_mem;
	    }
	}
      else if ( (bootinfo->mem_0x0)&&(bootinfo->mem_0x100000) )
	{
	  if (start_e88_generate(bootinfo) != EXIT_SUCCESS)
	    {
	      goto err_mem;
	    }
	}
      else
	{
	  goto err_mem;
	}
    }

  /* Corrige les eventuels chevauchements */
  if (start_e820_sanitize(bootinfo) != EXIT_SUCCESS)
    {
      goto err_mem;
    }

  /* Tronque a 4G */
  if (start_e820_truncate32b(bootinfo) != EXIT_SUCCESS)
    {
      goto err_mem;
    }

  /* Calcul la memoire totale corrigee */
  bootinfo->mem_total = 0;
  for(i=0;i<bootinfo->mem_map_count;i++)
    {

      /* HACK TEST USER */
      if ( (u32_t)(((struct boot_mmap_e820*)bootinfo->mem_map_addr)[i].size) == 0x9F000)
	{
	  (((struct boot_mmap_e820*)bootinfo->mem_map_addr)[i].size) = 0x9E000;
	}
      
      bootinfo->mem_total += ((struct boot_mmap_e820*)bootinfo->mem_map_addr)[i].size;
    }

  /* Initialise les tables du mode protege */
  if ( (gdt_init() != EXIT_SUCCESS)||(idt_init() != EXIT_SUCCESS) ) 
    {
      goto err_tables;
    }
  klib_printf("GDT & IDT initialized\n");

  return;

 err_mem:
  klib_printf("Memory Error ! Aborting...\n");

 err_tables:
  klib_printf("GDT & IDT Error\n");

  while(1){}

  return;
}



/*========================================================================
 * Assainit le memory map e820
 *========================================================================*/



PRIVATE u8_t start_e820_sanitize(struct boot_info* bootinfo)
{
  u8_t i,j,flag;
  u64_t A,B,C,D;
  struct boot_mmap_e820* mmap;
  struct boot_mmap_e820 tmpEntry;

  mmap = (struct boot_mmap_e820*)bootinfo->mem_map_addr;

  /* Parcours glouton de la table */
  for(i=0;i<bootinfo->mem_map_count-1;i++)
    {
      /* Extremite du segment */
      A=mmap[i].addr;
      B=mmap[i].size+mmap[i].addr-1;

      for(j=i+1;j<bootinfo->mem_map_count;j++)
	{
	  /* (Re)Initialisation du flag */
	  flag = 3;

	  /* Extremites du segment compare */
	  C=mmap[j].addr;
	  D=mmap[j].size+mmap[j].addr-1;

	  /* Overlap */
	  if ((s64_t)(MIN(B,D)-MAX(A,C))>0)
	    {
	      /* Differenciation des cas: flag represente le degre de confonte des segments */
	      if (A!=C) flag&=2;
	      if (B!=D) flag&=1;

	      switch(flag)
		{
		case 0:
		  {
		    /* Creation de 3 segments */

		    if (bootinfo->mem_map_count+1 < START_E820_MAX)
		      {
			u8_t t0,t1,t2;
			
			/* Types pour chaque segment */
			t0 = (MIN(A,C) == A ? mmap[i].type : mmap[j].type);
			t1 = MAX(mmap[i].type,mmap[j].type);
			t2 = (MAX(B,D) == B ? mmap[i].type : mmap[j].type);

			mmap[i].addr = MIN(A,C);
			mmap[i].size = MAX(A,C)-MIN(A,C);
			mmap[i].type = t0;

			mmap[j].addr = MAX(A,C);
			mmap[j].size = MIN(B,D)-MAX(A,C)+1;
			mmap[j].type = t1;

			mmap[bootinfo->mem_map_count].addr = MIN(B,D)+1;
			mmap[bootinfo->mem_map_count].size = MAX(B,D)-MIN(B,D);
			mmap[bootinfo->mem_map_count].type = t2;

			bootinfo->mem_map_count++; 
		      }
		    else
		      {
			/* Trop d'overlaps */
			return EXIT_FAILURE;
		      }

		    break;
		  }
		case 1:
		  {
		    /* Creation de 2 segments, 1ere extremite commune */
		    u8_t t0,t1;
			
		    /* Types pour chaque segment */
		    t0 = MAX(mmap[i].type,mmap[j].type);
		    t1 = (MAX(B,D) == B ? mmap[i].type : mmap[j].type);

		    mmap[i].addr = MAX(A,C);
		    mmap[i].size = MIN(B,D)-MAX(A,C)+1;
		    mmap[i].type = t0;
		    
		    mmap[j].addr = MIN(B,D)+1;
		    mmap[j].size = MAX(B,D)-MIN(B,D);
		    mmap[j].type = t1;
		    

		    break;
		  }
		case 2:
		  {
		    /* Creation de 2 segments, 2eme extremite commune */
		    u8_t t0,t1;

		    t0 = (MIN(A,C) == A ? mmap[i].type : mmap[j].type);
		    t1 = MAX(mmap[i].type,mmap[j].type);
		    
		    mmap[i].addr = MIN(A,C);
		    mmap[i].size = MAX(A,C)-MIN(A,C);
		    mmap[i].type = t0;

		    mmap[j].addr = MAX(A,C);
		    mmap[j].size = MIN(B,D)-MAX(A,C)+1;
		    mmap[j].type = t1;


		    break;
		  }
		case 3:
		  {
		    /* Segments confondus, retrait du segment de plus faible type */
		    u8_t k;

		    /* Prend le type le plus haut */
		    mmap[i].type = MAX(mmap[i].type,mmap[j].type);
		    
		    /* Supprime le segment confondu */
		    for(k=j;k<bootinfo->mem_map_count-1;k++)
		      {
			mmap[k]=mmap[k+1];
		      }
		    
		    bootinfo->mem_map_count--;

		    break;
		  }
		default:
		  {
		    return EXIT_FAILURE;
		  }
		}
	      
	      /* Refait les calculs avec l entree modifiee */
	      i--;
	      break;

	    }
	}
    }

  /* Tri a bulle ascendant du memory map */
  do
    {
      flag=0;
      for(i=0;i<bootinfo->mem_map_count-1;i++)
	{
	  if (mmap[i].addr > mmap[i+1].addr)
	    {
	      /* Echange des entrees */

	      tmpEntry.addr=mmap[i].addr;
	      tmpEntry.size=mmap[i].size;
	      tmpEntry.type=mmap[i].type;

	      mmap[i].addr=mmap[i+1].addr;
	      mmap[i].size=mmap[i+1].size;
	      mmap[i].type=mmap[i+1].type;

	      mmap[i+1].addr=tmpEntry.addr;
	      mmap[i+1].size=tmpEntry.size;
	      mmap[i+1].type=tmpEntry.type;

	      flag=1;

	    }
	}
    }while(flag);


  /* Lissage du memory map */
  for(i=0;i<bootinfo->mem_map_count-1;i++)
    {
      /* Fusion des segments s ils se suivent et sont de meme type */
      if ( (mmap[i].addr+mmap[i].size == mmap[i+1].addr)&&(mmap[i].type == mmap[i+1].type) )
	{
	  /* Ajuste la taille */
	  mmap[i].size = mmap[i].size+mmap[i+1].size;

	  /* Supprime le segment suivant */
	  for(j=i+1;j<bootinfo->mem_map_count-1;j++)
	    {
	      mmap[j]=mmap[j+1];
	    }

	  /* Met a jour le nombre d entrees */
	  bootinfo->mem_map_count--;

	  /* Recalcul avec la nouvelle entree */
	  i--;

	}
    }

  return EXIT_SUCCESS;
}



/*========================================================================
 * Tronque la memoire a 4G dans un memory map trie
 *========================================================================*/


PRIVATE u8_t start_e820_truncate32b(struct boot_info* bootinfo)
{
  u8_t i;
  struct boot_mmap_e820* mmap;

  mmap = (struct boot_mmap_e820*)bootinfo->mem_map_addr;

  for(i=0;i<bootinfo->mem_map_count;i++)
    {

      /* Marque la memoire au dela de 4G comme reservee */
      if ( (mmap[i].addr>>32)&&(mmap[i].type == START_E820_AVAILABLE) )
	{
	  mmap[i].type = START_E820_RESERVED;
	}
      else
	{
	  /* Cas de l'entree disponible a cheval */
	  if ( ((mmap[i].addr+mmap[i].size)>>32)&&(mmap[i].type == START_E820_AVAILABLE) )
	    {
	      mmap[i].size = (u32_t)(-1) - mmap[i].addr;
	    }
	}
      
    }
  
  return EXIT_SUCCESS;
}




/************************************************
 * Genere un memory map depuis int 0x15 ax=0e801 
 ************************************************/


PRIVATE u8_t start_e801_generate(struct boot_info* bootinfo)
{
  struct boot_mmap_e820* entry;
  
  /* Cree un faux memory map */
  bootinfo->mem_map_count = 4;
  
  entry = (struct boot_mmap_e820*)bootinfo->mem_map_addr;
  entry->addr = 0;
  entry->size = CONST_ROM_AREA_START;
  entry->type = START_E820_AVAILABLE;

  entry = (struct boot_mmap_e820*)(bootinfo->mem_map_addr+sizeof(struct boot_mmap_e820));
  entry->addr = CONST_ROM_AREA_START;
  entry->size = CONST_ROM_AREA_SIZE;
  entry->type = START_E820_RESERVED;
  
  entry = (struct boot_mmap_e820*)(bootinfo->mem_map_addr+2*sizeof(struct boot_mmap_e820));
  entry->addr = CONST_ROM_AREA_START+CONST_ROM_AREA_SIZE+1;
  entry->size = bootinfo->mem_lower << 10;
  entry->type = START_E820_AVAILABLE;
  
  entry = (struct boot_mmap_e820*)(bootinfo->mem_map_addr+3*sizeof(struct boot_mmap_e820));
  entry->addr = 0x1000000;
  entry->size = bootinfo->mem_upper << 16;
  entry->type = START_E820_AVAILABLE;
  
  return EXIT_SUCCESS;
}



/************************************************
 * Genere un memory map depuis int 0x15 ax=0e88 
 ************************************************/


PRIVATE u8_t start_e88_generate(struct boot_info* bootinfo)
{
  struct boot_mmap_e820* entry;
  
  /* Taille totale */
  bootinfo->mem_total = (bootinfo->mem_0x0 + bootinfo->mem_0x100000) << 10;
  
  /* Cree un faux memory map */
  bootinfo->mem_map_count = 3;

  entry = (struct boot_mmap_e820*)bootinfo->mem_map_addr;
  entry->addr = 0;
  entry->size = bootinfo->mem_0x0;
  entry->type = START_E820_AVAILABLE;

  /* Possible chevauchement, corrige ensuite via e820_sanitize */
  entry = (struct boot_mmap_e820*)(bootinfo->mem_map_addr+sizeof(struct boot_mmap_e820));
  entry->addr = CONST_ROM_AREA_START;
  entry->size = CONST_ROM_AREA_SIZE;
  entry->type = START_E820_RESERVED;

  
  entry = (struct boot_mmap_e820*)(bootinfo->mem_map_addr+2*sizeof(struct boot_mmap_e820));
  entry->addr = CONST_ROM_AREA_START+CONST_ROM_AREA_SIZE+1;
  entry->size = bootinfo->mem_0x100000 << 10;
  entry->type = START_E820_AVAILABLE;
  
  return EXIT_SUCCESS;
}
