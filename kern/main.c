/*
 * RhinOS Main
 *
 */


/*========================================================================
 * Includes 
 *========================================================================*/

#include <types.h>
#include <llist.h>
#include "start.h"
#include "klib.h"
#include "physmem.h"
#include "paging.h"
#include "irq.h"
#include "pit.h"
#include "virtmem_buddy.h"
#include "virtmem_slab.h"


/*========================================================================
 * Fonction principale 
 *========================================================================*/

PUBLIC int main()
{



  /* Initialisation de la memoire physique */
  phys_init();
  bochs_print("Physical Memory Manager initialized\n");

  /* Initialisation de la pagination */
  paging_init();
  bochs_print("Paging enabled\n");

  /* Initialisation de la memoire virtuelle */
  virtmem_cache_init();
  virtmem_buddy_init();
  bochs_print("Virtual Memory Manager initialized\n");

  /* Initialisation du gestionnaire des IRQ */
  irq_init();

  /* Initialisation Horloge */
  //pit_init();
  //bochs_print("Clock initialized (100Hz)\n");

  /* On ne doit plus arriver ici (sauf DEBUG) */
  while(1)
    {
    }

  return EXIT_SUCCESS;
}
