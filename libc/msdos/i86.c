/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */
/*
 * These functions will not normally be useful for Linux-8086. But they
 * can be used and may be useful in the kernel.
 */

#ifdef __AS386_16__

#ifdef L___seg_regs
unsigned int
__get_cs()
{
#asm
   mov	ax,cs
#endasm
}

unsigned int
__get_ds()
{
#asm
   mov	ax,ds
#endasm
}

unsigned int
__get_es()
{
#asm
   mov	ax,es
#endasm
}

void
__set_es(seg)
{
#asm
#if __FIRST_ARG_IN_AX__
   mov	es,ax
#else
   mov	bx,sp
   mov	es,[bx+2]
#endif
#endasm
}
#endif

#ifdef L___peek_es
int
__peek_es(off)
unsigned int off;
{
#asm
#if __FIRST_ARG_IN_AX__
  mov	bx,ax
#else
  mov	bx,sp
  mov	bx,[bx+2]
#endif
  seg	es
  mov	al,[bx]
  xor	ah,ah
#endasm
}
#endif

#ifdef L___poke_es
int
__poke_es(off, value)
unsigned int off;
int value;
{
#asm
#if __FIRST_ARG_IN_AX__
  mov	bx,sp
  mov	bx,[bx+2]
  xchg	ax,bx
#else
  mov	bx,sp
  mov	ax,[bx+4]
  mov	bx,[bx+2]
#endif
  seg	es
  mov	[bx],al
  xor	ah,ah
#endasm
}
#endif

#ifdef L___deek_es
int
__deek_es(off)
unsigned int off;
{
#asm
#if __FIRST_ARG_IN_AX__
  mov	bx,ax
#else
  mov	bx,sp
  mov	bx,[bx+2]
#endif
  seg	es
  mov	ax,[bx]
#endasm
}
#endif

#ifdef L___doke_es
int
__doke_es(off, value)
unsigned int off;
int value;
{
#asm
#if __FIRST_ARG_IN_AX__
  mov	bx,sp
  mov	bx,[bx+2]
  xchg	ax,bx
#else
  mov	bx,sp
  mov	ax,[bx+4]
  mov	bx,[bx+2]
#endif
  seg	es
  mov	[bx],ax
#endasm
}
#endif

#ifdef L___strchr_es
char *
__strchr_es(s, c)
char * s;
int c;
{
#asm
  mov	bx,sp
  push	si
#if __FIRST_ARG_IN_AX__
  mov	bx,[bx+2]
  mov	si,ax
#else
  mov	si,[bx+2]
  mov	bx,[bx+4]
#endif
  xor	ax,ax

#ifdef PARANOID
  cld
#endif
  push	ds
  push	es
  pop	ds

in_loop:
  lodsb
  cmp	al,bl
  jz	got_it
  or	al,al
  jnz	in_loop
  pop	ds
  pop	si
  ret
got_it:
  lea	ax,[si-1]
  pop	ds
  pop	si

#endasm
}
#endif

#ifdef L___strnget_es
char *
__strnget_es(d, s, c)
char *d;
char *s;
register int c;
{
   register int i = __strlen_es(s);
   if(i < c) c = i+1;
   /* else s[--c] = 0;  ?? */
   /* else return -E2BIG; ?? */

   __movedata(__get_es(), s, __get_ds(), d, c);
}
#endif

#ifdef L___strlen_es
int __strlen_es(str)
char * str;
{
#asm
#if !__FIRST_ARG_IN_AX__
  mov	bx,sp
#endif
  push	di
  cld

#if __FIRST_ARG_IN_AX__
  mov	di,ax
#else
  mov	di,[bx+2]
#endif
  mov	cx,#-1
  xor	ax,ax
  repne
  scasb		! Scans [ES:DI]
  not	cx
  dec	cx
  mov	ax,cx

  pop	di
#endasm
}
#endif

#endif /* __AS386_16__ */
