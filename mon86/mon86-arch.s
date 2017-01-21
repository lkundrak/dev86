;------------------------------------------------------------------------------
; MON86 - Architecture specific routines for the Advantech SMNP-1000-B1 SBC
;------------------------------------------------------------------------------

; Register offsets

io_timer1_mode  EQU $FF5E
io_port0_data   EQU $FF74


; Interrupt vectors

vect_tick       EQU $1C


;------------------------------------------------------------------------------

; Tick interrupt
; Called by the BIOS @ 1 Hz
; Used to quiet the hardware watchdog

int_tick:
	push   ax
	push   dx
	mov    dx,#io_port0_data
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
	push   dx

	; disable timer @ 1000 Hz

	mov     dx,#io_timer1_mode
	in      ax,dx
	and     ax,#$7FFF                  ; disable timer
	or      ax,#$4000                  ; unlock enable bit
	out     dx,ax

	; insert tick handler @ 1 Hz

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

	pop    dx
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
