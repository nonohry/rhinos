/*
 * Gestion des processus
 *
 */

#include <types.h>
#include "klib.h"
#include "prot.h"
#include "proc.h"
#include "elf.h"

/************************************
 * Declaration PRIVATE
 ************************************/

PRIVATE void sched_insert(struct skip_list* list, struct skip_node* node);
PRIVATE void sched_delete(struct skip_list* list, u32_t key);
PRIVATE void sched_search(struct skip_list* list, u32_t key, u32_t* index);

/**************************************
 * Initialisation de l ordonnancement
 **************************************/

PUBLIC void sched_init(struct skip_list* list)
{
  u32_t i;

  /* Initialisation de la cle de NIL*/
  NIL->key = 0xFFFFFFFE;

  /* Niveau intial = 0 */
  list->level = 0;
  list->tickets = 0; 

  /* Tous les successeurs du header sont NIL */
 for(i=0; i<SKIP_MAX_LEVEL; i++)
   {
     list->update[i] = list->header;
     list->header->forwards[i] = NIL;
   }

 return;
}

/******************************
 * Insertion dans la skip_list
 ******************************/

PRIVATE void sched_insert(struct skip_list* list, struct skip_node* node)
{
  int i;
  u32_t lvl;
  
  /* Calcule la key */
  node->key = node->tickets + list->tickets;

  /* Choisit un niveau */
  lvl = random()%SKIP_MAX_LEVEL;
  
  /* Si le niveau est superieur au niveau courant, on l ajuste*/
  if (lvl > list->level) list->level = lvl;

  /* On met à jour ses prédécesseurs et ses succeseeurs */
  for(i=0; i<=lvl; i++)
    {
      list->update[i]->forwards[i] = node;
      node->forwards[i] = NIL;
      list->update[i] = node;
    }

  /* Ajuste le total de ticket */
  list->tickets += node->tickets;

  return;
}


/********************************
 * Suppression dans la Skip_list
 ********************************/

PRIVATE void sched_delete(struct skip_list* list, u32_t key)
{
  struct skip_node* update[SKIP_MAX_LEVEL];
  struct skip_node* x;
  struct skip_node* y;
  int i;
  
  /* Part du header pour trouver la clé */
  x=list->header;
  
  for(i=list->level; i>=0; i--)
    {
      /* Trouve la clé précédant celle à enlever */
      while (x->forwards[i]->key < key)
	{
	  x = x->forwards[i];
	}
      /* Sauve l'element precedent du niveau i pour le mettre a jour */
      update[i] = x;
    }
  
  /* En theorie, le noeud a enlever est le successeur de la derniere cle trouvée */
  x = x->forwards[0];
  
  if(x->key == key)
    {
      for(i=0; i<=list->level; i++)
       {
         /* On s'arrete s il n y a plus de x */
         if (update[i]->forwards[i] != x )
           {
             break;
           }
         /* Court circuite le noeud à oter */
         update[i]->forwards[i] = x->forwards[i];
       }

      /* Met a jour les cles des elements qui suivent */
      y = x->forwards[0];
      while(y != NIL)
	{
	  y->key -= x->tickets;
	  y = y->forwards[0];
	}
     
     /* Si l element etait le plus haut niveau, il faut changer le niveau de la liste */
     while((list->level > 0)&&(list->header->forwards[list->level] == NIL))
       {
         list->level = list->level-1;
       }

     /* Ajuste le total de tickets */
     list->tickets -= x->tickets;
     
    }
  
  return;
}


/******************************
 * Recherche dans la skip_list
 ******************************/

PRIVATE void sched_search(struct skip_list* list, u32_t key, u32_t* index)
{
  struct skip_node* x;
  int i;
  
  /* Part du header pour trouver la clé */
  x=list->header;
  
  for(i=list->level; i>=0; i--)
    {
      /* Trouve la clé précédant celle a enlever */
      while (x->forwards[i]->key < key)
	{
	  x = x->forwards[i];
	}
    }
  
  /* le noeud recherche est le successeur de la derniere cle trouvee */
  x = x->forwards[0];
  
  /* Retourne son index */
  *index = x->index;
  
  return;
}


/****************
 * Ordonnanceur
 ****************/

PUBLIC void task_schedule()
{
  /* Decremente le quantum temps de la tache */
  proc_current->quantum--;

  /* Si tout le quantum est utilise, on ordonnance */
  if (proc_current->quantum == 0)
    {
      u32_t ticket;
      u32_t index;

      /* Rempli le quantum */
      proc_current->quantum = PROC_QUANTUM;

      /* Change l'etat */
      proc_current->state = PROC_READY;

      /* Tire un ticket au sort */
      ticket = random()%proc_ready.tickets;
      
      /* Cherche le gagnant */
      sched_search(&proc_ready,ticket,&index);

      /* Le gagnant devient la tache courante */
      proc_current = &proc_table[index];
      proc_current->state = PROC_RUNNING;

    }

  return;
}
