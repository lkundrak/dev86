/* Copyright (C) 1989,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the terms of the GNU Library General Public License.
 **********************************************************************
 *
 * This does a determination of the cpu type that is actually being used.
 * It can determine the CPU on anything upto and including a 386 accuratly
 * whatever mode the CPU is in (This is 16 bit code)
 *
 * For Post 386 interpretation the argument must be set to 1, if this is done
 * an attempt to determine the CPU type will be made using MSDOS calls and
 * potentially Illegal instructions.
 *
 * If STANDALONE is defined this will decode and print the output from cputype
 *
 * $ cputype	       # Call cputype(0) and interpret
 * $ cputype +	       # Call cputype(1) get a SIGILL (or perhaps interpret)
 *
 */

#ifdef STANDALONE
#define cputype cpu

#include <stdio.h>
#ifndef __MSDOS__
#include <signal.h>
#endif

char * name_808x[] = {
"8088", "8086", "80C88", "80C86", "NEC V20", "NEC V30", "808x Clone"
};

char * name_8018x[] = {
"80188", "80186", "8018x Clone"
};

void
main(argc, argv)
int argc; char **argv;
{
   int c, major, flg, fpu;
#ifdef SIGFPE
   signal(SIGFPE, SIG_IGN);
#endif

   printf("Cpu identifier - (C) R de Bath <rdebath@cix.compulink.co.uk>\n");

   c = cputype(argc!=1);
   fpu = (c<0); major = ((c>>8)&0x1F); c &= 0xFF;

   if( major == 0 )
   {
      if( c > 6 ) c = 6;
      printf("Cpu is an %s\n", name_808x[c]);
   }
   else if( major == 1 )
   {
      if( c > 3 ) c = 3;
      printf("Cpu is an %s\n", name_8018x[c]);
   }
   else
   {
      printf("Cpu is an 80%x86%s", major&0xF, major>15?"+":"");
      if(c&0x01) printf( " in protected mode");
      printf(" MSW= ");
      if( c&0x10 ) printf("ET,"); else printf("--,");
      if( c&0x08 ) printf("TS,"); else printf("--,");
      if( c&0x04 ) printf("EM,"); else printf("--,");
      if( c&0x02 ) printf("MP,"); else printf("--,");
      if( c&0x01 ) printf("PE\n"); else printf("--\n");

      if( !fpu && ( c&0x06) )
	 printf("An FPU appears to exist but it is unavailable\n");
      else
      {
	 if( c&0x02 ) printf("Math processor requires WAIT\n");
	 if( c&0x04 ) printf("Emulated math present\n");
	 if( c&0x08 ) printf("Math processor belongs to a different process\n");
	 /* if( c&0x10 ) printf("Humm\n"); */
      }
   }
   if( fpu ) printf("FPU available for use\n");

   exit(0);
}
#endif

/*
 *  The assembler for CPU determination.
 *
 *  Improvements and additional CPUs are solicited.
 */

#ifdef __AS386_16__
#asm
	.text
#ifdef STANDALONE
export _cpu
_cpu:
#else
export _cputype
_cputype:
#endif
	; First save everything ...
	push bp
	mov  bp,sp
	push ds
	push es
	push bx
	push cx
	push dx
	pushf
#if __FIRST_ARG_IN_AX__
	mov cx, ax		; Collect the flag
#else
	mov cx, [bp+4]		; Collect the flag
#endif

	; Tiny mode code ...
	mov ax, cs
	mov es, ax
	mov ds, ax
	mov bx, #0	 ; Init to 8086

	; First easy check is it a 286 or better ...
	push sp
	pop ax
	cmp ax, sp
	jz ge286
	br pre286

	; Come here when we`re done  (286+)
cpu_prot:
	; .286P
	smsw ax		; Fetch 5 LSB of MSW (PE,MP,EP,...)
	and  al,#31
	mov  bl,al

	; Check for FPU
	fninit
	xor ax,ax
	push ax
	mov bp,sp
	fnstcw word ptr [bp]
	pop ax
	cmp ah,#3
	jne cpuend
	or bh,#$80

	; Another check for FPU *BUT* I think this only get`s 287+
;	finit
;	fstsw ax
;	or al,al
;	jnz cpuend
;	or bh,#$80

	; .8086
cpuend:
	mov ax, bx
	popf
	pop dx
	pop cx
	pop bx
	pop es
	pop ds
	pop bp
	ret

ge286:	; .286P
	; Does the caller want the exact CPU
	cmp cx,#0
	jne try_486
	
; Simple test for a 286 ...

	mov bh,#2	 ; Major CPU type >= 80286
	; What`s the contents of the GDT pointer
	sub sp,#6
	mov bp,sp
	sgdt [bp]
	add sp,#4
	pop ax		; For 286, ah can only be 0xFF
	inc ah
	jz cpu_prot
	mov bh,#$13	; Major CPU type >= 80386

#ifdef __MSDOS__
	smsw ax		; If we`re in MSDOS and running in real mode
	ror ax,#1	; we can do the int 6 detection.
	jnc try_486
#endif

	jmp cpu_prot	; Assume 486 test will NOT work in prot mode

	; This is an alternate way of finding a 386 ...
	; But it *can* be hidden by V86 mode.
;	pushf
;	mov ax,#$7000
;	push ax
;	popf
;	pushf
;	pop ax
;	popf
;	and ax,#$7000
;	jz is_a_286

try_486:
	; This trys to trap undefined instructions
	; it may not work if the CPU is in protected mode
	; Note: This code works for anything 286+
	cli
	push bp
	mov bp, sp
	mov ax,#$3506
	int #$21		; WARNING - DOS DOS DOS DOS DOS !!!!!
	mov [vector+2], es
	mov [vector], bx
	mov ax,#$2506
	lea dx, [int6]
	int #$21
	mov bh,#2		; 286

	; .486
test386:
	mov ebx,#$00040300	; 386 or 486
test486:
	bswap ebx		; Byte twiddle now 486

	mov ax,#1
do_cpuid:
	db $0F			; CPUID instruction
	db $A2

	mov ax,#1		; And again cause of Nasty EMM386s
	db $0F			; CPUID instruction
	db $A2

	and ah,#15		; Select family number
	mov bh,ah		; put it where we want it

	; .286P
fail386:
	mov ax, #$2506
	mov dx, [vector]
	mov ds, [vector+2]
	int #$21
	pop bp
	sti
	br cpu_prot


	; Tests for processors before the 80286 ...
	; .8086
pre286:
	; Is it an 8018x ? These mask shifts to less that 32 bits
	mov cl,#32
	mov ax, #$0100
	shl ax,cl
	mov bx, ax
	jnz test8

	; Try for an NEC V20/30
	mov ax, #$0208
	db $D5
	db  16		; Only the 8088 actually checks the arg to AAD
	cmp al, #$28	; as intel ran out of microcode space
	jz cmos
	mov bx,#4	; NEC V20
	jmp test8

	; The CMOS 8088/6 had the bug with rep lods repaired.
cmos:	push si
	sti
	mov cx, #$FFFF
	rep
	 lodsb
	pop si
	or cx,cx
	jne test8
	mov bx,#2	 ; Intel 80C88

	; This tests the prefetch of the CPU, 8 bit ones have 4 bytes
	; 16 bit cpus have a queue of 6 bytes.
test8:	push di
	push bx
	mov dx,#0
	mov bx,#4
	std
	mov al,#$90

retest: lea di,[_nop]
	cli
	mov cx,#3
	rep
	 stosb
	nop
	nop
	nop
	nop
_inc:	inc dx
	nop
_nop:	nop
	sti
	mov byte ptr [_inc], #$42
	dec bx
	jnz retest
	pop bx
	cmp dx,#0
	jz done8
	inc bx
done8:	pop di
	cld

	br cpuend


	; Function called by the illegal instruction trap
int6:
	mov sp, bp
	jmp fail386

; This was the old way, didn`t always work tho.
;	push bp
;	mov bp, sp
;	push ax
;	mov ax,cs
;	cmp 4[bp],ax
;	pop ax
;	jnz pass
;	cmp bh,#2
;	je move23
;	cmp bh,#3
;	je move34
;	add [bp+2], #(fail386 - do_cpuid)
;	jmp return
;move34:	add [bp+2], #(fail386 - test486)
;	jmp return
;move23:	add [bp+2], #(fail386 - test386)
;return:	pop bp
;	iret
;
;pass:	pop bp
;	jmp [vector]

vector: dd 0

#endasm

#endif
