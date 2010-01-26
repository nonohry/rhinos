#ifndef I82C54_H
#define I82C54_H

#include "types.h"
#include "prot.h"

/* Constantes */

#define CLOCK_MAX_FREQ     1193182    /* Frequence maximal du PIT */
#define CLOCK_FREQ         100        /* Frequence a 100Hz */

/* I/O Ports */

#define CLOCK_COUNTER0     0x40
#define CLOCK_COUNTER1     0x41
#define CLOCK_COUNTER2     0x42
#define CLOCK_CWREG        0x43

/* Mot de Controle */

#define CLOCK_MODE2        0x34   /* 0x34=00110100b, Mode 2 pour le compteur 0 */

/* Counter Latch */

#define CLOCK_LATCH        0x00   /* 00000000b Counter Latch pour le compteur 0 */

/* chaine pour l ISR */

PUBLIC struct irq_chaine clock_chaine;

/* Prototypes */

PUBLIC void clock_init();
PUBLIC u16_t clock_read();

#endif
