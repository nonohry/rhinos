/*
 * Gestion des threads
 * 
 */


/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include <llist.h>
#include "const.h"
#include "klib.h"
#include "tables.h"
#include "virtmem_slab.h"
#include "virtmem.h"
#include "sched.h"
#include "physmem.h"
#include "paging.h"
#include "thread.h"


/*========================================================================
 * Declaration Private
 *========================================================================*/


PRIVATE struct vmem_cache* thread_cache;
PRIVATE struct vmem_cache* id_info_cache;
PRIVATE s32_t thread_IDs;

PRIVATE void thread_exit(struct thread* th);
PRIVATE void thread_cpu_trampoline(thread_cpu_func_t start_func, void* start_arg, thread_cpu_func_t exit_func, void* exit_arg);


/*========================================================================
 * Initialisation
 *========================================================================*/


PUBLIC u8_t thread_init(void)
{
  u16_t i;

  /* Alloue des caches */
  thread_cache = virtmem_cache_create("thread_cache",sizeof(struct thread),0,0,VIRT_CACHE_DEFAULT,NULL,NULL);
  if (thread_cache == NULL)
    {
      return EXIT_FAILURE;
    }
 
  id_info_cache = virtmem_cache_create("id_info_cache",sizeof(struct id_info),0,0,VIRT_CACHE_DEFAULT,NULL,NULL);
  if (id_info_cache == NULL)
    {
      return EXIT_FAILURE;
    }

  /* Nullifie la hashtable */
  for(i=0;i<THREAD_HASH_SIZE;i++)
    {
      LLIST_NULLIFY(thread_hashID[i]);
    }

  /* Cree une coquille de thread pour le noyau */
  kern_th = (struct thread*)virtmem_cache_alloc(thread_cache,VIRT_CACHE_DEFAULT);
  if ( kern_th == NULL )
    {
      return EXIT_FAILURE;
    }

  kern_th->cpu.ss = CONST_KERN_SS_SELECTOR;

  /* la coquille noyau devient le thread courant */
  cur_th = kern_th;

  /* Affecte les registres de pile du TSS suivant la coquille noyau */
  tss.esp0 = (u32_t)kern_th;
  tss.ss0 = CONST_KERN_SS_SELECTOR;

  /* Initialise le compteur d ID global */
  thread_IDs = 1;

  return EXIT_SUCCESS;
}


/*========================================================================
 * Creation d un thread
 *========================================================================*/


PUBLIC struct thread* thread_create(const char* name, s32_t id, virtaddr_t start_entry, void* start_arg, s8_t nice_level, u8_t quantum, u8_t ring)
{
  struct thread* th;
  struct id_info* thID;
  physaddr_t paddr;
   u8_t i;

  /* Controles */
  if ( (start_entry == 0)
       || (nice_level > THREAD_NICE_TOP)
       || (nice_level < THREAD_NICE_BOTTOM) )
    {
      return NULL;
    }

  /* Rings autorises */
  if ( (ring != CONST_RING0)&&(ring != CONST_RING3) )
    {
      return NULL;
    }
       
  /* Cas ring 3: pas d'arguments */
  if ( (ring==CONST_RING3)&&(start_arg!=NULL) )
    {
      return NULL;
    }

   /* Allocation dans les cache */
  th = (struct thread*)virtmem_cache_alloc(thread_cache,VIRT_CACHE_DEFAULT);
  if ( th == NULL )
    {
      return NULL;
    }
 
  thID = (struct id_info*)virtmem_cache_alloc(id_info_cache,VIRT_CACHE_DEFAULT);
  if (thID == NULL)
    {
      goto err00;
    }
  
  /* Alloue la pile */
  if (ring == CONST_RING0)
    {
      th->stack_base = (virtaddr_t)virt_alloc(CONST_PAGE_SIZE);
      if ((void*)th->stack_base == NULL)
	{
	  goto err01;
	}
    }
  else {
    
    /* Mappage de la pile en ring3 */
    th->stack_base = (virtaddr_t)virtmem_buddy_alloc(CONST_PAGE_SIZE,VIRT_BUDDY_NOMAP);
    if ((void*)th->stack_base == NULL)
      {
	goto err01;
      }

    paddr = (physaddr_t)phys_alloc(CONST_PAGE_SIZE);
    if (!paddr)
      {
	goto err01;
      }
    
    
    if (paging_map(th->stack_base,paddr,PAGING_USER) == EXIT_FAILURE)
      {
	goto err01;
      }
  }


  /* Definit la taille de la pile */
  th->stack_size = CONST_PAGE_SIZE;

  /* Initialise le contexte */
  if (thread_cpu_init((struct cpu_info*)th,start_entry,start_arg,(virtaddr_t)thread_exit,(void*)th,th->stack_base,ring) != EXIT_SUCCESS)
    {
      goto err02;
    }


  /* Copie du nom */
  i=0;
  while( (name[i]!=0)&&(i<THREAD_NAMELEN-1) )
    {
      th->name[i] = name[i];
      i++;
    }
  th->name[i]=0;

  /* Etats */
  th->state = THREAD_READY;
  th->next_state = THREAD_READY;

  /* Nice Level */
  th->nice = nice_level;

  /* Priorite */
  th->sched.static_prio = THREAD_NICE2PRIO(nice_level);
  th->sched.dynamic_prio = th->sched.static_prio;
  th->sched.head_prio = th->sched.static_prio;

  /* Quantum */
  th->sched.static_quantum = quantum;
  th->sched.dynamic_quantum = quantum;

  /* Chainage */
  sched_enqueue(SCHED_READY_QUEUE,th);

  /* ID */
  if (id == THREAD_ID_DEFAULT)
    {
      thID->id = thread_IDs;
      thread_IDs++;
     }
  else
    {
      thID->id = id;
    }
  thID->thread = th;

  /* Back pointer ID */
  th->id = thID;

  /* Chainage ID */
  LLIST_ADD(thread_hashID[THREAD_HASHID_FUNC(thID->id)],thID);

  /* IPC */
  th->ipc.send_to = NULL;
  th->ipc.send_message = NULL;
  th->ipc.receive_from = NULL;
  th->ipc.receive_message = NULL;
  LLIST_NULLIFY(th->ipc.receive_waitlist);


  /* Retour */
  return th;

 err02:
  /* Libere la pile */
  if (ring == CONST_RING0)
    {
      virt_free((void*)th->stack_base);
    }
  else
    {
      /* paging_unmap liberera la page physique */
      paging_unmap((virtaddr_t)th->stack_base);
      virtmem_buddy_free((void*)th->stack_base);
    }

 err01:
  /* Libere le id_info */
  virtmem_cache_free(id_info_cache,thID);

 err00:
  /* Libere le thread */
  virtmem_cache_free(thread_cache,th);

  /* retour */
  return NULL;

}


/*========================================================================
 * Destruction d un thread
 *========================================================================*/


PUBLIC u8_t thread_destroy(struct thread* th)
{
  u16_t i;
  struct id_info* thID;

  /* Controle */
  if ( th == NULL )
    {
      return EXIT_FAILURE;
    }
 
  /* Libere le id_info correspondant */
  i=THREAD_HASHID_FUNC(th->id->id);
  if (!LLIST_ISNULL(thread_hashID[i]))
    {
      thID = LLIST_GETHEAD(thread_hashID[i]);
      do
	{
	  if (thID->thread == th)
	    {
	      LLIST_REMOVE(thread_hashID[i],thID);
	      virtmem_cache_free(id_info_cache,thID);
	      break;
	    }
	  thID = LLIST_NEXT(thread_hashID[i],thID);
	}while(!LLIST_ISHEAD(thread_hashID[i],thID));
    }

  /* Libere simplement les parties allouees */
  return virt_free((void*)th->stack_base) 
    &&  virtmem_cache_free(thread_cache,th);

}


/*========================================================================
 * Initialisation du contexte cpu
 *========================================================================*/


PUBLIC u8_t thread_cpu_init(struct cpu_info* ctx, virtaddr_t start_entry, void* start_arg, virtaddr_t exit_entry, void* exit_arg, virtaddr_t stack_base, u8_t ring)
{
  
  virtaddr_t* esp;
  
  /* Petite verification */
  if ( (start_entry == 0)
       || (exit_entry == 0)
       || (stack_base == 0)
       || (ctx == NULL) )
    {
      return EXIT_FAILURE;
    }

  /* Rings autorises */
  if ( (ring != CONST_RING0)&&(ring != CONST_RING3) )
    {
      return EXIT_FAILURE;
    }
  
  /* Cas ring 3: pas d'arguments */
  if ( (ring==CONST_RING3)&&(start_arg!=NULL) )
    {
      return EXIT_FAILURE;
    }

  /* Nettoie la pile */
  klib_mem_set(0,(addr_t)stack_base,CONST_PAGE_SIZE);
  /* Recupere l'adresse de pile pour empiler les arguments */
  esp = (virtaddr_t*)(stack_base+CONST_PAGE_SIZE/2);

  /* Installe les registres de segments */
  ctx->cs = (ring == CONST_RING0 ? CONST_KERN_CS_SELECTOR : CONST_USER_CS_SELECTOR);
  ctx->ds = (ring == CONST_RING0 ? CONST_KERN_DS_SELECTOR : CONST_USER_DS_SELECTOR);
  ctx->es = (ring == CONST_RING0 ? CONST_KERN_ES_SELECTOR : CONST_USER_ES_SELECTOR);
  ctx->ss = (ring == CONST_RING0 ? CONST_KERN_SS_SELECTOR : CONST_USER_SS_SELECTOR);

  /* Positionne un faux code d erreur */
  ctx->error_code = THREAD_CPU_FEC;

  /* Active les interruptions */
  ctx->eflags = (1<<THREAD_CPU_INTFLAG_SHIFT);


  /* Pointe EIP sur le point d entree */
  ctx->eip = (reg32_t)start_entry;

  /* Utilisation d un trampoline (ring0) */
  if (ring == CONST_RING0)
    {
      /* Pointe EIP sur le trampoline */
      ctx->eip = (reg32_t)thread_cpu_trampoline;

      /* Arguments du trampoline */
      *(--esp) = (virtaddr_t)exit_arg;
      *(--esp) = exit_entry;
      *(--esp) = (virtaddr_t)start_arg;
      *(--esp) = start_entry;
  
      /* Fausse adresse de retour pour la fonction de trampoline */
      *(--esp) = 0;
    

      /* Simule une pile interrompue (le switch passe par une interruption logicielle sans changement de pile pour le ring0) */
      *(--esp) = ctx->eflags;
      *(--esp) = CONST_KERN_CS_SELECTOR;
      *(--esp) = ctx->eip;
      *(--esp) = ctx->error_code;
      *(--esp) = 0;
    }
  /* Installe la pile */
  ctx->esp = (reg32_t)esp;


  return EXIT_SUCCESS;
}


/*========================================================================
 * Bascule de thread courant
 *========================================================================*/


PUBLIC void thread_switch_to(struct thread* th)
{
  /* Affecte le thread courant */
  if (th != NULL)
    {
      cur_th = th;
      
      /* Positionne le tss */
      tss.esp0 = (u32_t)th;

    }

  return;
}


/*========================================================================
 * Post traitement de la sauvegarde du contexte
 *========================================================================*/


PUBLIC void thread_cpu_postsave(struct thread* th, reg32_t* esp)
{

  
  /* Traitement si pas changement de privileges (Note: SS est sur 16bits) */
  if ((th->cpu.ss & 0xFF) == CONST_KERN_SS_SELECTOR)
    {
      /* Recupere les registres oublies */
      cur_th->cpu.ret_addr = *(esp);
      cur_th->cpu.error_code = *(esp+1);
      cur_th->cpu.eip = *(esp+2);
      cur_th->cpu.cs = *(esp+3);
      cur_th->cpu.eflags = *(esp+4);
      cur_th->cpu.esp = (reg32_t)(esp);
      cur_th->cpu.ss = CONST_KERN_SS_SELECTOR;
    }
   
  return;
}


/*========================================================================
 * Trampoline de lancement
 *========================================================================*/


PRIVATE void thread_cpu_trampoline(thread_cpu_func_t start_func, void* start_arg, thread_cpu_func_t exit_func, void* exit_arg)
{
  /* Trampoline ! */
  start_func(start_arg);
  exit_func(exit_arg);

  /* Pas de retour normalement*/
  return;
}


/*========================================================================
 * Recherche d un thread via son id_info
 *========================================================================*/


PUBLIC struct thread* thread_id2thread(s32_t n)
{
  u16_t i;
  struct id_info* thID;

  /* Controle */
  if ( n == 0 )
    {
      return NULL;
    }

  /* Parcours de la hashtable */
  i=THREAD_HASHID_FUNC(n);
  if (!LLIST_ISNULL(thread_hashID[i]))
    {
      thID = LLIST_GETHEAD(thread_hashID[i]);
      do
	{
	  /* On trouve le bon ID, on renvoie le thread */
	  if (thID->id == n)
	    {
	      return thID->thread;
	    }
	  thID = LLIST_NEXT(thread_hashID[i],thID);
	}while(!LLIST_ISHEAD(thread_hashID[i],thID));
    }
  
  /* Ici, on ne trouve rien, on retourn NULL */
  return NULL;
}


/*========================================================================
 * Sortie d un thread
 *========================================================================*/


PRIVATE void thread_exit(struct thread* th)
{
  /* Controle */
  if ( th == NULL )
    {
      return;
    }

  /* Nouvel etat */
  th->next_state = THREAD_DEAD;

  /* DEBUG TEMPORAIRE: Attend une interruption ... */
  while(1){}

  return;
}



