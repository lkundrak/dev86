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

#ifdef  L_int86
int
int86(intr, in_regs, out_regs)
int intr;
union REGS* in_regs;
union REGS* out_regs;
{
#asm
  push	bp
  mov	bp,sp
  push	ds			! save ds
  				! es too ?
  push	bp			! same for new bp

  pushf				! iret flags
  mov	ax,[bp-6]		! flags for simulated int
  push	cs			! iret address segment
  mov	bx,#ret_addr		! iret address offset
  push	bx
  and	ah,#$0C			! simulate interrupt flags
  push	ax			! flags are pushed first

  xor	bx,bx
  mov	es,bx			! interrupt vectors in seg 0
  mov	bl,[bp+4]
  shl	bx,#1
  shl	bx,#1			! intr*4 => interrupt vector address
  seg	es
  push	[bx+2]			! fetch interrupt segment
  seg	es
  push	[bx]			! fetch interrupt offset
  mov	bx,[bp+6]		! input union REGS*

  mov	ax,[bx]
  mov	cx,[bx+4]
  mov	dx,[bx+6]
  mov	si,[bx+8]
  mov	di,[bx+10]
  mov	bx,[bx+2]
  ! Ignore cflag/flags ?

  iret				! simulate interrupt.
  				! But won't be nice for protected mode ...
ret_addr:
  ! Int $25/6 would need resetting sp:ss too ... should I ?

  pop	bp			! unzapped versions
  pop	ds			! paranoia

  pushf				! save interrupt flags
  push	bx			! save pointer register
  mov	bx,[bp+8]		! output union REGS*

  mov	[bx],ax
  pop	[bx+2]
  mov	[bx+4],cx
  mov	[bx+6],dx
  mov	[bx+8],si
  mov	[bx+10],di
  mov	word [bx+12],#0		! cflag
  jnc	no_carry
  mov	byte [bx+12],#1
no_carry:
  pop	[bx+14]			! flags

  pop	bp
#endasm
}

#endif

#ifdef  L_int86x
int
int86x(intr, in_regs, out_regs, segr)
int intr;
union REGS* in_regs;
union REGS* out_regs;
struct SREGS * segr;
{
#asm
  push	bp
  mov	bp,sp
  push	ds			! save ds
  				! es too ?
  push	bp			! same for new bp

  pushf				! iret flags
  mov	ax,[bp-6]		! flags for simulated int
  push	cs			! iret address segment
  mov	bx,#ret_addr		! iret address offset
  push	bx
  and	ah,#$0C			! simulate interrupt flags
  push	ax			! flags are pushed first

  xor	bx,bx
  mov	es,bx			! interrupt vectors in seg 0
  mov	bl,[bp+4]
  shl	bx,1
  shl	bx,1			! intr*4 => interrupt vector address
  seg	es
  push	word [bx+2]		! fetch interrupt segment
  seg	es
  push	word [bx]		! fetch interrupt offset

  mov	bx,[bp+10]		! struct SREGS*
  mov	es,[bx]
  push	[bx+6]			! ds
 
  mov	bx,[bp+6]		! input union REGS*

  mov	ax,[bx]
  mov	cx,[bx+4]
  mov	dx,[bx+6]
  mov	si,[bx+8]
  mov	di,[bx+10]
  mov	bx,[bx+2]
  ! Ignore cflag/flags ?

  pop	ds

  iret				! simulate interrupt
  				! But won't be nice for protected mode ...

ret_addr:
  ! Int $25/6 would need resetting sp:ss too ... should I ?

  pop	bp			! in case it was zapped

  pushf				! save interrupt flags

  push	cx			! save work register
  mov	cx,ds
  push	bx			! save pointer register

  mov	ds,word [bp-2]		! restore original ds
  mov	bx,[bp+10]		! struct SREGS*
  mov	[bx],es
  mov	[bx+6],cx
 
  mov	bx,[bp+8]		! output union REGS*
  mov	[bx],ax
  pop	[bx+2]			! bx
  pop	[bx+4]			! cx
  mov	[bx+6],dx
  mov	[bx+8],si
  mov	[bx+10],di
  mov	word [bx+12],#0		! cflag
  jnc	no_carry
  mov	byte [bx+12],#1
no_carry:
  pop	[bx+14]			! flags

  pop	ds
  pop	bp
#endasm
}
#endif

#ifdef L_segread
segread(segp)
struct SREGS * segp;
{
#asm
#if __FIRST_ARG_IN_AX__
  mov	bx,ax
#else
  mov	bx,sp
  mov	bx,[bx+2]
#endif
  mov	[bx],es
  mov	[bx+2],cs
  mov	[bx+4],ss
  mov	[bx+6],ds
#endasm
}
#endif

#endif /* __AS386_16__ */
