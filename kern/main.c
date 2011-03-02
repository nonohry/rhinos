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
#include "pit.h"
#include "bootmem.h"


/***********************
 * Fonction principale 
 ***********************/

PUBLIC void main()
{
  /* Initialisation de la memoire physique */
  bootmem_init();
  bochs_print("Boot Memory Manager initialized\n");

  /* Initialisation Horloge */
  clock_init();
  bochs_print("Clock initialized (100Hz)\n");
	
  /* On ne doit plus arriver ici (sauf DEBUG) */
  while(1)
    {
    }

  return;
}
