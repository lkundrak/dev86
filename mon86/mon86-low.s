;------------------------------------------------------------------------------
; MON86 - Low level routines
;------------------------------------------------------------------------------


; Interrupt vectors

vect_trace  EQU $01
vect_break  EQU $03
vect_exit   EQU $FF


; Offsets in globals structure
; Synchronize with mon86-main

glob_magic  EQU $0
master_sp   EQU $2
master_ss   EQU $4
slave_sp    EQU $6
slave_ss    EQU $8
slave_run   EQU $A
slave_ret   EQU $B


; Offsets in stack frame

iret_size  EQU $6


; Offsets in registers

reg_max    EQU $E
regs_size  EQU 2*reg_max

reg_ax     EQU 2*$0
reg_cx     EQU 2*$1
reg_dx     EQU 2*$2
reg_bx     EQU 2*$3
reg_sp     EQU 2*$4
reg_bp     EQU 2*$5
reg_si     EQU 2*$6
reg_di     EQU 2*$7
reg_es     EQU 2*$8
reg_cs     EQU 2*$9
reg_ss     EQU 2*$A
reg_ds     EQU 2*$B
reg_ip     EQU 2*$C
reg_fl     EQU 2*$D


; Offsets in context stucture
; Synchronize with mon86-common

context_off  EQU $0
context_seg  EQU $2
context_len  EQU $4
context_val  EQU $6


;------------------------------------------------------------------------------

	.TEXT

	IMPORT _main

; Program entry

_entry:
	push   ax
	push   cx
	call   entry_1                     ; self-call to push IP

entry_1:
	pop    ax                          ; compute offset displacement
	sub    ax,#entry_1
	mov    cl,#4                       ; compute segment displacement
	shr    ax,cl
	mov    cx,cs                       ; realign segment and offset
	add    ax,cx
	push   ax
	mov    ax,#entry_2
	push   ax
	retf                               ; self-return to pop CS:IP

entry_2:
	mov    ax,ss                       ; BCC assumes DS=ES=SS
	mov    ds,ax                       ; and no data segment used here
	mov    es,ax                       ; so stay on the stack segment
	pop    cx
	pop    ax
	br     _main


;------------------------------------------------------------------------------

; Advantech specific
; Toggle bit 0 of port
; Probably a watchdog
; Directly sourced from original ROM

_toggle_0:
	push   bp
	mov    bp,sp
	push   dx
	push   ax
	mov    dx,#0xFF74
	in     ax,dx
	mov    dx,ax
	not    ax
	and    ax,#1
	and    dx,#0xFFFE
	or     ax,dx
	mov    dx,#0xFF74
	out    dx,ax
	pop    ax
	pop    dx
	pop    bp
	ret


; Read one character from serial
; Using Advantech tweaked BIOS

_read_char:
	push   bp
	mov    bp,sp
	push   bx
	mov    bx,[bp+4]                   ; arg1 = char * c

_read_loop:
	call   _toggle_0                   ; quiet the watching thing
	mov    ah,#$10                     ; get extended key
	int    $16                         ; BIOS keyboard service
	or     ah,ah
	jz     _read_exit                  ; got a key

	hlt                                ; idle until next interrupt
	jmp    _read_loop

_read_exit:
	mov    [bx],al
	pop    bx
	xor    ax,ax
	pop    bp
	ret


; Write one character to serial
; Using Advantech tweaked BIOS

_write_char:
	push   bp
	mov    bp,sp
	mov    ax,[bp+4]                   ; BCC pushes char_t as word
	mov    ah,#$0A                     ; write character
	int    $10                         ; BIOS video service
	xor    ax,ax
	pop    bp
	ret


; Write string to serial
; Using Advantech tweaked BIOS

_write_string:
	push   bp
	mov    bp,sp
	push   cx
	mov    cx,[bp+6]                   ; arg2 = word_t : string length
	push   bp
	mov    bp,[bp+4]                   ; arg1 = char * : string pointer (ES already set)
	mov    ah,#$13                     ; write string
	int    $10                         ; BIOS video service
	pop    bp
	pop    cx
	xor    ax,ax
	pop    bp
	ret


;------------------------------------------------------------------------------

; Read from memory to context

_mem_read:
	push   bp
	mov    bp,sp
	push   ax
	push   bx
	push   si
	mov    bx,[bp+4]                   ; arg1 = context *
	mov    si,[bx+context_off]
	push   ds
	mov    ds,[bx+context_seg]
	lodsb
	pop    ds
	mov    ah,#0
	mov    [bx+context_val],ax
	mov    [bx+context_off],si
	pop    si
	pop    bx
	pop    ax
	pop    bp
	ret


; Write from context to memory

_mem_write:
	push   bp
	mov    bp,sp
	push   ax
	push   bx
	push   di
	mov    bx,[bp+4]                   ; arg1 = context *
	mov    di,[bx+context_off]
	mov    ax,[bx+context_val]
	push   es
	mov    es,[bx+context_seg]
	stosb
	pop    es
	mov    [bx+0],di                   ; context.off++
	pop    di
	pop    bx
	pop    ax
	pop    bp
	ret


;------------------------------------------------------------------------------

; Setup registers
; Safe values

_reg_setup:
	push   bp
	mov    bp,sp
	push   ax
	push   bx

	mov    bx,[bp+4]                   ; arg1 = regs *

	xor    ax,ax
	mov    [bx+reg_ax],ax
	mov    [bx+reg_cx],ax
	mov    [bx+reg_dx],ax
	mov    [bx+reg_bx],ax
	mov    [bx+reg_sp],ax
	mov    [bx+reg_bp],ax
	mov    [bx+reg_si],ax
	mov    [bx+reg_di],ax

	mov    ax,es
	mov    [bx+reg_es],ax
	mov    ax,cs
	mov    [bx+reg_cs],ax
	mov    ax,ss
	mov    [bx+reg_ss],ax
	mov    ax,ds
	mov    [bx+reg_ds],ax

	mov    ax,#task_stub
	mov    [bx+reg_ip],ax

	pushf
	pop    ax
	mov    [bx+reg_fl],ax

	pop    bx
	pop    ax
	pop    bp
	ret


;------------------------------------------------------------------------------

; Call far procedure

_proc_call:
	push   bp
	mov    bp,sp
	push   ax
	push   bx
	push   cx
	push   dx
	push   si
	push   di

	mov    bx,[bp+4]                   ; arg1 = context *

	push   bp

; Push return far address

	push   cs
	mov    ax,#proc_ret
	push   ax

; Push call far address

	mov    ax,[bx+context_seg]
	push   ax
	mov    ax,[bx+context_off]
	push   ax

; Load registers through the stack
; Excluding FL - Including BP

	mov    si,[bp+6]                   ; arg2 = regs *
	mov    cx,#reg_max

proc_push:
	lodsw
	push   ax
	loop   proc_push

	add    sp,#4                       ; skip FL and IP
	pop    ds
	add    sp,#4                       ; skip CS and SS
	pop    es
	pop    di
	pop    si
	pop    bp
	add    sp,#2                       ; skip SP
	pop    bx
	pop    dx
	pop    cx
	pop    ax

; Jump to procedure

	retf

proc_ret:

	pop    bp

; Save registers through the stack
; Including FL - Excluding BP

	pushf
	sub    sp,#2                       ; skip IP
	push   ds
	sub    sp,#4                       ; skip CS and SS
	push   es
	push   di
	push   si
	sub    sp,#4                       ; skip SP and BP
	push   bx
	push   dx
	push   cx
	push   ax

	mov    ax,ss                       ; BCC assumes DS=ES=SS
	mov    ds,ax
	mov    es,ax

	mov    di,[bp+6]                   ; arg2 = regs *
	mov    cx,#reg_max

proc_pop:
	pop    ax
	stosw
	loop   proc_pop

	pop    di
	pop    si
	pop    dx
	pop    cx
	pop    bx
	pop    ax
	pop    bp
	ret


;------------------------------------------------------------------------------

; INT 01h - Trace
; Much code in 8086 to perform 80186 PUSH immediate

int_trace:
	STI
	SUB    SP,#2                       ; space for int_num

	PUSH   BP
	MOV    BP,SP
	PUSH   AX

	MOV    AX,#vect_trace
	MOV    [BP+2],AX                   ; BP+2 = int_num

	POP    AX
	POP    BP

	JMP    int_hand


; INT 03h - Trace
; Much code in 8086 to perform 80186 PUSH immediate

int_break:
	STI
	SUB    SP,#2                       ; space for int_num

	PUSH   BP
	MOV    BP,SP
	PUSH   AX

	MOV    AX,#vect_break
	MOV    [BP+2],AX                   ; BP+2 = int_num

	POP    AX
	POP    BP

	JMP    int_hand


; IRET - Exit (fake INT FFh)
; Shift slave task to insert IRET to stub

int_exit:
	STI
	SUB    SP,#2+iret_size             ; space for int_num and IRET to stub

	PUSH   BP
	MOV    BP,SP
	PUSH   AX

	MOV    AX,#vect_exit
	MOV    [BP+2],AX                   ; BP+2 = int_num

	MOV    AX,#task_stub
	MOV    [BP+4],AX
	MOV    [BP+6],CS
	PUSHF
	POP    AX
	MOV    [BP+8],AX

	POP    AX
	POP    BP

	JMP    int_hand


int_hand:
	SUB    SP,#4                       ; space for IP FL
	PUSH   DS
	SUB    SP,#4                       ; space for CS SS
	PUSH   ES
	PUSH   DI
	PUSH   SI
	PUSH   BP
	SUB    SP,#2                       ; space for SP
	PUSH   BX
	PUSH   DX
	PUSH   CX
	PUSH   AX

; Retrieve globals * to DS:BX
; From fixed & safe location

	MOV    AX,#0                       ; vector table segment
	MOV    DS,AX
	LDS    BX,[$3FC]                   ; INT FFh reserved for globals *

; Switch to master if slave is running

	MOV    AL,[BX+slave_run]
	TEST   AL,#1
	JNZ    master_stack

	JMP    int_ret


; Switch to master stack
; Assume DS:BX = globals *

master_stack:
	MOV    AL,#0
	MOV    [BX+slave_run],AL

	MOV    [BX+slave_sp],SP
	MOV    [BX+slave_ss],SS

	CLI
	MOV    SP,[BX+master_sp]
	MOV    SS,[BX+master_ss]
	STI

	JMP    int_ret


; Switch to slave stack
; Assume DS:BX = globals *

slave_stack:
	MOV    [BX+master_sp],SP
	MOV    [BX+master_ss],SS

	CLI
	MOV    SP,[BX+slave_sp]
	MOV    SS,[BX+slave_ss]
	STI

	MOV    AL,#1
	MOV    [BX+slave_run],AL

	JMP    int_ret


; Return from interrupt
; Or switch from one task to another

int_ret:
	POP    AX
	POP    CX
	POP    DX
	POP    BX
	ADD    SP,#2                       ; space for SP
	POP    BP
	POP    SI
	POP    DI
	POP    ES
	ADD    SP,#4                       ; space for CS SS
	POP    DS
	ADD    SP,#6                       ; space for IP FL and int_num

	IRET


;------------------------------------------------------------------------------

; Task stub

task_stub:
	IRET

; Execute slave task
; arg1 : globals *
; arg2 : regs *
; arg3 : push IRET to master flag

_task_exec:
	PUSH   BP
	MOV    BP,SP
	PUSH   BX
	PUSH   CX
	PUSH   DX
	PUSH   SI
	PUSH   DI

; Update globals from regs

	MOV    BX,[BP+4]                   ; arg1 = globals *
	MOV    SI,[BP+6]                   ; arg2 = regs *

	MOV    DI,[SI+$8]                  ; SP
	SUB    DI,#regs_size+2+iret_size   ; allocate space for slave frame
	MOV    DX,[SI+$14]                 ; SS

; Option for IRET to master

	mov    ax,[bp+8]                   ; arg3 = flag
	test   ax,ax                       ; more space for IRET to master
	jz     task_exec_1
	sub    di,#iret_size

	PUSH   DS
	MOV    DS,DX

	mov    ax,#int_exit
	mov    [di+regs_size+2+iret_size+0], ax
	mov    [di+regs_size+2+iret_size+2], cs
	pushf
	pop    ax
	mov    [di+regs_size+2+iret_size+4], ax

	POP    DS

task_exec_1:

; copy regs to slave frame

	PUSH   DI
	PUSH   ES
	MOV    ES,DX

	MOV    CX,#reg_max
	REP
	MOVSW

	POP    ES
	POP    DI

; Expand the slave frame
; Group IP CS FL for slave IRET

	PUSH   DS
	MOV    DS,DX

	XOR    AX,AX
	MOV    [DI+regs_size+0],AX

	MOV    AX,[DI+reg_ip]
	MOV    [DI+regs_size+2+0],AX

	MOV    AX,[DI+reg_cs]
	MOV    [DI+regs_size+2+2],AX

	MOV    AX,[DI+reg_fl]
	MOV    [DI+regs_size+2+4],AX

	POP    DS

	MOV    [BX+slave_sp],DI
	MOV    [BX+slave_ss],DX

; Push the master back IRET

	PUSHF
	PUSH   CS
	MOV    AX,#master_back
	PUSH   AX

; Push the master frame

	SUB    SP,#6                       ; space for immediate value and IP FL
	PUSH   DS
	SUB    SP,#4                       ; space for CS SS
	PUSH   ES
	PUSH   DI
	PUSH   SI
	PUSH   BP
	SUB    SP,#2                       ; space for SP
	PUSH   BX
	PUSH   DX
	PUSH   CX
	PUSH   AX

; Switch to slave

	BR	   slave_stack


; Back from slave
; Assume DS:BX = globals *

master_back:

; Update regs from globals

	MOV    DI,[BP+6]                   ; arg2 = regs *

	MOV    SI,[BX+slave_sp]            ; BX = globals *
	MOV    DX,[BX+slave_ss]

; Compress the slave frame
; Ungroup IP CS FL from slave IRET

	PUSH   DS
	MOV    DS,DX

	MOV    AX,[SI+regs_size+2+4]
	MOV    [SI+reg_fl],AX

	MOV    AX,[SI+regs_size+2+2]
	MOV    [SI+reg_cs],AX

	MOV    AX,[SI+regs_size+2+0]
	MOV    [SI+reg_ip],AX

	MOV    AX,[SI+regs_size+0]         ; int_num

; copy slave frame to regs

	PUSH   SI

	MOV    CX,#reg_max
	REP
	MOVSW

	POP    SI
	POP    DS

	ADD    SI,#regs_size+2+iret_size   ; release space of slave frame

	MOV    DI,[BP+6]                   ; arg2 = regs *
	MOV    [DI+$8],SI                  ; SP
	MOV    [DI+$14],DX                 ; SS

; Back to caller

	POP    DI
	POP    SI
	POP    DX
	POP    CX
	POP    BX
	POP    BP
	RET


;------------------------------------------------------------------------------

; Interrupt setup

_int_setup:
	PUSH   BP
	MOV    BP,SP
	PUSH   AX
	PUSH   BX
	PUSH   DX

	MOV    DX,[BP+4]                   ; arg1 = globals *

	PUSH   DS
	MOV    AX,#0
	MOV    DS,AX

	MOV    BX,#vect_trace*4            ; trace interrupt
	MOV    AX,#int_trace;
	CLI
	MOV    [BX],AX
	MOV    [BX+2],CS
	STI

	MOV    BX,#vect_break*4            ; break interrupt
	MOV    AX,#int_break;
	CLI
	MOV    [BX],AX
	MOV    [BX+2],CS
	STI

	MOV    BX,#vect_exit*4             ; pointer to globals
	CLI
	MOV    [BX],DX
	MOV    [BX+2],ES
	STI

	POP    DS

	POP    DX
	POP    BX
	POP    AX
	POP    BP
	RET


;------------------------------------------------------------------------------

; Exported labels

	EXPORT _read_char
	EXPORT _write_char
	EXPORT _write_string

	EXPORT _mem_read
	EXPORT _mem_write

	EXPORT _reg_setup

	EXPORT _proc_call
	EXPORT _task_exec

	EXPORT _int_setup

	ENTRY  _entry

	END

;------------------------------------------------------------------------------
