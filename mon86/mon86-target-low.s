
int_trace	EQU 0x01
int_break	EQU 0x03


	.BSS

master_sp:  DW 0
master_ss:  DW 0

slave_run:  DW 0
slave_int:  DW 0
slave_sp:   DW 0
slave_ss:   DW 0


	.TEXT

	EXPORT _slave_exec

_int_generic_sub:
	RET

_int_trace_hand:
	PUSH   WORD int_trace
	JMP    _int_generic_hand

_int_break_hand:
	PUSH   WORD int_break
	JMP    _int_generic_hand

_int_generic_hand:
	PUSH   ES
	PUSH   DS
	PUSH   BP
	PUSH   DI
	PUSH   SI
	PUSH   DX
	PUSH   CX
	PUSH   BX
	PUSH   AX

	MOV    DS, 0x40
	MOV    AX,[slave_run]
	CMP    AX, WORD 1
	JNE    _slave_hand

	MOV    BP, SP
	MOV    AX, [BP + 0x12]
	CMP    AX, int_trace
	JE     _master_stack
	CMP    AX, int_break
	JE     _master_stack

_slave_hand:
	CALL   _int_generic_sub

_int_generic_ret:
	POP    AX
	POP    BX
	POP    CX
	POP    DX
	POP    SI
	POP    DI
	POP    BP
	POP    DS
	POP    ES
	ADD    SP, 2
	IRET

_master_stack:
	CLI
	MOV    [slave_sp], SP
	MOV    [slave_ss], SS
	MOV    SP, [master_sp]
	MOV    SS, [master_ss]
	STI

	MOV    ES, [slave_ss]
	MOV    BX, [slave_sp]
	SEG    ES
	MOV    AX, [BX + 0x12]
	MOV    [slave_int], AX

	JMP    _master_hand

_slave_stack:
	CLI
	MOV    [master_sp], SP
	MOV    [master_ss], SS
	MOV    SP, [slave_sp]
	MOV    SS, [slave_ss]
	STI

	JMP _int_generic_ret

_slave_exec:
	PUSH   ES
	PUSH   DS
	PUSH   BP
	PUSH   DI
	PUSH   SI
	PUSHA

	MOV    ES, [slave_ss]
	MOV    BX, [slave_sp]

	LEA    AX, _master_hand
	SEG    ES
	MOV    [BX + 0x1A], AX
	SEG    ES
	MOV    [BX + 0x1C], CS

	XOR    AX, AX
	LAHF
	SEG    ES
	MOV    [BX + 0x1E], AX

	MOV    AX, 1
	MOV    [slave_run], AX

	JMP	   _slave_stack

_master_hand:
	POPA
	POP    SI
	POP    DI
	POP    BP
	POP    DS
	POP    ES
	RET

	END
