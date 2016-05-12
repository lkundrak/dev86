// DIS86 - 80x86 disassembler
// Operation handles

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "op-id.h"
#include "op-id-name.h"

#include "op-class.h"
#include "op-exec.h"

#include "emu-proc.h"
#include "emu-mem-io.h"
#include "emu-int.h"


// Offset & address

// TODO: case of absolute offset ?

static word_t off_get (op_var_t * var)
	{
	assert (var->type == VT_INDEX);

	byte_t f = var->flags;
	short s = 0;

	if (f & AF_BX) s += reg16_get (REG_BX);
	if (f & AF_BP) s += reg16_get (REG_BP);
	if (f & AF_SI) s += reg16_get (REG_SI);
	if (f & AF_DI) s += reg16_get (REG_DI);

	if (f & AF_DISP) s += var->val.w;

	return (word_t) s;
	}


// Variable access

static void val_get (op_var_t * var, op_val_t * val)
	{
	word_t o = 0;

	switch (var->type)
		{
		// TODO: merge near distance and immediate

		case VT_NEAR:
		case VT_IMM:
			val->w = var->val.w;
			break;

		case VT_REG:
			switch (var->size)
				{
				case VS_BYTE:
					val->b = reg8_get (var->val.r);
					break;

				case VS_WORD:
					val->w = reg16_get (var->val.r);
					break;

				default:
					assert (0);

				}

			break;

		case VT_SEG:
			val->w = seg_get (var->val.r);
			break;

		case VT_INDEX:
			o = off_get (var);

			// TODO: add segment base

			switch (var->size)
				{
				case VS_BYTE:
					val->b = mem_read_byte (o);
					break;

				case VS_WORD:
					val->w = mem_read_word (o);
					break;

				default:
					assert (0);

				}

			break;

		default:
			assert (0);

		}
	}


static void val_set (op_var_t * var, op_val_t * val)
	{
	word_t o = 0;

	switch (var->type)
		{
		case VT_REG:
			switch (var->size)
				{
				case VS_BYTE:
					reg8_set (var->val.r, val->b);
					break;

				case VS_WORD:
					reg16_set (var->val.r, val->w);
					break;

				default:
					assert (0);

				}

			break;

		case VT_SEG:
			seg_set (var->val.r, val->w);
			break;

		case VT_INDEX:
			o = off_get (var);

			// TODO: add segment base

			switch (var->size)
				{
				case VS_BYTE:
					mem_write_byte (o, val->b);
					break;

				case VS_WORD:
					mem_write_word (o, val->w);
					break;

				default:
					assert (0);

				}

			break;

		default:
			assert (0);

		}
	}


// Move & load

static void op_move_load (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 2);

	op_val_t val;
	memset (&val, 0, sizeof (op_val_t));

	switch (op_desc->op_id)
		{
		case OP_MOV:
			val_get (&op_desc->var_from, &val);
			val_set (&op_desc->var_to, &val);
			break;

		case OP_LEA:
			val.w = off_get (&op_desc->var_from);
			val_set (&op_desc->var_to, &val);
			break;

		default:
			assert (0);

		}
	}


// Swap

static void op_swap (op_desc_t * op_desc)
	{
	assert (op_desc->op_id == OP_XCHG);
	assert (op_desc->var_count == 2);

	op_var_t * var_from;
	op_var_t * var_to;

	var_to   = &op_desc->var_to;
	var_from = &op_desc->var_from;

	assert (var_to->size == var_from->size);

	op_val_t val_to;
	op_val_t val_from;

	memset (&val_to,   0, sizeof (op_val_t));
	memset (&val_from, 0, sizeof (op_val_t));

	val_get (var_to,   &val_to);
	val_get (var_from, &val_from);

	val_set (var_to,   &val_from);
	val_set (var_from, &val_to);
	}


// Input & output

static void op_port (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 2);

	op_val_t val_from;
	memset (&val_from, 0, sizeof (val_from));
	val_get (&op_desc->var_from, &val_from);

	op_val_t val_to;
	memset (&val_to, 0, sizeof (val_to));

	switch (op_desc->op_id)
		{
		case OP_IN:
			switch (op_desc->var_to.size)
				{
				case VS_BYTE:
					val_to.b = io_read_byte (val_from.w);
					break;

				case VS_WORD:
					val_to.w = io_read_word (val_from.w);
					break;

				default:
					assert (0);
				}

			val_set (&op_desc->var_to, &val_to);
			break;

		case OP_OUT:
			val_get (&op_desc->var_to, &val_to);

			switch (op_desc->var_from.size)
				{
				case VS_BYTE:
					io_write_byte (val_to.w, val_from.b);
					break;

				case VS_WORD:
					io_write_byte (val_to.w, val_from.w);
					break;

				default:
					assert (0);
				}
			break;

		default:
			assert (0);

		}
	}


// Arithmetic & logic

static void op_calc_1 (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);

	op_val_t val;
	val_get (&op_desc->var_to, &val);

	switch (op_desc->op_id)
		{
		case OP_NOT:
			val.w = !val.w;
			break;

		default:
			assert (0);

		}

	// TODO: set flags

	val_set (&op_desc->var_to, &val);
	}


static void op_calc_2 (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 2);

	op_var_t * var_from;
	op_var_t * var_to;

	var_to   = &op_desc->var_to;
	var_from = &op_desc->var_from;

	assert (var_to->size == var_from->size);
	byte_t s = var_to->size;

	op_val_t val_to;
	op_val_t val_from;

	memset (&val_to,   0, sizeof (op_val_t));
	memset (&val_from, 0, sizeof (op_val_t));

	val_get (var_to,   &val_to);
	val_get (var_from, &val_from);

	word_t a = val_to.w;
	word_t b = val_from.w;
	word_t r = 0;

	word_t co = 0;  // carry out
	word_t ci = 0;  // carry in
	word_t cf = 0;

	word_t id = op_desc->op_id;
	switch (id)
		{
		case OP_ADC:
			a += (word_t) flag_get (FLAG_CF);
			// TODO: check carries
			// no break

		case OP_ADD:
			r = a + b;
			co = (a & b) | ((a ^ b) & ~r);
			cf = co & (s == VS_WORD ? 0x8000: 0x0080);
			ci = a ^ b ^ r;
			break;

		case OP_SUB:
		case OP_CMP:
			r = a - b;
			co = (~a & b) | (~(a ^ b) & r);
			cf = co & (s == VS_WORD ? 0x8000: 0x0080);
			ci = a ^ b ^ r;
			break;

		case OP_OR:
			r = a | b;
			break;

		case OP_AND:
		case OP_TEST:
			r = a & b;
			break;

		case OP_XOR:
			r = a ^ b;
			break;

		default:
			assert (0);

		}

	// TODO: complete flags support

	flag_set (FLAG_ZF, r ? 0 : 1);
	flag_set (FLAG_CF, cf ? 1 : 0);

	if (id != OP_TEST && id != OP_CMP)
		{
		val_to.w  = r;
		val_set (var_to, &val_to);
		}
	}


// Increment & decrement

static void op_inc_dec (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);

	op_val_t val;
	memset (&val, 0, sizeof (op_val_t));
	val_get (&op_desc->var_to, &val);

	switch (op_desc->op_id)
		{
		case OP_INC:
			val.w++;
			break;

		case OP_DEC:
			val.w--;
			break;

		default:
			assert (0);
		}

	val_set (&op_desc->var_to, &val);
	}


// Shift & rotate

static void op_shift_rot (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 2);

	op_var_t * var_from;
	op_var_t * var_to;

	var_to   = &op_desc->var_to;
	var_from = &op_desc->var_from;

	assert (var_from->size == VS_BYTE);
	byte_t s = var_to->size;

	op_val_t val_to;
	op_val_t val_from;

	memset (&val_to,   0, sizeof (op_val_t));
	memset (&val_from, 0, sizeof (op_val_t));

	val_get (var_to,   &val_to);
	val_get (var_from, &val_from);

	word_t a = val_to.w;
	word_t b = val_from.b;
	word_t r = 0;

	switch (op_desc->op_id)
		{
		case OP_SHL:
			r  = a << b;
			break;

		case OP_SHR:
			r  = a >> b;
			break;

		default:
			assert (0);

		}

	val_to.w  = r;
	val_set (var_to, &val_to);
	}


// Push & pop

static void op_push (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);
	op_var_t * var = &op_desc->var_to;
	assert (var->size == VS_WORD);

	op_val_t val;
	val_get (var, &val);
	stack_push (val.w);
	}

static void op_pop (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);
	op_var_t * var = &op_desc->var_to;
	assert (var->size == VS_WORD);

	op_val_t val;
	val.w = stack_pop ();
	val_set (var, &val);
	}

static void op_pushf (op_desc_t * op_desc)
	{
	assert (!op_desc->var_count);
	stack_push (reg16_get (REG_FL));
	}

void op_popf (op_desc_t * op_desc)
	{
	assert (!op_desc->var_count);
	reg16_set (REG_FL, stack_pop ());
	}


// Jump & call

static void op_jump_call (op_desc_t * op_desc)
	{
	word_t h = (op_desc->op_id & 0xFF00);
	byte_t l = (op_desc->op_id & 0x00FF);
	assert (h == OP_JMP || h == OP_CALL);

	word_t ip = reg16_get (REG_IP);

	if (h == OP_CALL)
		{
		if (l & OF_FAR) stack_push (seg_get (SEG_CS));
		stack_push (ip);
		}

	assert (op_desc->var_count == 1);
	op_var_t * v = &(op_desc->var_to);

	if (l & OF_FAR)
		{
		assert (v->type == VT_FAR);
		reg16_set (REG_IP, v->val.w);
		seg_set (SEG_CS, v->seg);
		}
	else
		{
		assert (v->type == VT_NEAR);

		// TODO: group displacement operation

		switch (v->size)
			{
			case VS_BYTE:
				ip = (word_t) ((short) ip + (short) v->val.c);
				break;

			case VS_WORD:
				ip = (word_t) ((short) ip + v->val.s);
				break;

			default:
				assert (0);
			}

		reg16_set (REG_IP, ip);
		}
	}


// Interrupt

static void op_int (op_desc_t * op_desc)
	{
	byte_t i = 0;

	switch (op_desc->op_id)
		{
		case OP_INT:
			assert (op_desc->var_count == 1);
			op_var_t * var = &op_desc->var_to;
			assert (var->type == VT_IMM);
			assert (var->size == VS_BYTE);
			i = var->val.b;
			break;

		default:
			assert (0);

		}

	// Interrupt handling

	int err = int_hand (i);
	if (err)
		{
		// Not intercepted -> emulate

		assert (0);
		}
	}


// Return

static void op_return (op_desc_t * op_desc)
	{
	word_t ip = 0;

	assert (!op_desc->var_count);

	switch (op_desc->op_id)
		{
		case OP_RET:
			ip = stack_pop ();
			reg16_set (REG_IP, ip);
			break;

		default:
			assert (0);
		}
	}


// Jump conditional

static void op_jump_cond (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);
	op_var_t * var = &op_desc->var_to;
	assert (var->type == VT_NEAR);
	assert (var->size == VS_BYTE);

	op_val_t val;
	val_get (var, &val);
	short s = (short) val.c;

	switch (op_desc->op_id)
		{
		case OP_JB:
			s = flag_get (FLAG_CF) ? s : 0;
			break;

		case OP_JZ:
			s = flag_get (FLAG_ZF) ? s : 0;
			break;

		case OP_JNZ:
			s = flag_get (FLAG_ZF) ? 0 : s;
			break;

		case OP_JNA:
			s = (flag_get (FLAG_CF) || flag_get (FLAG_ZF)) ? s : 0;
			break;

		case OP_JA:
			s = (flag_get (FLAG_CF) || flag_get (FLAG_ZF)) ? 0 : s;
			break;

		default:
			assert (0);
		}

	if (s)
		{
		word_t ip = (word_t) ((short) reg16_get (REG_IP) + s);
		reg16_set (REG_IP, ip);
		}
	}


// String

static void op_string (op_desc_t * op_desc)
	{
	byte_t b;

	word_t ds,es;
	word_t si,di;

	switch (op_desc->op_id)
		{
		case OP_LODSB:
			ds = seg_get (SEG_DS);
			si = reg16_get (REG_SI);
			b = mem_read_byte (addr_seg_off (ds,si++));
			reg8_set (REG_AL, b);
			reg16_set (REG_SI, si);
			break;

		case OP_STOSB:
			es = seg_get (SEG_ES);
			di = reg16_get (REG_DI);
			b = reg8_get (REG_AL);
			mem_write_byte (addr_seg_off (es,di++), b);
			reg16_set (REG_DI, di);
			break;

		default:
			assert (0);

		}
	}


// Table of operation handlers

typedef void (* op_hand_t) (op_desc_t * op_desc);

struct op_id_hand_s
	{
	word_t id;
	op_hand_t hand;
	};

typedef struct op_id_hand_s op_id_hand_t;


static op_id_hand_t _id_hand_tab [] = {
	{ OP_MOV, op_move_load },
	{ OP_LEA, op_move_load },

	{ OP_XCHG, op_swap },

	{ OP_IN,  op_port },
	{ OP_OUT, op_port },

	{ OP_ADD,  op_calc_2 },
	{ OP_OR,   op_calc_2 },
	{ OP_ADC,  op_calc_2 },
	{ OP_SBB,  op_calc_2 },
	{ OP_AND,  op_calc_2 },
	{ OP_SUB,  op_calc_2 },
	{ OP_XOR,  op_calc_2 },
	{ OP_CMP,  op_calc_2 },
	{ OP_TEST, op_calc_2 },

	{ OP_NOT,  op_calc_1 },
	{ OP_NEG,  op_calc_1 },
	{ OP_MUL,  op_calc_1 },
	{ OP_IMUL, op_calc_1 },
	{ OP_DIV,  op_calc_1 },
	{ OP_IDIV, op_calc_1 },

	{ OP_INC, op_inc_dec },
	{ OP_DEC, op_inc_dec },

	{ OP_ROL, op_shift_rot },
	{ OP_ROR, op_shift_rot },
	{ OP_RCL, op_shift_rot },
	{ OP_RCR, op_shift_rot },
	{ OP_SHL, op_shift_rot },
	{ OP_SHR, op_shift_rot },
	{ OP_SAL, op_shift_rot },
	{ OP_SAR, op_shift_rot },

	{ OP_PUSH,  op_push },
	{ OP_POP,   op_pop },
	{ OP_PUSHF, op_pushf },
	{ OP_POPF,  op_popf },

	{ OP_JMP,   op_jump_call },
	{ OP_JMPF,  op_jump_call },
	{ OP_CALL,  op_jump_call },
	{ OP_CALLF, op_jump_call },

	{ OP_INT,  op_int },
	{ OP_INT3, op_int },
	{ OP_INTO, op_int },

	{ OP_RET,  op_return },
	{ OP_RETF, op_return },
	{ OP_IRET, op_return },

	{ OP_JO,  op_jump_cond },
	{ OP_JNO, op_jump_cond },
	{ OP_JB,  op_jump_cond },
	{ OP_JNB, op_jump_cond },
	{ OP_JZ,  op_jump_cond },
	{ OP_JNZ, op_jump_cond },
	{ OP_JNA, op_jump_cond },
	{ OP_JA,  op_jump_cond },
	{ OP_JS,  op_jump_cond },
	{ OP_JNS, op_jump_cond },
	{ OP_JP,  op_jump_cond },
	{ OP_JNP, op_jump_cond },
	{ OP_JL,  op_jump_cond },
	{ OP_JNL, op_jump_cond },
	{ OP_JNG, op_jump_cond },
	{ OP_JG,  op_jump_cond },

	{ OP_MOVSB, op_string },
	{ OP_MOVSW, op_string },
	{ OP_CMPSB, op_string },
	{ OP_CMPSW, op_string },
	{ OP_STOSB, op_string },
	{ OP_STOSW, op_string },
	{ OP_LODSB, op_string },
	{ OP_LODSW, op_string },
	{ OP_SCASB, op_string },
	{ OP_SCASW, op_string },

/*
#define OP_AAA   0x0D00
#define OP_AAD   0x0D01
#define OP_AAM   0x0D02
#define OP_AAS   0x0D03

#define OP_CBW   0x0E00
#define OP_CWD   0x0E01

#define OP_CLC   0x0F00
#define OP_CLD   0x0F01
#define OP_CLI   0x0F02
#define OP_CMC   0x0F03
#define OP_STC   0x0F04
#define OP_STD   0x0F05
#define OP_STI   0x0F06

// TODO: allocate op id

#define OP_WAIT   0xFFFF
#define OP_XLAT   0xFFFF
#define OP_ESC    0xFFFF
#define OP_DAA    0xFFFF
#define OP_DAS    0xFFFF
#define OP_HLT    0xFFFF
#define OP_JCXZ   0xFFFF
#define OP_LAHF   0xFFFF
#define OP_SAHF   0xFFFF
#define OP_LDS    0xFFFF
#define OP_LES    0xFFFF
#define OP_LOCK   0xFFFF
#define OP_LOOP   0xFFFF
#define OP_LOOPNZ 0xFFFF
#define OP_LOOPZ  0xFFFF
#define OP_REPNZ  0xFFFF
#define OP_REPZ   0xFFFF
#define OP_SEG    0xFFFF
*/

	{ 0, NULL }
	};


// Execute operation

int op_exec (op_desc_t * op_desc)
	{
	int err = -1;

	op_id_hand_t * desc = _id_hand_tab;

	while (1)
		{
		word_t id = desc->id;
		op_hand_t hand = desc->hand;
		if (!id && !hand)
			{
			assert (0);
			err = -1;
			break;
			}

		if (desc->id == op_desc->op_id)
			{
			(*hand) (op_desc);
			err = 0;
			break;
			}

		desc++;
		}

	return err;
	}
