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


PRIVATE u8_t start_mmap_sanitize(struct multiboot_info* bootinfo);
PRIVATE u8_t start_mmap_truncate32b(struct multiboot_info* bootinfo);


/*========================================================================
 * Declarations Static
 *========================================================================*/


static struct multiboot_mmap_entry start_mmap[START_MULTIBOOT_MMAP_MAX];


/*========================================================================
 * Fonction start_main
 *========================================================================*/


PUBLIC void start_main(u32_t magic, physaddr_t mbi_addr)
{ 

  u8_t i;
  struct multiboot_mmap_entry* mmap;

  /* Initialise le port serie pour la sortie noyau */
  klib_serial_init();

  /* Affecte la structure multiboot */
  start_mbi = (struct multiboot_info*)mbi_addr;

  /* Copie les entrees mmap dans un lieu maitrise si presence d un mmap */
  if (start_mbi->flags & START_MULTIBOOT_FLAG_MMAP)
    {
      for(mmap = (struct multiboot_mmap_entry*)start_mbi->mmap_addr, i=0;
	  (unsigned long)mmap <  start_mbi->mmap_addr + start_mbi->mmap_length;
	  i++, mmap = (struct multiboot_mmap_entry*)((unsigned long)mmap + mmap->size + sizeof(mmap->size)))
	{
	
	  start_mmap[i].size = mmap->size;
	  start_mmap[i].addr = mmap->addr;
	  start_mmap[i].len = mmap->len;
	  start_mmap[i].type = mmap->type;

	  /* Trop d'entrees (buggy mmap) */
	  if (i>START_MULTIBOOT_MMAP_MAX)
	    {
	       goto err_mem;
	    }
	  
	}
    }
  else
    {
      /* Construit un memory map avec les informations upper/lower */
      if (start_mbi->flags & START_MULTIBOOT_FLAG_MEMORY)
	{
	  /* Nombre d'entrees */
	  i=3;
	  
	  /* Creation d'un mmap */

	  start_mmap[0].size = sizeof(struct multiboot_mmap_entry);
	  start_mmap[0].addr = 4096;
	  start_mmap[0].len = start_mbi->mem_lower*1024;
	  start_mmap[0].type = START_E820_AVAILABLE;

	  start_mmap[1].size = sizeof(struct multiboot_mmap_entry);
	  start_mmap[1].addr = CONST_ROM_AREA_START;
	  start_mmap[1].len = CONST_ROM_AREA_SIZE;
	  start_mmap[1].type = START_E820_RESERVED;


	  start_mmap[2].size = sizeof(struct multiboot_mmap_entry);
	  start_mmap[2].addr = 0x100000;
	  start_mmap[2].len = start_mbi->mem_upper*1024;
	  start_mmap[2].type = START_E820_AVAILABLE;
	  
	}
      else
	{
	  /* Aucune information memoire, on quitte */
	   goto err_mem;
	}
    }

  /* Pointe vers la copie */
  start_mbi->mmap_addr = (u32_t)start_mmap;
  /* Nombre d'entrees */
  start_mbi->mmap_length = i;


  /* Corrige les eventuels chevauchements dans le  mmap */
  if ( start_mmap_sanitize(start_mbi) != EXIT_SUCCESS )
    {
      goto err_mem;
    }


  /* Tronque a 4G si besoin */
  if ( start_mmap_truncate32b(start_mbi) != EXIT_SUCCESS )
    {
      goto err_mem;
    }    

  /* Calcule la memoire totale */
  start_mem_total = 0;
  mmap = (struct multiboot_mmap_entry*)start_mbi->mmap_addr;
  for(i=0;i<start_mbi->mmap_length;i++)
    {
      start_mem_total += mmap[i].len;
      
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



PRIVATE u8_t start_mmap_sanitize(struct multiboot_info* bootinfo)
{
  u8_t i,j,flag;
  u64_t A,B,C,D;
  struct multiboot_mmap_entry* mmap;
  struct multiboot_mmap_entry tmpEntry;

  mmap = (struct multiboot_mmap_entry*)bootinfo->mmap_addr;

  /* Parcours glouton de la table */
  for(i=0;i<bootinfo->mmap_length-1;i++)
    {
      /* Extremite du segment */
      A=mmap[i].addr;
      B=mmap[i].len+mmap[i].addr-1;

      for(j=i+1;j<bootinfo->mmap_length;j++)
	{
	  /* (Re)Initialisation du flag */
	  flag = 3;

	  /* Extremites du segment compare */
	  C=mmap[j].addr;
	  D=mmap[j].len+mmap[j].addr-1;

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

		    if (bootinfo->mmap_length+1 < START_MULTIBOOT_MMAP_MAX)
		      {
			u8_t t0,t1,t2;
			
			/* Types pour chaque segment */
			t0 = (MIN(A,C) == A ? mmap[i].type : mmap[j].type);
			t1 = MAX(mmap[i].type,mmap[j].type);
			t2 = (MAX(B,D) == B ? mmap[i].type : mmap[j].type);

			mmap[i].addr = MIN(A,C);
			mmap[i].len = MAX(A,C)-MIN(A,C);
			mmap[i].type = t0;

			mmap[j].addr = MAX(A,C);
			mmap[j].len = MIN(B,D)-MAX(A,C)+1;
			mmap[j].type = t1;

			mmap[bootinfo->mmap_length].addr = MIN(B,D)+1;
			mmap[bootinfo->mmap_length].len = MAX(B,D)-MIN(B,D);
			mmap[bootinfo->mmap_length].type = t2;

			bootinfo->mmap_length++; 
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
		    mmap[i].len = MIN(B,D)-MAX(A,C)+1;
		    mmap[i].type = t0;
		    
		    mmap[j].addr = MIN(B,D)+1;
		    mmap[j].len = MAX(B,D)-MIN(B,D);
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
		    mmap[i].len = MAX(A,C)-MIN(A,C);
		    mmap[i].type = t0;

		    mmap[j].addr = MAX(A,C);
		    mmap[j].len = MIN(B,D)-MAX(A,C)+1;
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
		    for(k=j;k<bootinfo->mmap_length-1;k++)
		      {
			mmap[k]=mmap[k+1];
		      }
		    
		    bootinfo->mmap_length--;

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
      for(i=0;i<bootinfo->mmap_length-1;i++)
	{
	  if (mmap[i].addr > mmap[i+1].addr)
	    {
	      /* Echange des entrees */

	      tmpEntry.addr=mmap[i].addr;
	      tmpEntry.len=mmap[i].len;
	      tmpEntry.type=mmap[i].type;

	      mmap[i].addr=mmap[i+1].addr;
	      mmap[i].len=mmap[i+1].len;
	      mmap[i].type=mmap[i+1].type;

	      mmap[i+1].addr=tmpEntry.addr;
	      mmap[i+1].len=tmpEntry.len;
	      mmap[i+1].type=tmpEntry.type;

	      flag=1;

	    }
	}
    }while(flag);


  /* Lissage du memory map */
  for(i=0;i<bootinfo->mmap_length-1;i++)
    {
      /* Fusion des segments s ils se suivent et sont de meme type */
      if ( (mmap[i].addr+mmap[i].len == mmap[i+1].addr)&&(mmap[i].type == mmap[i+1].type) )
	{
	  /* Ajuste la taille */
	  mmap[i].len = mmap[i].len+mmap[i+1].len;

	  /* Supprime le segment suivant */
	  for(j=i+1;j<bootinfo->mmap_length-1;j++)
	    {
	      mmap[j]=mmap[j+1];
	    }

	  /* Met a jour le nombre d entrees */
	  bootinfo->mmap_length--;

	  /* Recalcul avec la nouvelle entree */
	  i--;

	}
    }

  return EXIT_SUCCESS;
}



/*========================================================================
 * Tronque la memoire a 4G dans un memory map trie
 *========================================================================*/


PRIVATE u8_t start_mmap_truncate32b(struct multiboot_info* bootinfo)
{
  u8_t i;
  struct multiboot_mmap_entry* mmap;

  mmap = (struct multiboot_mmap_entry*)bootinfo->mmap_addr;

  for(i=0;i<bootinfo->mmap_length;i++)
    {

      /* Marque la memoire au dela de 4G comme reservee */
      if ( (mmap[i].addr>>32)&&(mmap[i].type == START_E820_AVAILABLE) )
	{
	  mmap[i].type = START_E820_RESERVED;
	}

      /* Cas de l'entree disponible a cheval */
      if ((!(mmap[i].addr>>32))&& ((mmap[i].addr+mmap[i].len)>>32)&&(mmap[i].type == START_E820_AVAILABLE) )
	{
	  mmap[i].len = (u32_t)(-1) - mmap[i].addr;
	}

      
    }
  
  return EXIT_SUCCESS;
}

