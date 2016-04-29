
#pragma once

#include "op-common.h"

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
#define REG_MAX 0xA

#define SEG_ES  0x0
#define SEG_CS  0x1
#define SEG_SS  0x2
#define SEG_DS  0x3
#define SEG_MAX 0x4


word_t reg16_get (byte_t reg);
void reg16_set (byte_t reg, word_t val);

word_t seg_get (byte_t seg);
void seg_set (byte_t seg, word_t val);

byte_t fetch_cs_ip ();

void push_ss_sp (word_t val);

void proc_reset ();
