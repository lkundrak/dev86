/*   simplified linux malloc.h
     Copyright (C) 1995  Joel N. Weber II

     This program is free software; you can redistribute it and/or
     modify it under the terms of the GNU General Public License
     as published by the Free Software Foundation; either version 2
     of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <malloc.h>

typedef struct __malloc_struct malloc_struct;

typedef union __malloc_union
{
  char *c;
  malloc_struct *m;
} malloc_union;


struct __malloc_struct
{
  unsigned char status;
#define ALLOC		0x53
#define FREE		0x55
#define END_OF_WORLD	0x57
  malloc_union next;
} *malloc_start;

extern int __STACKSIZE;

/* WARNING: this init will only work if there is a hard limit on the
            amount of RAM that can be allocated.
 */

#ifdef __AS386_16__
#asm
  loc	1		! Make sure the pointer is in the correct segment
auto_func:		! Label for bcc -M to work.
  .word	_malloc_init	! Pointer to the autorun function
  .word no_op		! Space filler cause segs are padded to 4 bytes.
  .text			! So the function after is also in the correct seg.
#endasm
#endif

malloc_init()
{
  extern unsigned int sbrk();

  unsigned int ptr, sz, count;

  malloc_start = (malloc_struct*) ((sbrk(16)+1)&~1);
  malloc_start->status = FREE;

  count=254;
  for(sz=16384; sz>64; )
  {
     ptr= sbrk(sz);
     if( ptr == (unsigned)-1 ) sz>>=1;
     else                      count+=sz;
  }
  if( __STACKSIZE > count || __STACKSIZE <= 0 ) __STACKSIZE = ((count>>1)&-2);
  ptr = sbrk(-__STACKSIZE);

  malloc_start->next.m = ((malloc_struct*)ptr) - 1;

  malloc_start->next.m->status = END_OF_WORLD;
}

char *malloc(size)
size_t size;
{
  register malloc_union tmp, tmp2;

  /* Make sure we don't lose the alignment */
  size = (size+sizeof(malloc_struct)-1)/sizeof(malloc_struct);

  tmp.m = malloc_start;
  while ( ( tmp.m->next.m - tmp.m - 2 ) < size
    ||    ( tmp.m->status == ALLOC ))
    tmp.m = tmp.m->next.m;

  if (tmp.m->status == FREE){
    tmp2.m = size + tmp.m + 1;
    tmp2.m->status = FREE;
    tmp2.m->next.c = tmp.m->next.c;
    tmp.m->status = ALLOC;
    tmp.m->next.c = tmp2.c;
  }
  else return 0;
  tmp.m++;
  return tmp.c;
}

__malloc_cleanup() /* finds consecutive free blocks and joins them */
{
  malloc_struct *tmp;

  tmp = malloc_start;
  while ((tmp->status != END_OF_WORLD)&&(tmp->next.m->status != END_OF_WORLD)){
    if ((tmp->status==FREE)&&(tmp->next.m->status==FREE))
      tmp->next.m = tmp->next.m->next.m;
    else tmp = tmp->next.m;
  }
}

free(what)
char *what;
{
  malloc_union tmp;

  tmp.c = what; tmp.m--;
  if( tmp.m->status == ALLOC )
  {
     tmp.m->status = FREE;
     __malloc_cleanup;
  }
}
