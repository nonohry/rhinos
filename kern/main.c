/*
 * RhinOS Main
 *
 */


/*************
 * Includes 
 *************/

#include <types.h>
#include "start.h"
#include "klib.h"
#include "i82C54.h"
#include "proc.h"


/**********
 * EXTERN
 **********/

EXTERN void task_mgmt();

/***********************
 * Fonction principale 
 ***********************/

PUBLIC void main()
{
 
  /* Initialisation Horloge */
  clock_init();
  bochs_print("Clock initialized (100Hz)\n");

  /* Initialisation de l'ordonnancement */
  sched_init(&proc_ready);
  bochs_print("Lottery Scheduling initialized\n");

  /* Initialisation du processus courant */
  /* proc_current = &proc_table[0]; */

  /* Gestion des taches */
  /* task_mgmt(); */

  /* On ne doit plus arriver ici (sauf DEBUG) */
  while(1)
    {
    }

  return;
}
