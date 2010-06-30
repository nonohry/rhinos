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
  u32_t i;

  /* Initialisation Horloge */
  clock_init();
  bochs_print("Clock initialized (100Hz)\n");

  /* Initialisation de l'ordonnancement */
  sched_init(&proc_ready);
  bochs_print("Lottery Scheduling initialized\n");

  /* Idle Task */
  task_init(&proc_table[PROC_IDLE], PROC_IDLE, bootinfo.idle_offset, PROC_IDLE_SIZE, bootinfo.idle_offset+PROC_IDLE_SIZE,1,bootinfo.idle_offset+PROC_IDLE_SIZE+1,1,0, (u32_t)&idle_task,1);

  /* Initialisation du processus courant */
  proc_current = &proc_table[0];

  /* Gestion des taches */
  task_mgmt();

  /* On ne doit plus arriver ici */
  while(1)
    {
    }

  return;
}
