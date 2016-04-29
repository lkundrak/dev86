// DIS86 - 80x86 disassembler
// Operation handling

#pragma once

#include "op-class.h"


// Operation handlers

void op_move_load (op_desc_t * op_desc);
void op_swap (op_desc_t * op_desc);
void op_port (op_desc_t * op_desc);
void op_calc_1 (op_desc_t * op_desc);
void op_calc_2 (op_desc_t * op_desc);
void op_inc_dec (op_desc_t * op_desc);
void op_shift_rot (op_desc_t * op_desc);

void op_push  (op_desc_t * op_desc);
void op_pop   (op_desc_t * op_desc);
void op_pushf (op_desc_t * op_desc);
void op_popf  (op_desc_t * op_desc);

void op_jump_call (op_desc_t * op_desc);
void op_int (op_desc_t * op_desc);
void op_return (op_desc_t * op_desc);
void op_jump_cond (op_desc_t * op_desc);
void op_string (op_desc_t * op_desc);

