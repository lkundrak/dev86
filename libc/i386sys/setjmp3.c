
#include <setjmp.h>

#if __AS386_32__

int
setjmp(env)
jmp_buf env;
{
#asm
export __setjmp
__setjmp:

  pop	ecx		! PC
#if __FIRST_ARG_IN_AX__
  mov	ebx,eax
#else
  mov	ebx,esp
  mov	ebx,[ebx]	! TOS is prt -> env
#endif
  mov	[ebx+0],ecx	! PC
  mov	[ebx+4],esp	! This registers are all that may be constant.
  mov	[ebx+8],ebp
  mov	[ebx+12],esi	! Is saving these the "right thing" ?
  mov	[ebx+16],edi
  xor	eax,eax
  jmp	ecx
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

  pop	ecx	! pc
#if __FIRST_ARG_IN_AX__
  mov	ebx,eax	! env->
#else
  pop	ebx	! env->
#endif
  pop	eax	! rv
  mov	ecx,[ebx+0]	! PC
  mov	esp,[ebx+4]
  mov	ebp,[ebx+8]
  mov	esi,[ebx+12]
  mov	edi,[ebx+16]
  jmp	ecx
#endasm
}

#endif
