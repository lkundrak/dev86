
#include <setjmp.h>

#if __AS386_16__

int
setjmp(env)
jmp_buf env;
{
#asm
export __setjmp
__setjmp:

  pop	cx		! PC
#if __FIRST_ARG_IN_AX__
  mov	bx,ax
#else
  mov	bx,sp
  mov	bx,[bx]		! TOS is prt -> env
#endif
  mov	[bx+0],cx	! PC
  mov	[bx+2],sp	! This registers are all that may be constant.
  mov	[bx+4],bp
  mov	[bx+6],si	! Is saving these the "right thing" ?
  mov	[bx+8],di
  xor	ax,ax
  jmp	cx
#endasm
}

void
longjmp(env, rv)
jmp_buf env;
int rv;
{
#asm
export __longjmp
__longjmp:

  pop	cx	! pc
#if __FIRST_ARG_IN_AX__
  mov	bx,ax	! env->
#else
  pop	bx	! env->
#endif
  pop	ax	! rv
  mov	cx,[bx+0]	! PC
  mov	sp,[bx+2]
  mov	bp,[bx+4]
  mov	si,[bx+6]
  mov	di,[bx+8]
  jmp	cx
#endasm
}

#endif
