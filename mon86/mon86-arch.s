;------------------------------------------------------------------------------
; MON86 - Architecture specific routines for the Advantech SMNP-1000-B1 SBC
;------------------------------------------------------------------------------


; Interrupt vectors

vect_tick  EQU $1C


;------------------------------------------------------------------------------

; Tick interrupt
; Called every second by the BIOS
; Used to quiet the hardware watchdog

int_tick:
	push   ax
	push   dx
	mov    dx,#0xFF74                  ; I/O port 0
	in     ax,dx
	xor    ax,#1
	out    dx,ax
	pop    dx
	pop    ax
	iret


;------------------------------------------------------------------------------

; Architecture setup

_arch_setup:
	PUSH   BP
	MOV    BP,SP
	PUSH   AX
	PUSH   BX

	PUSH   DS
	MOV    AX,#0
	MOV    DS,AX

	MOV    BX,#vect_tick*4             ; tick interrupt
	MOV    AX,#int_tick;
	CLI
	MOV    [BX],AX
	MOV    [BX+2],CS
	STI

	POP    DS

	POP    BX
	POP    AX
	POP    BP
	RET


;------------------------------------------------------------------------------

; Read one character from serial
; Using Advantech tweaked BIOS

_recv_char:
	push   bp
	mov    bp,sp
	push   bx
	mov    bx,[bp+4]                   ; arg1 = char * c

read_loop:
	mov    ah,#$10                     ; get extended key
	int    $16                         ; BIOS keyboard service
	or     ah,ah
	jz     read_exit                   ; got a key

	hlt                                ; idle until next interrupt
	jmp    read_loop

read_exit:
	mov    [bx],al
	pop    bx
	xor    ax,ax
	pop    bp
	ret


; Write one character to serial
; Using Advantech tweaked BIOS

_send_char:
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

_send_string:
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

; Exported labels

	EXPORT _arch_setup

	EXPORT _recv_char
	EXPORT _send_char

	EXPORT _send_string

	END


;------------------------------------------------------------------------------
