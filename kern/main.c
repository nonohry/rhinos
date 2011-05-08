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

PUBLIC void main()
{
  /* Initialisation de la memoire physique */
  physmem_init();
  bochs_print("Physical Memory Manager initialized\n");

  /* Initialisation du gestionnaire des IRQ */
  irq_init();

  /* Initialisation Horloge */
  clock_init();
  bochs_print("Clock initialized (100Hz)\n");

  /* On ne doit plus arriver ici (sauf DEBUG) */
  while(1)
    {
    }

  return;
}
