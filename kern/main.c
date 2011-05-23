/*
 * RhinOS Main
 *
 */


/*************
 * Includes 
 *************/

#include <types.h>
#include <llist.h>
#include "start.h"
#include "klib.h"
#include "physmem.h"
#include "paging.h"
#include "irq.h"
#include "pit.h"


/***********************
 * Fonction principale 
 ***********************/

PUBLIC int main()
{
  /* Initialisation de la memoire physique */
  physmem_init();
  bochs_print("Physical Memory Manager initialized\n");

  /* Initialisation de la pagination */
  paging_init();
  bochs_print("Paging enabled\n");

  /* Tentative de mappage */
  u8_t* page=(u8_t*)phys_alloc(4096);
  virtaddr_t virt = 0;
  bochs_print("Tentative de mappage de %x sur %x\n",virt,(physaddr_t)page);
  paging_map((struct pde*)PAGING_GET_PD, virt, (physaddr_t)page,TRUE);


  /* Initialisation du gestionnaire des IRQ */
  irq_init();

  /* Initialisation Horloge */
  //clock_init();
  //bochs_print("Clock initialized (100Hz)\n");

  /* On ne doit plus arriver ici (sauf DEBUG) */
  while(1)
    {
    }

  return EXIT_SUCCESS;
}
