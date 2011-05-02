/*
 * Header de la pagination
 *
 */

#ifndef PAGING_H
#define PAGING_H

/***********
 * Includes
 ***********/

#include <types.h>


/*************
 * Constantes
 *************/

#define PAGING_DIRSHIFT     22
#define PAGING_TBLSHIFT     12
#define PAGING_TBLMASK      0x3FF


/*************
 * Structures
 *************/

/* Page directory entry */

struct pde
{
  u32_t present   :1  ;  /* Page presente en memoire  */
  u32_t rw        :1  ;  /* Page en lecture/ecriture  */
  u32_t user      :1  ;  /* Privilege User/Supervisor */
  u32_t pwt       :1  ;  /* Write through             */
  u32_t pcd       :1  ;  /* Cache disable             */
  u32_t accessed  :1  ;  /* Page accedee (ro ou rw)   */ 
  u32_t zero      :1  ;  /* Zero                      */
  u32_t pagesize  :1  ;  /* Taille des pages          */
  u32_t global    :1  ;  /* Page globale              */
  u32_t available :3  ;  /* Bits disponibles          */ 
  u32_t baseaddr  :20 ;  /* Adresse physique          */
}__attribute__ ((packed));


/* Page table entry */

struct pte
{
  u32_t present   :1  ;
  u32_t rw        :1  ;
  u32_t user      :1  ;
  u32_t pwt       :1  ;
  u32_t pcd       :1  ;
  u32_t accessed  :1  ;
  u32_t dirty     :1  ;  /* Page ecrite */
  u32_t zero      :1  ;
  u32_t global    :1  ;
  u32_t available :3  ;
  u32_t baseaddr  :20 ;
}__attribute__ ((packed));


/**********
 * Macros
 **********/

#define GET_DIR(addr)				\
  ( addr >> PAGING_DIRSHIFT );

#define GET_TBL(addr)				\
  ( (addr >> PAGING_TBLSHIFT)&PAGING_TBLMASK );


/**********
 * typedef
 **********/

typedef struct pde pagedir_t[1024];
typedef struct pte tabledir_t[1024];



#endif
