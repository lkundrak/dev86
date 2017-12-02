
#pragma once

#include "op-common.h"


// Register numbers

#define REG8_MAX 8

#define REG_AL  0x0
#define REG_CL  0x1
#define REG_DL  0x2
#define REG_BL  0x3
#define REG_AH  0x4
#define REG_CH  0x5
#define REG_DH  0x6
#define REG_BH  0x7


#define REG16_MAX 10

#define REG_AX  0x0
#define REG_CX  0x1
#define REG_DX  0x2
#define REG_BX  0x3

#define REG_SP  0x4
#define REG_BP  0x5
#define REG_SI  0x6
#define REG_DI  0x7

#define REG_IP  0x8
#define REG_FL  0x9


#define SEG_MAX 4

#define SEG_ES  0x0
#define SEG_CS  0x1
#define SEG_SS  0x2
#define SEG_DS  0x3


#define FLAG_MAX 12

#define FLAG_CF 0
#define FLAG_PF 2
#define FLAG_AF 4
#define FLAG_ZF 6
#define FLAG_SF 7
#define FLAG_TF 8
#define FLAG_IF 9
#define FLAG_DF 10
#define FLAG_OF 11


// Register operations

byte_t reg8_get (byte_t reg);
void reg8_set (byte_t reg, byte_t val);

word_t reg16_get (byte_t reg);
void reg16_set (byte_t reg, word_t val);

word_t seg_get (byte_t seg);
void seg_set (byte_t seg, word_t val);

byte_t flag_get (byte_t flag);
void flag_set (byte_t flag, byte_t val);

void regs_print ();


// Memory operations

addr_t addr_seg_off (word_t seg, word_t off);

void mem_print (word_t seg, word_t begin, word_t end);


// Stack operations

void stack_push (word_t val);
word_t stack_pop ();

void stack_print ();


// Processor operations

void proc_reset ();
