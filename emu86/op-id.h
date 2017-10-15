// LIB86 - 80x86 library

#pragma once

#include "op-common.h"


// Operation identifiers

// TODO: continuous ID to enable LUT

#define OP_NULL     0xFFFF

// LUT BEGIN

#define OP_JUMP     0x0000  // base for jump 1 operations
#define OP_JO       0x0000  // same order as in opcode
#define OP_JNO      0x0001
#define OP_JB       0x0002
#define OP_JNB      0x0003
#define OP_JZ       0x0004
#define OP_JNZ      0x0005
#define OP_JNA      0x0006
#define OP_JA       0x0007
#define OP_JS       0x0008
#define OP_JNS      0x0009
#define OP_JP       0x000A
#define OP_JNP      0x000B
#define OP_JL       0x000C
#define OP_JNL      0x000D
#define OP_JNG      0x000E
#define OP_JG       0x000F

#define OP_CALC1    0x0010  // base for 1 operand calculations
#define OP_TEST     0x0010  // same order as in opcode with holes
#define OP_CALC11   0x0011
#define OP_NOT      0x0012
#define OP_NEG      0x0013
#define OP_MUL      0x0014
#define OP_IMUL     0x0015
#define OP_DIV      0x0016
#define OP_IDIV     0x0017

#define OP_CALC2    0x0018  // base for 2 operands calculation
#define OP_ADD      0x0018  // same order as in opcode
#define OP_OR       0x0019
#define OP_ADC      0x001A
#define OP_SBB      0x001B
#define OP_AND      0x001C
#define OP_SUB      0x001D
#define OP_XOR      0x001E
#define OP_CMP      0x001F

#define OP_BITS     0x0020  // base for bit 1 operations
#define OP_ROL      0x0020  // same order as in opcode
#define OP_ROR      0x0021
#define OP_RCL      0x0022
#define OP_RCR      0x0023
#define OP_SHL      0x0024
#define OP_SHR      0x0025
#define OP_SAL      0x0026
#define OP_SAR      0x0027

#define OP_STRING0  0x0028  // base for string operations
#define OP_STRING1  0x0029  // same order as in opcode with holes
#define OP_MOVS     0x002A
#define OP_CMPS     0x002B
#define OP_STRING3  0x002C
#define OP_STOS     0x002D
#define OP_LODS     0x002E
#define OP_SCAS     0x002F

#define OP_FLAGS2   0x0030  // base for flags 2 operations
#define OP_CLC      0x0030  // same order as in opcode
#define OP_STC      0x0031
#define OP_CLI      0x0032
#define OP_STI      0x0033
#define OP_CLD      0x0034
#define OP_STD      0x0035
#define OP_FLAGS26  0x0036
#define OP_FLAGS27  0x0037

#define OP_PREFIX   0x0038  // base for prefix 1 operations
#define OP_LOCK     0x0038  // same order as in opcode with holes
#define OP_INT1     0x0039  // F1h reserved
#define OP_REPNZ    0x003A
#define OP_REPZ     0x003B
#define OP_HLT      0x003C
#define OP_CMC      0x003D
#define OP_PREFIX6  0x003E
#define OP_PREFIX7  0x003F

#define OP_ADJUST1  0x0040  // base for adjust 1 operations
#define OP_DAA      0x0040  // same order as in opcode
#define OP_DAS      0x0041
#define OP_AAA      0x0042
#define OP_AAS      0x0043

#define OP_INTS     0x0044  // base for interrupt 1 operations
#define OP_INT3     0x0044
#define OP_INT      0x0045
#define OP_INTO     0x0046
#define OP_IRET     0x0047

#define OP_LOOP1    0x0048  // base for loop 1 operations
#define OP_LOOPNZ   0x0048  // same order as in opcode
#define OP_LOOPZ    0x0049
#define OP_LOOP     0x004A
#define OP_JCXZ     0x004B

#define OP_ADJUST2  0x004C  // base for adjust 2 operations
#define OP_AAM      0x004C  // same order as in opcode
#define OP_AAD      0x004D

#define OP_CONVERT1 0x004E  // base for convert operations
#define OP_CBW      0x004E  // same order as in opcode
#define OP_CWD      0x004F

#define OP_STEP1    0x0050  // base for step operations
#define OP_INC      0x0050  // same order as in opcode
#define OP_DEC      0x0051

#define OP_FLOW1    0x0052  // base for flow 1 operations
#define OP_CALL     0x0052  // same order as in opcode
#define OP_JMP      0x0053

#define OP_STACK1   0x0054  // base for stack 1 operations
#define OP_PUSH     0x0054  // same order as in opcode
#define OP_POP      0x0055

#define OP_STACK3   0x0056  // base for stack 3 operations
#define OP_PUSHF    0x0056  // same order as in opcode
#define OP_POPF     0x0057

#define OP_STACK2   0x0058  // base for stack 2 operations
#define OP_PUSHA    0x0058  // same order as in opcode
#define OP_POPA     0x0059

#define OP_CALLF    0x005A  // base for flow 2 operations
#define OP_JMPF     0x005B

#define OP_RET      0x005C
#define OP_RETF     0x005D

#define OP_PORT1    0x005E  // base for port 1 operations
#define OP_IN       0x005E  // same order as in opcode
#define OP_OUT      0x005F

#define OP_LOAD1    0x0060  // base for load operations
#define OP_LES      0x0060  // same order as in opcode
#define OP_LDS      0x0061

#define OP_FLAGS1   0x0062  // base for flag 1 operations
#define OP_SAHF     0x0062  // same order as in opcode
#define OP_LAHF     0x0063

#define OP_MISC1    0x0064  // base for misc 1 operations
#define OP_SALC     0x0064
#define OP_XLAT     0x0065

#define OP_MOV      0x0066
#define OP_LEA      0x0067
#define OP_NOP      0x0068
#define OP_XCHG     0x0069

#define OP_SEG      0x006A

#define OP_WAIT     0x006B
#define OP_ESC      0x006C

// LUT END
