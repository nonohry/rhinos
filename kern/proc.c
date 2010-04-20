/*
 * Gestion des processus
 *
 */

#include "types.h"
#include "klib.h"
#include "prot.h"
#include "proc.h"

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
     list->header->forwards[i] = NIL;
   }

 return;
}

/******************************
 * Insertion dans la skip_list
 ******************************/

PUBLIC void sched_insert(struct skip_list* list, struct skip_node* node)
{
  struct skip_node* update[SKIP_MAX_LEVEL];
  struct skip_node* x;
  int i;
  u32_t lvl;
  
  /* Calcule la key */
  node->key = node->tickets + list->tickets;

  /* Part du header */
  x=list->header;

  for(i=list->level; i>=0; i--)
    {
      /* Trouve la clé précédent le nouvel element */
      while (x->forwards[i]->key < node->key)
	{
	  x = x->forwards[i];
	}
      /* Sauve le noeud a mettre à jour au niveau i */
      update[i] = x;
    }

  /* Choisit un niveau */
  lvl = random()%SKIP_MAX_LEVEL;
  
  /* Si le niveau est superieur au niveau courant */
  if (lvl > list->level)
    {
      /* Le header des niveaux en surplus est a mettre à jour */
      for(i=list->level+1; i<=lvl; i++)
	{
	  update[i]=list->header;
	}

      /* Du coup on a un nouveau niveau */
      list->level = lvl;
    }

  /* On met à jour ses prédécesseurs et ses succeseeurs */
  for(i=0; i<=lvl; i++)
    {
      node->forwards[i] = update[i]->forwards[i];
      update[i]->forwards[i] = node;
    }

  /* Ajuste le total de ticket */
  list->tickets += node->tickets;

  return;
}

/********************************
 * Suppression dans la Skip_list
 ********************************/

PUBLIC void sched_delete(struct skip_list* list, u32_t key)
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

/****************************
 * Print Skip_List (DEBUG)
 ****************************/

PUBLIC void sched_print(struct skip_list* list)
{
  int i;

      if (list->tickets == 10) bochs_print("10\n");
      if (list->tickets == 20) bochs_print("20\n");
      if (list->tickets == 30) bochs_print("30\n");
      if (list->tickets == 40) bochs_print("40\n");
      if (list->tickets == 50) bochs_print("50\n");
      if (list->tickets == 60) bochs_print("60\n");
      if (list->tickets == 70) bochs_print("70\n");
      if (list->tickets == 80) bochs_print("80\n");
      if (list->tickets == 90) bochs_print("90\n");
      if (list->tickets == 100) bochs_print("100\n");
      if (list->tickets == 110) bochs_print("110\n");
      if (list->tickets == 120) bochs_print("120\n");
      if (list->tickets == 130) bochs_print("130\n");
      if (list->tickets == 140) bochs_print("140\n");
      if (list->tickets == 150) bochs_print("150\n");
      if (list->tickets == 160) bochs_print("160\n");
      if (list->tickets == 170) bochs_print("170\n");
      if (list->tickets == 180) bochs_print("180\n");
      if (list->tickets == 190) bochs_print("190\n");

  for(i=SKIP_MAX_LEVEL-1; i>=0; i--)
    {
      struct skip_node* p;

      p = list->header;
      bochs_print("HDR -> ");
   
      while (p->forwards[i] != NIL)
	{
	  p = p->forwards[i];
	  if (p->key == 10) bochs_print("10");
	  if (p->key == 20) bochs_print("20");
	  if (p->key == 30) bochs_print("30");
	  if (p->key == 40) bochs_print("40");
	  if (p->key == 50) bochs_print("50");
	  if (p->key == 60) bochs_print("60");
	  if (p->key == 70) bochs_print("70");
	  if (p->key == 80) bochs_print("80");
	  if (p->key == 90) bochs_print("90");
	  if (p->key == 100) bochs_print("100");
	  if (p->key == 110) bochs_print("110");
	  if (p->key == 120) bochs_print("120");
	  if (p->key == 130) bochs_print("130");
	  if (p->key == 140) bochs_print("140");
	  if (p->key == 150) bochs_print("150");
	  if (p->key == 160) bochs_print("160");
	  if (p->key == 170) bochs_print("170");
	  if (p->key == 180) bochs_print("180");
	  if (p->key == 190) bochs_print("190");
	  bochs_print(" -> ");
       }
      
      bochs_print("NIL\n");
    }

  return;
}


/***************************
 * Creation d un processus 
 ***************************/

PUBLIC void task_init(struct proc* pr, u32_t base, u32_t data_code, u32_t stack, u8_t priv, u32_t entry_point)
{

  /* Cree les segments */
  init_code_seg(&(pr->ldt[LDT_CS_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_DS_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_ES_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_FS_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_GS_INDEX]), base, data_code, priv);
  init_data_seg(&(pr->ldt[LDT_SS_INDEX]), base+data_code, stack, priv);

  /* Affecte les registres de segments */
  pr->context.cs = LDT_CS_SELECTOR | priv;
  pr->context.ds = LDT_DS_SELECTOR | priv;
  pr->context.es = LDT_ES_SELECTOR | priv;
  pr->context.fs = LDT_FS_SELECTOR | priv;
  pr->context.gs = LDT_GS_SELECTOR | priv;
  pr->context.ss = LDT_SS_SELECTOR | priv;

  /* Copie le code au bon endroit */
  phys_copy(entry_point, base, data_code);
  
  /* Positionne les registres nécessaires */
  pr->context.esp = stack;   /* La pile */ 
  pr->context.eip = 0;       /* Pointeur d instructions */
  pr->context.eflags = 512;  /* Interrupt Enable Flag */

  /* Le selecteur de la LDT */
  init_ldt_seg(&gdt[LDT_INDEX],(u32_t) &(pr->ldt[0]), sizeof(pr->ldt), 0);
  pr->ldt_selector = LDT_INDEX << SHIFT_SELECTOR;

  return;
}

