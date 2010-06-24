#ifndef START_H
#define START_H


/**************
 * Includes
 **************/

#include <types.h>


/********************** 
 * Structure boot_info
 **********************/

PUBLIC struct boot_info
{
  u16_t mem_lower;      /* Memoire basse en Ko */
  u16_t mem_upper;      /* Memoire haute en 64Ko */
};


/*************
 * Prototypes
 *************/

PUBLIC struct boot_info bootinfo;

#endif