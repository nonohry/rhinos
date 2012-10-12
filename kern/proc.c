/*
 * Gestion des processus
 *
 */



/*========================================================================
 * Includes
 *========================================================================*/


#include <types.h>
#include <llist.h>
#include "const.h"
#include "thread.h"
#include "klib.h"
#include "physmem.h"
#include "virtmem_slab.h"
#include "paging.h"
#include "proc.h"


/*========================================================================
 * Declaration Private
 *========================================================================*/


PRIVATE struct vmem_cache* proc_cache;
PRIVATE struct vmem_cache* thread_info_cache;
PRIVATE struct vmem_cache* pd_cache;


/*========================================================================
 * Initialisation
 *========================================================================*/


PUBLIC u8_t proc_init(void)
{

  /* Alloue les caches */

  proc_cache = virtmem_cache_create("proc_cache",sizeof(struct proc),0,0,VIRT_CACHE_DEFAULT,NULL,NULL);
  if (proc_cache == NULL)
    {
      return EXIT_FAILURE;
    }
 
  thread_info_cache = virtmem_cache_create("thread_info_cache",sizeof(struct thread_info),0,0,VIRT_CACHE_DEFAULT,NULL,NULL);
  if (thread_info_cache == NULL)
    {
      return EXIT_FAILURE;
    }

  pd_cache = virtmem_cache_create("pd_cache",PAGING_ENTRIES*sizeof(struct pde),0,0,VIRT_CACHE_DEFAULT,NULL,NULL);
  if (pd_cache == NULL)
    {
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}



/*========================================================================
 * Creation d'un processus
 *========================================================================*/


PUBLIC struct proc* proc_create(char* name)
{
  struct proc* proc;
  struct pde*  pd;
  struct pde* kern_pd;
  u16_t i;

  /* Allocation du processus */
  proc = (struct proc*)virtmem_cache_alloc(proc_cache,VIRT_CACHE_DEFAULT);
  if (proc == NULL)
    {
      return NULL;
    }

  /* Allocation du page directory */
  pd = (struct pde*)virtmem_cache_alloc(pd_cache,VIRT_CACHE_DEFAULT);
  if (!pd)
    {
      goto err00;
    }

  /* Petit controle du nom */
  if (name == NULL)
    {
      name = "NONAME";
    }

  /* Copie du nom */
  i=0;
  while( (name[i]!=0)&&(i<PROC_NAMELEN-1) )
    {
      proc->name[i] = name[i];
      i++;
    }
  proc->name[i]=0;

  /* Initialisation de la liste de threads */
  LLIST_NULLIFY(proc->thread_list);

  /* Nettoie le page directory */
  klib_mem_set(0,(addr_t)pd,PAGING_ENTRIES*sizeof(struct pde));

  /* Synchronise le page directory avec l espace noyau */
  kern_pd = (struct pde*)PAGING_GET_PD();
  for(i=0;i<CONST_KERN_HIGHMEM/CONST_PAGE_SIZE/PAGING_ENTRIES;i++)
    {
      pd[i]=kern_pd[i];
    }

  /* Affecte le page directory */
  proc->v_pd = pd;
  proc->p_pd = paging_virt2phys((virtaddr_t)pd);
  if (!(proc->p_pd))
    {
      goto err01;
    }

  /* Self Mapping */
  proc->v_pd[PAGING_SELFMAP].present = 1;
  proc->v_pd[PAGING_SELFMAP].rw = 1;
  proc->v_pd[PAGING_SELFMAP].user = 0;
  proc->v_pd[PAGING_SELFMAP].baseaddr = proc->p_pd >> PAGING_BASESHIFT;

  return proc;

 err01:
  /* Libere le page directory */
  virtmem_cache_free(pd_cache,pd);

 err00:
  /* Libere le processus */
   virtmem_cache_free(proc_cache,proc);
  

  return NULL;
}


/*========================================================================
 * Ajout d'un thread a un processus
 *========================================================================*/


PUBLIC u8_t proc_add_thread(struct proc* proc, struct thread* th)
{
  struct thread_info* thinfo;

  if ( (th == NULL) || (proc == NULL) )
    {
      return EXIT_FAILURE;
    }

  /* Alloue un thread_info */
  thinfo = (struct thread_info*)virtmem_cache_alloc(thread_info_cache,VIRT_CACHE_DEFAULT);
  if (thinfo == NULL)
    {
      return EXIT_FAILURE;
    }

  /* Back pointer du thread sur le processus */
  th->proc = proc;
  /* Affectation du thread au thread info */
  thinfo->thread = th;
  /* Liaison aux threads du processus */
  LLIST_ADD(proc->thread_list,thinfo);

  return EXIT_SUCCESS;
}


/*========================================================================
 * Retrait d'un thread d un processus
 *========================================================================*/


PUBLIC u8_t proc_remove_thread(struct proc* proc, struct thread* th)
{
  struct thread_info* thinfo;

  if ( (th == NULL) || (proc == NULL) || LLIST_ISNULL(proc->thread_list) )
    {
      return EXIT_FAILURE;
    }

  /* Cherche le thread_info correspondant */
  thinfo = LLIST_GETHEAD(proc->thread_list);
  do
    {
      if (thinfo->thread == th)
	{
	  /* Enleve et libere le thread_info */
	  LLIST_REMOVE(proc->thread_list, thinfo);
	  virtmem_cache_free(thread_info_cache,thinfo);
	  return EXIT_SUCCESS;
	}
      thinfo = LLIST_NEXT(proc->thread_list,thinfo);
    }while(!LLIST_ISHEAD(proc->thread_list,thinfo));
    

  return EXIT_FAILURE;
}
