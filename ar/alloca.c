
#ifdef __STDC__
#include <stdlib.h>
#else
#include <memory.h>
#include <string.h>
#endif

#if defined(__TINYC__) || defined(__HP_cc)
typedef union mem_cell
{
   union mem_cell *next;	/* A pointer to the next mem */
   unsigned int size;		/* An int >= sizeof pointer */
   char *depth;			/* For the alloca hack */
}
mem;

#define m_size(p)  ((p) [0].size)	/* For malloc */
#define m_next(p)  ((p) [1].next)	/* For malloc and alloca */
#define m_deep(p)  ((p) [0].depth)	/* For alloca */

static mem *alloca_stack = 0;

void *
alloca(size)
size_t size;
{
   auto char probe;		/* Probes stack depth: */
   register mem *hp;

   /*
    * Reclaim garbage, defined as all alloca'd storage that was allocated
    * from deeper in the stack than currently.
    */

   for (hp = alloca_stack; hp != 0;)
      if (m_deep(hp) < &probe)
      {
	 register mem *np = m_next(hp);
	 free((void *) hp);	/* Collect garbage.  */
	 hp = np;		/* -> next header.  */
      }
      else
	 break;			/* Rest are not deeper.  */

   alloca_stack = hp;		/* -> last valid storage.  */
   if (size == 0)
      return 0;			/* No allocation required.  */

   hp = (mem *) malloc(sizeof(mem)*2 + size);
   if (hp == 0)
      return hp;

   m_next(hp) = alloca_stack;
   m_deep(hp) = &probe;
   alloca_stack = hp;

   /* User storage begins just after header.  */
   return (void *) (hp + 2);
}
#endif
