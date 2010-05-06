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

  struct skip_node node1;
  struct skip_node node2;
  struct skip_node node3;
  struct skip_node node4;

  node1.tickets = 10;
  node2.tickets = 20;
  node3.tickets = 30;
  node4.tickets = 40;

  sched_init(&proc_ready);
  sched_insert(&proc_ready, &node1);
  sched_insert(&proc_ready, &node2);
  sched_insert(&proc_ready, &node3);
  sched_insert(&proc_ready, &node4);
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
