
#include "doselks.h"

int in_process = 0;
int intr_pending = 0;
unsigned int current_ds, current_stack;
struct process * proc;

/***************************************************************************
 * These two routines to make doselks and the user program into co-routines.
 *
 * The Server calls run_cpu which runs the client.
 * The client then does an int $80 which calls elks_int.
 * Elks_int then returns to the caller of run_cpu.
 *
 * The return value is the value of AX when the int $80 occured.
 * On return ES is set to the DS of the client program.
 *
 * The instruction times are for the 286
 *
 * If we were using a 486 then using mov's would be faster on the 286 they
 * are the same speed but push/pop is smaller.
 */

#asm
  .data
bx_temp:	! Tempoary storage for BX.
  .word	0
  .text		! The is where we store the current_ds in the _TEXT_ segment
cs_current_ds: 	! for the elks_int routine
  .word	0
#endasm

unsigned int run_cpu()
{
#asm
  mov	ax,ds								; 2
  seg	cs			! Store ds for return
  mov	[cs_current_ds],ax						; 5

  cli				! We`re messing!			; 3
  test	[_intr_pending],#$FFFF	! Last chance ...
  jz	skp
  sti
  xor	ax,ax
  ret
skp:
  push	bp								; 3
  push	si								; 3
  push	di								; 3
  mov	[_current_stack],sp						; 5
  mov	ax,#1								; 2
  mov	[_in_process],ax						; 5
  mov	sp,[_proc]							; 5
  pop	ax			! pppop					; 5
  pop	[bx_temp]							; 5
  pop	cx								; 5
  pop	dx								; 5
  pop	di								; 5
  pop	si								; 5
  pop	bp								; 5
  pop	es								; 5
  mov	bx,sp			! Switch to client stack		; 2
  mov	ss,[bx+8]		! SS					; 5
  mov	sp,[bx+10]		! SP					; 5
  push	[bx+6]			! flags					; 5
  push	[bx+4]			! CS					; 5
  push	[bx+2]			! PC					; 5
  push	[bx+0]			! DS					; 5
  mov	bx,[bx_temp]							; 5
  pop	ds								; 5
  iret									; 17
#endasm
}

unsigned int elks_int()
{
#asm
.text
  cli									; 3
  push	ds								; 3
  push	cs								; 3
  pop	ds								; 5
  mov	ds,[cs_current_ds]						; 5
  mov	[bx_temp],bx							; 3
  mov	bx,[_proc]							; 5
  add	bx,#16								; 2
  pop	[bx+0]			! DS					; 5
  pop	[bx+2]			! PC					; 5
  pop	[bx+4]			! CS					; 5
  pop	[bx+6]			! flags					; 5
  mov	[bx+8],ss		! SS					; 3
  mov	[bx+10],sp		! SP					; 3

  mov	sp,bx			! Ready for pppppush			; 2
  mov	bx,ds								; 2
  mov	ss,bx								; 2

  push	es			! wheeee				; 3
  push	bp								; 3
  push	si								; 3
  push	di								; 3
  push	dx								; 3
  push	cx								; 3
  push	[bx_temp]							; 5
  push	ax			! This is the return value, handy	; 3

  mov	sp,[_current_stack]						; 5
  mov	bx,#1								; 2
  mov	[_in_process],bx						; 5
  sti				! Finished F**** around with the stack	; 2

  mov	bx,[_proc]							; 5
  mov	es,[bx+16]		! Set ES to the client`s DS		; 5
  pop	di								; 5
  pop	si								; 5
  pop	bp								; 5
#endasm
}

