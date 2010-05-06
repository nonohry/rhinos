/*
 * RhinOS Main
 *
 */


/*************
 * Includes 
 *************/

#include "types.h"
#include "klib.h"
#include "i82C54.h"
#include "proc.h"

/*********
 * DEBUG
 *********/

PUBLIC void dummy();

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

  proc_table[0].node.tickets = 10;
  proc_table[1].node.tickets = 20;
  proc_table[2].node.tickets = 30;
  proc_table[3].node.tickets = 40;

  sched_init(&proc_ready);
  sched_insert(&proc_ready, &(proc_table[0].node));
  sched_insert(&proc_ready, &(proc_table[1].node));
  sched_insert(&proc_ready, &(proc_table[2].node));
  sched_insert(&proc_ready, &(proc_table[3].node));
  sched_print(&proc_ready);
  sched_delete(&proc_ready, 30);
  sched_print(&proc_ready);

  /* Initialisation de la dummy task */
  task_init(&proc_table[0], 0xF00000, 1024, 3, (u32_t)&dummy);
  proc_current = &proc_table[0];

  /* Gestion des taches */
  task_mgmt();

  /* On ne doit plus arriver ici */
  while(1)
    {
    }

  return;
}


/**************
 * Dummy task
 **************/

PUBLIC void dummy()
{
  while(1)
    {
    }

  return;
}
