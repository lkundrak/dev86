// LIB86 - 80x86 library

#pragma once

#include "op-common.h"


// Operation identifiers

// TODO: continuous ID to enable LUT

#define OP_NULL  0xFFFF

// LUT BEGIN

#define OP_CALC2 0x0000  // base for 2 operands calculation
#define OP_ADD   0x0000  // same order as in opcode
#define OP_OR    0x0001
#define OP_ADC   0x0002
#define OP_SBB   0x0003
#define OP_AND   0x0004
#define OP_SUB   0x0005
#define OP_XOR   0x0006
#define OP_CMP   0x0007

// LUT END

#define OP_XCHG  0x0100

#define OP_PORT1 0x0200  // base for port 1 operations
#define OP_IN    0x0200  // same order as in opcode
#define OP_OUT   0x0201

#define OP_MOV   0x0301
#define OP_LEA   0x0302

#define OP_LOAD1 0x0303  // base for load operations
#define OP_LES   0x0303  // same order as in opcode
#define OP_LDS   0x0304

#define OP_CALC1  0x0400  // base for 1 operand calculation
#define OP_TEST   0x0400  // same order as in opcode with holes
#define OP_CALC11 0x0401
#define OP_NOT    0x0402
#define OP_NEG    0x0403
#define OP_MUL    0x0404
#define OP_IMUL   0x0405
#define OP_DIV    0x0406
#define OP_IDIV   0x0407

#define OP_STEP1  0x0500  // base for step operations
#define OP_INC    0x0500  // same order as in opcode
#define OP_DEC    0x0501

#define OP_FLOW1  0x0800  // base for flow 1 operations
#define OP_CALL   0x0800  // same order as in opcode
#define OP_JMP    0x0801

#define OP_STACK1 0x0700  // base for stack 1 operations
#define OP_PUSH   0x0700  // same order as in opcode
#define OP_POP    0x0701

#define OP_BIT1  0x0600  // base for bit operations
#define OP_ROL   0x0600  // same order as in opcode
#define OP_ROR   0x0601
#define OP_RCL   0x0602
#define OP_RCR   0x0603
#define OP_SHL   0x0604
#define OP_SHR   0x0605
#define OP_SAL   0x0606
#define OP_SAR   0x0607

#define OP_STACK3 0x0702  // base for stack 3 operations
#define OP_PUSHF  0x0702  // same order as in opcode
#define OP_POPF   0x0703

#define OP_STACK2 0x0710  // base for stack 2 operations
#define OP_PUSHA  0x0710  // same order as in opcode
#define OP_POPA   0x0711

#define OP_CALLF 0x0802
#define OP_JMPF  0x0803

#define OP_INT3  0x0900  // base for interrupt 1 operations
#define OP_INT   0x0901
#define OP_INTO  0x0902
#define OP_IRET  0x0903

#define OP_RET   0x0A00
#define OP_RETF  0x0A01

#define OP_JUMP1 0x0B00  // base for jump 1 operations
#define OP_JO    0x0B00  // same order as in opcode
#define OP_JNO   0x0B01
#define OP_JB    0x0B02
#define OP_JNB   0x0B03
#define OP_JZ    0x0B04
#define OP_JNZ   0x0B05
#define OP_JNA   0x0B06
#define OP_JA    0x0B07
#define OP_JS    0x0B08
#define OP_JNS   0x0B09
#define OP_JP    0x0B0A
#define OP_JNP   0x0B0B
#define OP_JL    0x0B0C
#define OP_JNL   0x0B0D
#define OP_JNG   0x0B0E
#define OP_JG    0x0B0F

#define OP_PREFIX1 0x0C10  // base for prefix 1 operations
#define OP_LOCK    0x0C10  // same order as in opcode with holes
#define OP_INT1    0x0C11  // F1h reserved
#define OP_REPNZ   0x0C12
#define OP_REPZ    0x0C13
#define OP_HLT     0x0C14
#define OP_CMC     0x0C15

#define OP_STRING   0x0C00  // same order as in opcode with holes
#define OP_STRING1  0x0C01
#define OP_MOVS     0x0C02
#define OP_CMPS     0x0C03
#define OP_STRING4  0x0C04
#define OP_STOS     0x0C05
#define OP_LODS     0x0C06
#define OP_SCAS     0x0C07

#define OP_ADJUST1 0x0D00  // base for adjust 1 operations
#define OP_DAA     0x0D00  // same order as in opcode
#define OP_DAS     0x0D01
#define OP_AAA     0x0D02
#define OP_AAS     0x0D03

#define OP_ADJUST2 0x0D08  // base for adjust 2 operations
#define OP_AAM     0x0D08  // same order as in opcode
#define OP_AAD     0x0D09

#define OP_CONVERT1 0x0E00  // base for convert operations
#define OP_CBW      0x0E00  // same order as in opcode
#define OP_CWD      0x0E01

#define OP_FLAGS2 0x0F00  // base for flags 2 operations
#define OP_CLC    0x0F00  // same order as in opcode
#define OP_STC    0x0F01
#define OP_CLI    0x0F02
#define OP_STI    0x0F03
#define OP_CLD    0x0F04
#define OP_STD    0x0F05

#define OP_FLAGS1 0x1000  // base for flag 1 operations
#define OP_SAHF   0x1000  // same order as in opcode
#define OP_LAHF   0x1001

#define OP_LOOP1  0x1200  // base for loop 1 operations
#define OP_LOOPNZ 0x1200  // same order as in opcode
#define OP_LOOPZ  0x1201
#define OP_LOOP   0x1202
#define OP_JCXZ   0x1203

#define OP_SEG    0x1300

#define OP_NOP    0x1400

#define OP_MISC1  0xFFE0  // base for misc 1 operations
#define OP_SALC   0xFFE0
#define OP_XLAT   0xFFE1

#define OP_WAIT   0xFFF0
#define OP_ESC    0xFFF3
