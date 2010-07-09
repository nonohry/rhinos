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
PRIVATE void task_index(u32_t* index);

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


/********************
 * Choix d un index
 ********************/

PRIVATE void task_index(u32_t* index)
{
  u32_t i;

  i=0;
  while( (proc_table[i].state != PROC_TERMINATED) && (i<MAX_INDEX) )
    {
      i++;
    }

  if (i >= MAX_INDEX)
    {
      bochs_print("process table full !\n");
    }

  *index = i;

  return;
}


/***************************
 * Creation d une tache 
 ***************************/

PUBLIC void task_init(struct proc* pr, 
		      u32_t index, 
		      u32_t code_base, 
		      u32_t code_size, 
		      u32_t data_base, 
		      u32_t data_size, 
		      u32_t stack_base, 
		      u32_t stack_size, 
		      u8_t priv, 
		      u32_t code_entry_point,
		      u32_t data_entry_point,
		      u32_t tickets)
{

  /* Cree les segments */
  init_code_seg(&(pr->ldt[LDT_CS_INDEX]), code_base, code_size, priv);
  init_data_seg(&(pr->ldt[LDT_DS_INDEX]), data_base, data_size, priv);
  init_data_seg(&(pr->ldt[LDT_ES_INDEX]), data_base, data_size, priv);
  init_data_seg(&(pr->ldt[LDT_FS_INDEX]), data_base, data_size, priv);
  init_data_seg(&(pr->ldt[LDT_GS_INDEX]), data_base, data_size, priv);
  init_data_seg(&(pr->ldt[LDT_SS_INDEX]), stack_base, stack_size, priv);

  /* Affecte les registres de segments */
  pr->context.cs = LDT_CS_SELECTOR | priv;
  pr->context.ds = LDT_DS_SELECTOR | priv;
  pr->context.es = LDT_ES_SELECTOR | priv;
  pr->context.fs = LDT_FS_SELECTOR | priv;
  pr->context.gs = LDT_GS_SELECTOR | priv;
  pr->context.ss = LDT_SS_SELECTOR | priv;

  /* Copie le code au bon endroit */
  phys_copy(code_entry_point, code_base, code_size);
  phys_copy(data_entry_point, data_base, data_size);

  /* Positionne les registres nécessaires */
  pr->context.esp = stack_size;  /* La pile */ 
  pr->context.eip = 0;           /* Pointeur d instructions */
  pr->context.eflags = PROC_IF;  /* Interrupt Enable Flag */

  /* Affecte son quantum */
  pr->quantum = PROC_QUANTUM;

  /* Indique l'etat */
  pr->state = PROC_READY;

  /* Affecte les tickets  */
  pr->node.tickets = tickets;

  /* Le selecteur de la LDT */
  init_ldt_seg(&gdt[LDT_INDEX+index],(u32_t) &(pr->ldt[0]), sizeof(pr->ldt), 0);
  pr->ldt_selector = (LDT_INDEX+index) << SHIFT_SELECTOR;

  /* Index de la tache */
  pr->node.index = index;

  /* Insert la tache dans la skip list */
  sched_insert(&proc_ready,&(pr->node));

  return;
}


/******************************
 * Chargement d un format ELF 
 ******************************/

PUBLIC void task_elf(u32_t* addr,u8_t flags)
{
  struct elf_header* header;
  struct prog_header* pheader;  /* Program Header */
  struct prog_header* cheader;  /* Program Header - Code */
  struct prog_header* dheader;  /* Program Header - Data */
  u16_t i;

  /* Recupere le header */
  header = (struct elf_header*)addr;

  /* Verifie le format */
  if (header->e_ident[EI_MAG0] != ELFMAG0 ||
      header->e_ident[EI_MAG1] != ELFMAG1 ||
      header->e_ident[EI_MAG2] != ELFMAG2 ||
      header->e_ident[EI_MAG3] != ELFMAG3 )
    {
      bochs_print("Not an ELF Format !\n");
      return;
    }

  /* Verifie la classe */
  if (header->e_ident[EI_CLASS] != ELFCLASS32)
    {
      bochs_print("Wrong ELF Class\n");
      return;
    }

  /* Verifie l encodage */
  if (header->e_ident[EI_DATA] != ELFDATA2LSB)
    {
      bochs_print("Wrong ELF Data Encoding\n");
      return;
    }

  /* Verifie le type */
  if (header->e_type != ET_EXEC)
    {
      bochs_print("Wrong ELF Type\n");
      return;
    }

  /* Verifie la version */
  if (header->e_version != EV_CURRENT)
    {
      bochs_print("Wrong ELF Version\n");
      return;
    }  

  /* Verifie qu il y a des choses a charger */
   if (header->e_phnum == 0)
    {
      bochs_print("Nothing to load (e_phnum==0) !\n");
      return;
    }  

  /* Parcours des Program Headers */
  for(i=0;i<header->e_phnum;i++)
    {
      /* Recupere l adresse  des structures */
      pheader = (struct prog_header*)((u32_t)addr+header->e_phoff+i*header->e_phentsize);

      /* Ne charge que le type PT_LOAD */
      if(pheader->p_type == PT_LOAD)
	{
	  /* Segment de code */
	  if ((pheader->p_flags & PF_R) && (pheader->p_flags & PF_X))
	    {
	      /* Sauvegarde le segment de code */
	      cheader = pheader;
	    }

	  /* Segment de donnees */
	    if ((pheader->p_flags & PF_R) && (pheader->p_flags & PF_W))
	    {
	      /* Sauvegarde le segment de donnees */
	      dheader = pheader;
	    }
	  
	}
    }
 
  /* On va agir différemment suivant l argument flags */
  switch(flags)
    {
    case PROC_MM_FLAG:
      {
	task_init(&proc_table[PROC_MM_INDEX],
		  PROC_MM_INDEX,
		  PROC_MM_LOAD,
		  cheader->p_filesz,
		  PROC_MM_LOAD+cheader->p_filesz,
		  dheader->p_filesz,
		  PROC_MM_LOAD+cheader->p_filesz+dheader->p_filesz,
		  PROC_STACK,
		  PROC_MM_PRIV,
		  (u32_t)addr+cheader->p_offset,
		  (u32_t)addr+dheader->p_offset,
		  PROC_MM_TICKETS);
      }
      break;

    default:
      break;
    }

  return;
}
