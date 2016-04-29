// DIS86 - 80x86 disassembler
// Operation handles


#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "op-class.h"
#include "op-exec.h"
#include "op-id-name.h"
#include "emu-proc.h"


// Register access

static word_t reg_get (byte_t type, byte_t num)
	{
	word_t val = 0;

	switch (type)
		{
		case RT_REG8:
			//val = reg8_get (num);
			assert (0);
			break;

		case RT_REG16:
			val = reg16_get (num);
			break;

		case RT_SEG:
			val = seg_get (num);
			break;
		}
	}

static void reg_set (byte_t type, byte_t num, word_t val)
	{
	switch (type)
		{
		case RT_REG8:
			//reg8_set (num, val);
			assert (0);
			break;

		case RT_REG16:
			reg16_set (num, val);
			break;

		case RT_SEG:
			seg_set (num, val);
			break;
		}
	}


// Variable access

static word_t addr_get (byte_t flags, short rel)
	{
	word_t offset = 0;

	if (flags & AF_BX) offset += reg16_get (REG_BX);
	if (flags & AF_BP) offset += reg16_get (REG_BP);
	if (flags & AF_SI) offset += reg16_get (REG_SI);
	if (flags & AF_DI) offset += reg16_get (REG_DI);

	if (flags & AF_DISP) offset += rel;

	return offset;
	}


static word_t val_get (op_var_t * var)
	{
	word_t val = 0;

	byte_t rt;

	switch (var->type)
		{
		case VT_IMM:
			val = var->val.abs;
			break;

		case VT_REG:
			rt = (var->size == VS_BYTE) ? RT_REG8 : RT_REG16;
			val = reg_get (rt, var->val.reg);
			break;

		case VT_SEG:
			val = reg_get (RT_SEG, var->val.reg);
			break;

		case VT_INDEX:
			val = addr_get (var->flags, var->val.rel);
			break;

		default:
			assert (0);

		}
	}


static void val_set (op_var_t * var, word_t val)
	{
	assert (0);
	}


// Move & load

void op_move_load (op_desc_t * op_desc)
	{
	byte_t op = (op_desc->op_id & 0xFF);
	assert (op < 0x2);
	assert (0);
	}


// Swap

void op_swap (op_desc_t * op_desc)
	{
	byte_t op = (op_desc->op_id & 0xFF);
	assert (op == 0x0);
	}


// Input & output

void op_port (op_desc_t * op_desc)
	{
	byte_t op = (op_desc->op_id & 0xFF);
	assert (op < 0x2);
	}


// Arithmetic & logic

void op_calc_1 (op_desc_t * op_desc)
	{
	byte_t op = (op_desc->op_id & 0xFF);
	assert (op < 0x06);
	assert (0);
	}

void op_calc_2 (op_desc_t * op_desc)
	{
	byte_t op = (op_desc->op_id & 0xFF);
	assert (op < 0x09);
	assert (0);
	}


// Increment & decrement

void op_inc_dec (op_desc_t * op_desc)
	{
	byte_t op = (op_desc->op_id & 0xFF);
	assert (op < 0x02);
	assert (0);
	}


// Shift & rotate

void op_shift_rot (op_desc_t * op_desc)
	{
	byte_t op = (op_desc->op_id & 0xFF);
	assert (op < 0x8);
	assert (0);
	}


// Push & pop

void op_push (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);
	op_var_t * v = &(op_desc->var_to);
	assert (v->size == VS_WORD);
	word_t val = val_get (v);
	push_ss_sp (val);
	}

void op_pop (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);
	op_var_t * v = &(op_desc->var_to);
	assert (v->size == VS_WORD);
	word_t val = pop_ss_sp ();
	val_set (&(op_desc->var_to), val);
	}

void op_pushf (op_desc_t * op_desc)
	{
	assert (!op_desc->var_count);
	push_ss_sp (reg16_get (REG_FL));
	}

void op_popf (op_desc_t * op_desc)
	{
	assert (!op_desc->var_count);
	reg16_set (REG_FL, pop_ss_sp ());
	}


// Jump & call

void op_jump_call (op_desc_t * op_desc)
	{
	word_t h = (op_desc->op_id & 0xFF00);
	byte_t l = (op_desc->op_id & 0x00FF);
	assert (h == OP_JMP || h == OP_CALL);

	word_t ip = reg16_get (REG_IP);

	if (h == OP_CALL)
		{
		if (l & OF_FAR) push_ss_sp (seg_get (SEG_CS));
		push_ss_sp (ip);
		}

	assert (op_desc->var_count == 1);
	op_var_t * v = &(op_desc->var_to);

	if (l & OF_FAR)
		{
		assert (v->type == VT_FAR);
		reg16_set (REG_IP, v->val.abs);
		seg_set (SEG_CS, v->seg);
		}
	else
		{
		assert (v->type == VT_NEAR);
		reg16_set (REG_IP, ip + v->val.rel);
		}
	}


// Interrupt

void op_int (op_desc_t * op_desc)
	{
	byte_t op = (op_desc->op_id & 0xFF);
	assert (op < 0x3);
	assert (0);
	}


// Return

void op_return (op_desc_t * op_desc)
	{
	byte_t op = (op_desc->op_id & 0xFF);
	assert (op < 0x3);
	assert (0);
	}


// Jump conditional

void op_jump_cond (op_desc_t * op_desc)
	{
	byte_t op = (op_desc->op_id & 0xFF);
	assert (op < 0x10);
	assert (0);
	}


// String

void op_string (op_desc_t * op_desc)
	{
	byte_t op = (op_desc->op_id & 0xFF);
	assert (op < 0xA);
	assert (0);
	}
