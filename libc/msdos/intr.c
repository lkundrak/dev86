
struct REGPACK
{
   unsigned r_ax, r_bx, r_cx, r_dx;
   unsigned r_bp, r_si, r_di, r_ds, r_es, r_flags;
};

/* DANGER DANGER -- Self modifying code! */

#asm
.text
save_sp:
  dw	0
#endasm

intr(intr, regs)
int intr;
struct REGPACK * regs;
{
#asm
  mov	bx,sp
  push	bp
  push	si
  push	di
  push	es
  push	ds

  mov	ax,[bx+2]
  seg	cs
  mov	[intr_inst+1],al
  seg	cs
  mov	[save_sp],sp

  mov	bx,[bx+4]

  mov	ah,[bx+18]	! Flags low byte
  sahf 

  mov	ax,[bx]
  push	[bx+2]
  mov	cx,[bx+4]
  mov	dx,[bx+6]
  mov	bp,[bx+8]
  mov	si,[bx+10]
  mov	di,[bx+12]
  mov	es,[bx+16]
  mov	ds,[bx+14]
  pop	bx

intr_inst:
  int	$FF		! Must be a real int .. consider protected mode.

  seg	cs		! Could be SS as DS==SS
  mov	sp,[save_sp]
  seg	cs
  mov	[save_sp],ds
  pop	ds
  push	[save_sp]

  push	bx
  mov	bx,sp
  mov	bx,[bx+12]

  mov	[bx],ax
  pop	[bx+2]
  mov	[bx+4],cx
  mov	[bx+6],dx 
  mov	[bx+8],bp
  mov	[bx+10],si
  mov	[bx+12],di
  pop	[bx+14]
  mov	[bx+16],es
  pushf
  pop	[bx+18]

  pop	es
  pop	di
  pop	si
  pop	bp
  
#endasm
}
