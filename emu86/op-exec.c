// EMU86 - 80x86 emulator
// Operation handles

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "op-id.h"
#include "op-id-name.h"
#include "op-class.h"

#include "emu-proc.h"
#include "emu-mem-io.h"
#include "emu-int.h"
#include "op-exec.h"


// Offset & address

static word_t off_get (const op_var_t * var)
	{
	assert (var->type == VT_IND);

	byte_t f = var->flags;
	short s = 0;

	if (f & AF_BX) s += (short) reg16_get (REG_BX);
	if (f & AF_BP) s += (short) reg16_get (REG_BP);
	if (f & AF_SI) s += (short) reg16_get (REG_SI);
	if (f & AF_DI) s += (short) reg16_get (REG_DI);

	if (f & AF_DISP) s += var->val.s;

	return (word_t) s;
	}


// Get actual value of variable

static void val_get (const op_var_t * var1, op_var_t * var2)
	{
	const op_val_t * val1 = &var1->val;
	op_val_t * val2 = &var2->val;

	var2->w = var1->w;

	word_t o;
	word_t s;
	word_t a;

	switch (var1->type)
		{
		case VT_IMM:
			var2->type = VT_IMM;
			val2->w = val1->w;
			break;

		case VT_REG:
			var2->type = VT_IMM;
			if (var1->w)
				{
				val2->w = reg16_get (val1->r);
				}
			else
				{
				val2->b = reg8_get (val1->r);
				}

			break;

		case VT_SEG:
			var2->type = VT_IMM;
			val2->w = seg_get (val1->r);
			break;

		case VT_LOC:
			var2->type = VT_LOC;
			var2->far = var1->far;
			val2->w = val1->w;
			var2->seg = var1->seg;
			break;

		case VT_IND:

			// TODO: segment override

			s = (var1->flags & AF_BP) ? seg_get (SEG_SS) : seg_get (SEG_DS);
			o = off_get (var1);
			a = addr_seg_off (s, o);

			var2->type = VT_IMM;
			var2->far = var1->far;

			if (var1->far)
				{
				val2->w = mem_read_word (a);
				var2->seg = mem_read_word (a + 2);
				}
			else
				{
				if (var1->w)
					{
					val2->w = mem_read_word (a);
					}
				else
					{
					val2->b = mem_read_byte (a);
					}
				}

			break;

		default:
			assert (0);

		}
	}


// Set actual value of variable

static void val_set (op_var_t * var1, const op_var_t * var2)
	{
	op_val_t * val1 = &var1->val;
	const op_val_t * val2 = &var2->val;

	assert (var2->type == VT_IMM);
	assert (var1->w == var2->w);

	word_t o;
	word_t s;
	addr_t a;

	switch (var1->type)
		{
		case VT_REG:
			if (var1->w)
				{
				reg16_set (val1->r, val2->w);
				}
			else
				{
				reg8_set (val1->r, val2->b);
				}

			break;

		case VT_SEG:
			seg_set (val1->r, val2->w);
			break;

		case VT_IND:
			assert (!var1->far);

			// TODO: add segment base

			s = (var1->flags & AF_BP) ? seg_get (SEG_SS) : seg_get (SEG_DS);
			o = off_get (var1);
			a = addr_seg_off (s, o);

			if (var1->w)
				{
				mem_write_word (a, val2->w);
				}
			else
				{
				mem_write_byte (a, val2->b);
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
	op_var_t * to   = &op_desc->var_to;
	op_var_t * from = &op_desc->var_from;
	assert (to->w == from->w);

	op_var_t temp;
	memset (&temp, 0, sizeof (op_var_t));

	switch (op_desc->op_id)
		{
		case OP_MOV:
			val_get (from, &temp);
			assert (temp.type == VT_IMM);
			assert (to->w == temp.w);
			val_set (to, &temp);
			break;

		case OP_LEA:
			temp.type = VT_IMM;
			temp.w = 1;
			temp.val.w = off_get (from);
			assert (to->w == temp.w);
			val_set (to, &temp);
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
	op_var_t * to   = &op_desc->var_to;
	op_var_t * from = &op_desc->var_from;
	assert (to->w == from->w);

	op_var_t temp1;
	op_var_t temp2;
	memset (&temp1, 0, sizeof (op_var_t));
	memset (&temp2, 0, sizeof (op_var_t));

	val_get (to,   &temp1);
	val_get (from, &temp2);
	assert (temp1.w == temp2.w);

	assert (to->w == temp2.w);
	val_set (to, &temp2);

	assert (from->w == temp1.w);
	val_set (from, &temp1);
	}


// Input & output

static void op_port (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 2);
	op_var_t * to   = &op_desc->var_to;
	op_var_t * from = &op_desc->var_from;

	op_var_t port;
	op_var_t data;
	memset (&port, 0, sizeof (op_var_t));
	memset (&data, 0, sizeof (op_var_t));

	switch (op_desc->op_id)
		{
		case OP_IN:
			val_get (from, &port);
			assert (port.type == VT_IMM);
			assert (port.w);

			data.type = VT_IMM;
			data.w = to->w;
			if (data.w)
				{
				data.val.w = io_read_word (port.val.w);
				}
			else
				{
				data.val.b = io_read_byte (port.val.w);
				}

			val_set (to, &data);
			break;

		case OP_OUT:
			val_get (to, &port);
			assert (port.type == VT_IMM);
			assert (port.w);
			val_get (from, &data);

			if (data.w)
				{
				io_write_byte (port.val.w, data.val.w);
				}
			else
				{
				io_write_byte (port.val.w, data.val.b);
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
	op_var_t * var = &op_desc->var_to;

	op_var_t temp;
	memset (&temp, 0, sizeof (op_var_t));

	val_get (var, &temp);
	assert (temp.type == VT_IMM);

	switch (op_desc->op_id)
		{
		case OP_NOT:
			temp.val.w = ~temp.val.w;
			break;

		default:
			assert (0);

		}

	// TODO: set flags

	val_set (var, &temp);
	}


static void op_calc_2 (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 2);
	op_var_t * to   = &op_desc->var_to;
	op_var_t * from = &op_desc->var_from;
	assert (to->w == from->w);

	op_var_t temp1;
	op_var_t temp2;
	memset (&temp1, 0, sizeof (op_var_t));
	memset (&temp2, 0, sizeof (op_var_t));

	val_get (to,   &temp1);
	val_get (from, &temp2);
	assert (temp1.type == VT_IMM);
	assert (temp2.type == VT_IMM);
	assert (temp1.w == temp2.w);

	word_t a = temp1.val.w;
	word_t b = temp2.val.w;
	word_t r = 0;

	word_t co = 0;  // carry out
	word_t ci = 0;  // carry in
	word_t cf = 0;

	word_t id = op_desc->op_id;
	switch (id)
		{
		case OP_ADC:
			r += (word_t) flag_get (FLAG_CF);
			// no break

		case OP_ADD:
			r += a + b;
			co = (a & b) | ((a ^ b) & ~r);
			cf = co & (to->w ? 0x8000: 0x0080);
			ci = a ^ b ^ r;
			break;

		case OP_SBB:
			r -= (word_t) flag_get (FLAG_CF);
			// no break

		case OP_SUB:
		case OP_CMP:
			r += a - b;
			co = (~a & b) | (~(a ^ b) & r);
			cf = co & (to->w ? 0x8000: 0x0080);
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
		temp1.val.w = r;
		val_set (to, &temp1);
		}
	}


// Increment & decrement

static void op_inc_dec (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);
	op_var_t * var = &op_desc->var_to;

	op_var_t temp;
	memset (&temp, 0, sizeof (op_var_t));

	val_get (var, &temp);
	assert (temp.type == VT_IMM);
	word_t r = temp.val.w;

	switch (op_desc->op_id)
		{
		case OP_INC:
			r++;
			break;

		case OP_DEC:
			r--;
			break;

		default:
			assert (0);
		}

	// TODO: update flags

	flag_set (FLAG_ZF, r ? 0 : 1);

	temp.val.w = r;
	val_set (var, &temp);
	}


// Shift & rotate

static void op_shift_rot (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 2);
	op_var_t * to   = &op_desc->var_to;
	op_var_t * from = &op_desc->var_from;

	op_var_t temp1;
	op_var_t temp2;
	memset (&temp1, 0, sizeof (op_var_t));
	memset (&temp2, 0, sizeof (op_var_t));

	val_get (to,   &temp1);
	val_get (from, &temp2);
	assert (temp1.type == VT_IMM);
	assert (temp2.type == VT_IMM);

	byte_t w = temp1.w;
	assert (!temp2.w);

	word_t a = temp1.val.w;
	word_t b = temp2.val.b;
	word_t t = b;

	word_t c;  // carry
	word_t q;

	word_t id = op_desc->op_id;
	switch (id)
		{
		case OP_ROL:
		case OP_SHL:
			while (t--)
				{
				c = a & (w ? 0x8000 : 0x0080);
				c = c ? 1 : 0;

				a  <<= 1;
				if (id == OP_ROL) a |= c;
				}

			flag_set (FLAG_CF, c);

			if (b == 1)
				{
				t = a & (w ? 0x8000 : 0x0080);
				t = t ? 1 : 0;

				flag_set (FLAG_OF, c ^ t);
				}

			break;

		case OP_ROR:
		case OP_SHR:
			while (t--)
				{
				c = a & 0x0001;
				c = c ? 1 : 0;

				a  >>= 1;
				if (id == OP_ROR) a |= c ? (w ? 0x8000 : 0x0080) : 0;
				}

			flag_set (FLAG_CF, c);

			if (b == 1)
				{
				t = a & (w ? 0x8000 : 0x0080);
				t = t ? 1 : 0;

				q = a & (w ? 0x4000 : 0x0040);
				q = q ? 1 : 0;

				flag_set (FLAG_OF, t ^ q);
				}

			break;

		default:
			assert (0);

		}

	temp1.val.w = a;
	val_set (to, &temp1);
	}


// Push & pop

static void op_push (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);
	op_var_t * var = &op_desc->var_to;
	assert (var->w);

	op_var_t temp;
	memset (&temp, 0, sizeof (op_var_t));

	val_get (var, &temp);
	assert (temp.w);
	stack_push (temp.val.w);
	}

static void op_pop (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);
	op_var_t * var = &op_desc->var_to;
	assert (var->w);

	op_var_t temp;
	memset (&temp, 0, sizeof (op_var_t));
	temp.type = VT_IMM;
	temp.w = 1;
	temp.val.w = stack_pop ();

	val_set (var, &temp);
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
	word_t op = op_desc->op_id;
	assert (op == OP_JMP || op == OP_CALL || op == OP_JMPF || op == OP_CALLF);

	assert (op_desc->var_count == 1);
	op_var_t * var = &(op_desc->var_to);

	byte_t t = var->type;
	assert (t == VT_LOC || t == VT_IND);

	op_var_t temp;
	memset (&temp, 0, sizeof (op_var_t));
	val_get (var, &temp);

	word_t ip = reg16_get (REG_IP);

	if (op == OP_CALL || op == OP_CALLF)
		{
		if (op == OP_CALLF) stack_push (seg_get (SEG_CS));
		stack_push (ip);
		}

	if (temp.far)
		{
		assert (op == OP_JMPF || op == OP_CALLF);

		// Far address is absolute

		ip = temp.val.w;
		seg_set (SEG_CS, temp.seg);
		}
	else
		{
		assert (op == OP_JMP || op == OP_CALL);

		// Near address are relative

		ip = (word_t) ((short) ip + temp.val.s);
		}

	reg16_set (REG_IP, ip);
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
			assert (!var->w);
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
	word_t op = op_desc->op_id;
	assert (op == OP_RET || op == OP_RETF);

	// TODO: implement stack unwind

	assert (!op_desc->var_count);

	reg16_set (REG_IP, stack_pop ());

	if (op == OP_RETF)
		{
		seg_set (SEG_CS, stack_pop ());
		}
	}


// Jump conditional

// TODO: Optimize with LUT on mask & value

static void op_jump_cond (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);
	op_var_t * var = &op_desc->var_to;
	assert (var->type == VT_LOC);
	assert (!var->far);

	byte_t f;

	switch (op_desc->op_id)
		{
		case OP_JO:
			f = flag_get (FLAG_OF);
			break;

		case OP_JNO:
			f = !flag_get (FLAG_OF);
			break;

		case OP_JB:
			f = flag_get (FLAG_CF);
			break;

		case OP_JNB:
			f = !flag_get (FLAG_CF);
			break;

		case OP_JZ:
			f = flag_get (FLAG_ZF);
			break;

		case OP_JNZ:
			f = !flag_get (FLAG_ZF);
			break;

		case OP_JNA:
			f = flag_get (FLAG_CF) || flag_get (FLAG_ZF);
			break;

		case OP_JA:
			f = !(flag_get (FLAG_CF) || flag_get (FLAG_ZF));
			break;

		case OP_JS:
			f = flag_get (FLAG_SF);
			break;

		case OP_JNS:
			f = !flag_get (FLAG_SF);
			break;

		case OP_JP:
			f = flag_get (FLAG_PF);
			break;

		case OP_JNP:
			f = !flag_get (FLAG_PF);
			break;

		default:
			assert (0);
		}

	if (f)
		{
		word_t ip = (word_t) ((short) reg16_get (REG_IP) + var->val.s);
		reg16_set (REG_IP, ip);
		}
	}

/*
{ OP_JL,  op_jump_cond },
{ OP_JNL, op_jump_cond },
{ OP_JNG, op_jump_cond },
{ OP_JG,  op_jump_cond },
*/


// String

static void op_string (op_desc_t * op_desc)
	{
	byte_t b;

	word_t ds,es;
	word_t si,di;

	switch (op_desc->op_id)
		{
		case OP_LODSB:

			// TODO: segment override

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


// Flags

static void op_flag (op_desc_t * op_desc)
	{
	switch (op_desc->op_id)
		{
		case OP_CLC:
			flag_set (FLAG_CF, 0);
			break;

		case OP_CLD:
			flag_set (FLAG_DF, 0);
			break;

		case OP_CLI:
			flag_set (FLAG_IF, 0);
			break;

		case OP_CMC:
			flag_set (FLAG_CF, ~flag_get (FLAG_CF));
			break;

		case OP_STC:
			flag_set (FLAG_CF, 1);
			break;

		case OP_STD:
			flag_set (FLAG_DF, 1);
			break;

		case OP_STI:
			flag_set (FLAG_IF, 1);
			break;

		default:
			assert (0);

		}
	}


// Flags and accumulator

static void op_flag_acc (op_desc_t * op_desc)
	{
	word_t fl;
	byte_t ah;

	switch (op_desc->op_id)
		{
		case OP_LAHF:
			ah = (byte_t) (reg16_get (REG_FL) & 0x00FF);
			reg8_set (REG_AH, ah);
			break;

		case OP_SAHF:
			fl = reg16_get (REG_FL) & 0xFF00;
			ah = reg8_get (REG_AH);
			reg16_set (REG_FL, fl  | (word_t) ah);
			break;

		default:
			assert (0);

		}
	}


// Halt

static void op_halt (op_desc_t * op_desc)
	{
	assert (op_desc->op_id == OP_HLT);
	assert (0);
	}


// Loop

static void op_loop (op_desc_t * op_desc)
	{
	assert (op_desc->op_id == OP_LOOP);
	assert (op_desc->var_count == 1);

	op_var_t * var = &op_desc->var_to;
	assert (var->type == VT_LOC);
	assert (!var->far);

	word_t cx = reg16_get (REG_CX);
	reg16_set (REG_CX, --cx);

	if (cx)
		{
		word_t ip = (word_t) ((short) reg16_get (REG_IP) + var->val.s);
		reg16_set (REG_IP, ip);
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
*/

	{ OP_CLC, op_flag },
	{ OP_CLD, op_flag },
	{ OP_CLI, op_flag },
	{ OP_CMC, op_flag },
	{ OP_STC, op_flag },
	{ OP_STD, op_flag },
	{ OP_STI, op_flag },

	{ OP_SAHF, op_flag_acc },
	{ OP_LAHF, op_flag_acc },

	{ OP_HLT, op_halt },

	{ OP_LOOP, op_loop },

// TODO: allocate op id

	/*
#define OP_WAIT   0xFFFF
#define OP_XLAT   0xFFFF
#define OP_ESC    0xFFFF
#define OP_DAA    0xFFFF
#define OP_DAS    0xFFFF
#define OP_JCXZ   0xFFFF
#define OP_LDS    0xFFFF
#define OP_LES    0xFFFF
#define OP_LOCK   0xFFFF
#define OP_LOOPNZ 0xFFFF
#define OP_LOOPZ  0xFFFF
#define OP_REPNZ  0xFFFF
#define OP_REPZ   0xFFFF
#define OP_SEG    0xFFFF
*/

	{ 0, NULL }
	};


// Execute operation

static word_t _last_id = 0;
static op_hand_t _last_hand = NULL;

int op_exec (op_desc_t * op_desc)
	{
	int err = -1;

	word_t id1 = op_desc->op_id;
	op_id_hand_t * desc = _id_hand_tab;

	while (1)
		{
		// Optimization for repeated instruction

		if (id1 == _last_id && _last_hand)
			{
			(*_last_hand) (op_desc);
			err = 0;
			break;
			}

		// Table lookup

		while (1)
			{
			word_t id2 = desc->id;
			op_hand_t hand = desc->hand;
			if (!id2 && !hand)
				{
				assert (0);
				err = -1;
				break;
				}

			if (id2 == id1)
				{
				_last_id = id2;
				_last_hand = hand;

				(*hand) (op_desc);
				err = 0;
				break;
				}

			desc++;
			}

		break;
		}

	return err;
	}
