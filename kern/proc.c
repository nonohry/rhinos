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
#include "virtmem_slab.h"
#include "paging.h"
#include "proc.h"


/*========================================================================
 * Declaration Private
 *========================================================================*/


PRIVATE struct vmem_cache* proc_cache;
PRIVATE struct vmem_cache* thread_info_cache;



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


  return EXIT_SUCCESS;
}

