/*
 *
 * Macros Water Mark Allocator
 *
 */

#ifndef WMALLOC_H
#define WMALLOC_H

/* Initialisation */
#define WMALLOC_INIT(wm,wmbase,wmsize)		\
  {						\
    (wm).base = wmbase;				\
    (wm).size = wmsize;				\
    (wm).offset = 0;				\
  }

/* Allocation */
#define WMALLOC_ALLOC(wm,wmsize)			\
  (void*)( (wm).offset < (wm).size ? ( ((wm).offset+=wmsize) ? ( (wm).offset <= (wm).size ? ((wm).base+(wm).offset-wmsize) : 0 ) : 0 ) : 0 ); 

/* Liberation */
#define WMALLOC_FREE(wm,addr)			

#endif
