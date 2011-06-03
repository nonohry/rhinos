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



  /* Tentative de mappage a chaud de main() */

  u8_t* new_ppage =(u8_t*)phys_alloc(4096); 
  virtaddr_t tmp_vaddr=0x7000;
  virtaddr_t main_vaddr=PAGING_ALIGN_INF((virtaddr_t)main);

  /* Map la page physique avec l adresse temporaire */
  paging_map(tmp_vaddr,(physaddr_t)new_ppage,TRUE);
  /* Copie (en virtuel) les donnees des pages */
  mem_copy(main_vaddr,tmp_vaddr,PAGE_SIZE);
  /* Map la page de main sur la nouvelle page physique */
  paging_map(main_vaddr,(physaddr_t)new_ppage, TRUE);
  /* Unmap l adresse temporaire */
  paging_unmap(tmp_vaddr);
  
  /* vide le tlb */
  invlpg(tmp_vaddr);
  invlpg(main_vaddr);  
  /* flush_tlb(); */





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
