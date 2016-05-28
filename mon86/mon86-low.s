; MON86 - Low level routines


; Interrupt vectors

vect_trace	EQU $01
vect_break	EQU $03


; Offsets in globals structure
; Synchronize with mon86-main

master_sp   EQU $0
master_ss   EQU $2
slave_sp    EQU $4
slave_ss    EQU $6
slave_run   EQU $8
int_num     EQU $9


; Offsets in stack frame

regs_size   EQU $20
iret_size   EQU $6


	.TEXT

	IMPORT _main

; Program entry

_entry:
	push   ax
	push   cx
	call   _entry_1                    ; self-call to push IP

_entry_1:
	pop    ax                          ; compute offset displacement
	sub    ax,#_entry_1
	mov    cl,#4                       ; compute segment displacement
	shr    ax,cl
	mov    cx,cs                       ; realign segment and offset
	add    ax,cx
	push   ax
	mov    ax,#_entry_2
	push   ax
	retf                               ; self-return to pop CS:IP

_entry_2:
	mov    ax,ss                       ; BCC assumes DS=ES=SS
	mov    ds,ax                       ; and no data segment used here
	mov    es,ax                       ; so stay on the stack segment
	pop    cx
	pop    ax
	br     _main


; INT 01h - Trace
; Much code in 8086 to perform 80186 PUSH immediate

_int_trace:
	SUB    SP,#4                       ; room for 2 immediates
	PUSH   BP
	MOV    BP,SP
	PUSH   AX
	MOV    AX,#vect_trace
	MOV    [BP+2],AX                   ; BP+2 = i1
	MOV    AX,#0
	MOV    [BP+4],AX                   ; BP+4 = i2
	POP    AX
	POP    BP
	JMP    _int_hand


; INT 03h - Trace
; Much code in 8086 to perform 80186 PUSH immediate

_int_break:
	SUB    SP,#4                       ; room for 2 immediates
	PUSH   BP
	MOV    BP,SP
	PUSH   AX
	MOV    AX,#vect_break
	MOV    [BP+2],AX                   ; BP+2 = i1
	MOV    AX,#0
	MOV    [BP+4],AX                   ; BP+4 = i2
	POP    AX
	POP    BP
	JMP    _int_hand


_int_hand:
	STI
	SUB    SP,#4                       ; room for IP FL
	PUSH   DS
	SUB    SP,#4                       ; room for CS SS
	PUSH   ES
	PUSH   DI
	PUSH   SI
	PUSH   BP
	SUB    SP,#2                       ; room for SP
	PUSH   BX
	PUSH   DX
	PUSH   CX
	PUSH   AX

; Retrieve globals

	MOV    AX,#0                       ; vector table segment
	MOV    DS,AX
	LDS    BX,[$3FC]                   ; INT FFh reserved for globals *

; Switch to master if slave is running

	MOV    AL,[BX+slave_run]
	TEST   AL,#1
	JNZ    _master_stack

	JMP    _int_ret


; Switch to master stack
; DS:BX = globals *

_master_stack:
	CLI

	MOV    AL,#0
	MOV    [BX+slave_run],AL

	MOV    [BX+slave_sp],SP
	MOV    [BX+slave_ss],SS
	MOV    SP,[BX+master_sp]
	MOV    SS,[BX+master_ss]

	STI

	JMP    _int_ret


; Switch to slave stack
; DS:BX = globals *

_slave_stack:
	CLI

	MOV    [BX+master_sp],SP
	MOV    [BX+master_ss],SS
	MOV    SP,[BX+slave_sp]
	MOV    SS,[BX+slave_ss]

	MOV    AL,#1
	MOV    [BX+slave_run],AL

	STI

	JMP    _int_ret


; Return from interrupt
; Or switch from one task to another

_int_ret:
	POP    AX
	POP    CX
	POP    DX
	POP    BX
	ADD    SP,#2                       ; room for SP
	POP    BP
	POP    SI
	POP    DI
	POP    ES
	ADD    SP,#4                       ; room for CS SS
	POP    DS
	ADD    SP,#8                       ; room for IP FL and 2 immediates

	IRET


; Execute slave task
; Called by the master
; arg1 : globals *
; arg2 : regs *

_slave_exec:
	PUSH   BP
	MOV    BP,SP

	PUSH   BX
	MOV    BX,[BP+4]                   ; arg1 = globals *

; Update globals from regs

	PUSH   SI
	MOV    SI,[BP+6]                   ; arg2 = regs *

	PUSH   DI
	MOV    DI,[SI+$8]                  ; SP
	SUB    DI,#regs_size+iret_size     ; allocate room for slave frame
	MOV    [BX+slave_sp],DI

	PUSH   DX
	MOV    DX,[SI+$14]                 ; SS
	MOV    [BX+slave_ss],DX

; copy regs to slave frame

	PUSH   DI
	PUSH   ES
	MOV    ES,DX
	PUSH   CX
	MOV    CX,#regs_size
	REP
	MOVSB
	POP    CX
	POP    ES
	POP    DI

; Expand the slave frame
; Group IP CS FL for slave IRET

	PUSH   DS
	MOV    DS,DX
	MOV    AX,[DI+$18]                 ; IP
	MOV    [DI+$20],AX
	MOV    AX,[DI+$12]                 ; CS
	MOV    [DI+$22],AX
	MOV    AX,[DI+$1A]                 ; FL
	MOV    [DI+$24],AX
	POP    DS

	POP    DX
	POP    DI
	POP    SI

; Set the master back IRET

	PUSHF
	PUSH   CS
	MOV    AX,#_master_back
	PUSH   AX

; Save the master registers

	SUB    SP,#8                       ; room for 2 immediates and IP FL
	PUSH   DS
	SUB    SP,#4                       ; room for CS SS
	PUSH   ES
	PUSH   DI
	PUSH   SI
	PUSH   BP
	SUB    SP,#2                       ; room for SP
	PUSH   BX
	PUSH   DX
	PUSH   CX
	PUSH   AX

; Switch to slave

	BR	   _slave_stack

; Back from slave

_master_back:

; Update regs from globals

	PUSH   DI
	MOV    DI,[BP+6]                   ; arg2 = regs *

	PUSH   SI
	MOV    SI,[BX+slave_sp]            ; BX = globals *

	PUSH   DX
	MOV    DX,[BX+slave_ss]

; Compress the slave frame
; Ungroup IP CS FL from IRET

	PUSH   DS
	MOV    DS,DX
	MOV    AX,[SI+$22]                 ; CS
	MOV    [SI+$12],AX
	MOV    AX,[SI+$20]                 ; IP
	MOV    [SI+$18],AX
	MOV    AX,[SI+$24]                 ; FL
	MOV    [DI+$1A],AX

; copy slave frame to regs

	PUSH   SI
	PUSH   CX
	MOV    CX,#$20                     ; regs_t size
	REP
	MOVSB
	POP    CX
	POP    SI
	POP    DS

	ADD    SI,#regs_size+iret_size     ; release room of slave frame

	MOV    DI,[BP+6]                   ; arg2 = regs *
	MOV    [DI+$8],SI                  ; SP
	MOV    [DI+$14],DX                 ; SS

	POP    DX
	POP    SI

; Return immediate value 1

	MOV    AX,[DI+$1C]                 ; i1 offset
	POP    DI

; Back to caller

	POP    BX
	POP    BP
	RET


; Interrupt setup

_int_setup:
	PUSH   BP
	MOV    BP,SP
	PUSH   DX
	MOV    DX,[BP+4]                   ; arg1 = globals *

	PUSH   AX
	PUSH   DS
	MOV    AX,#0
	MOV    DS,AX

	PUSH   BX
	MOV    BX,#vect_trace*4            ; trace interrupt
	MOV    AX,#_int_trace;
	MOV    [BX],AX
	MOV    [BX+2],CS

	MOV    BX,#vect_break*4            ; break interrupt
	MOV    AX,#_int_break;
	MOV    [BX],AX
	MOV    [BX+2],CS

	MOV    BX,#$FF*4                   ; pointer to globals
	MOV    [BX],DX
	MOV    [BX+2],ES

	POP    BX
	POP    DS
	POP    AX
	POP    DX
	POP    BP
	RET


; Read from memory to context

_mem_read:
	push   bp
	mov    bp,sp
	push   ax
	push   bx
	push   si
	mov    bx,[bp+4]                   ; arg1 = context *
	mov    si,[bx+0]                   ; context.off
	push   ds
	mov    ds,[bx+2]                   ; context.seg
	lodsb
	pop    ds
	mov    ah,#0
	mov    [bx+6],ax                   ; context.val
	mov    [bx+0],si                   ; context.off++
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
	mov    di,[bx+0]                   ; context.off
   	mov    ax,[bx+6]                   ; context.val
	push   es
	mov    es,[bx+2]                   ; context.seg
	stosb
	pop    es
	mov    [bx+0],di                   ; context.off++
	pop    di
	pop    bx
	pop    ax
	pop    bp
	ret


; Call a far procedure

_sub_call:
	push   bp
	mov    bp,sp
	push   bx
	mov    bx,[bp+4]                   ; arg1 = context *
	callf  [bx]                        ; context.off:context.seg
	pop    bx
	pop    bp
	ret


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


; Exported labels

	EXPORT _int_setup
	EXPORT _slave_exec
	EXPORT _mem_read
	EXPORT _mem_write
	EXPORT _sub_call
	EXPORT _read_char
	EXPORT _write_char
	EXPORT _write_string

	ENTRY  _entry

	END
