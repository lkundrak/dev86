// EMU86 - 80x86 emulator
// Operation handles

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "op-id.h"
#include "op-id-name.h"

#include "op-class.h"

#include "emu-mem-io.h"
#include "emu-proc.h"
#include "emu-int.h"

#include "op-exec.h"


// Segment prefix

// 1: prefix found
// 2: prefix ready
// 3: prefix active

static byte_t _seg_stat = 0;
static byte_t _seg_reg = 0xFF;

int seg_none ()
	{
	return (_seg_stat == 0);
	}

void seg_reset ()
	{
	if (_seg_stat == 3)
		{
		_seg_stat = 0;
		_seg_reg = 0xFF;
		}
	}


// Repeat prefix

static byte_t _rep_stat = 0;
static word_t _rep_op = OP_NULL;

int rep_none ()
	{
	return (_rep_stat == 0);
	}

int rep_active ()
	{
	return (_rep_stat == 3);
	}

void rep_reset ()
	{
	_rep_stat = 0;
	_rep_op = OP_NULL;
	}


// Offset & address

static word_t off_get (const op_var_t * var)
	{
	assert (var->type == VT_MEM);

	byte_t f = var->flags;
	short s = 0;

	if (f & AF_BX) s += (short) reg16_get (REG_BX);
	if (f & AF_BP) s += (short) reg16_get (REG_BP);
	if (f & AF_SI) s += (short) reg16_get (REG_SI);
	if (f & AF_DI) s += (short) reg16_get (REG_DI);

	if (f & AF_DISP) s += var->val.s;

	return (word_t) s;
	}


static word_t seg_over_get (const op_var_t * var)
	{
	word_t s;

	if (_seg_stat == 2)
		{
		_seg_stat = 3;
		}

	if (_seg_stat == 3)
		{
		assert (_seg_reg < SEG_MAX);
		s = seg_get (_seg_reg);
		}
	else
		{
		if (var)
			{
			s = (var->flags & AF_BP) ? seg_get (SEG_SS) : seg_get (SEG_DS);
			}
		else
			{
			s = seg_get (SEG_DS);
			}
		}

	return s;
	}


static addr_t addr_get (const op_var_t * var)
	{
	word_t s = seg_over_get (var);
	word_t o = off_get (var);
	return addr_seg_off (s, o);
	}


// Get actual value of variable

static void val_get (const op_var_t * var1, op_var_t * var2)
	{
	const op_val_t * val1 = &var1->val;
	op_val_t * val2 = &var2->val;

	var2->w = var1->w;

	addr_t a;

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

		case VT_MEM:

			a = addr_get (var1);

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

		case VT_MEM:
			assert (!var1->far);

			a = addr_get (var1);

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

static int op_move_load (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 2);
	op_var_t * to   = &op_desc->var_to;
	op_var_t * from = &op_desc->var_from;
	assert (to->w == from->w);

	op_var_t temp;
	memset (&temp, 0, sizeof (op_var_t));

	switch (OP_ID)
		{
		case OP_MOV:
			val_get (from, &temp);
			assert (temp.type == VT_IMM);
			assert (from->w == temp.w);
			val_set (to, &temp);
			break;

		case OP_LEA:
			temp.type = VT_IMM;
			temp.w = 1;
			temp.val.w = off_get (from);
			val_set (to, &temp);
			break;

		case OP_LDS:
			assert (from->far);
			val_get (from, &temp);
			assert (temp.type == VT_IMM);
			assert (temp.far);

			temp.far = 0;
			temp.w = 1;
			val_set (to, &temp);

			seg_set (SEG_DS, temp.seg);
			break;

		default:
			assert (0);

		}

	return 0;
	}


// Swap

static int op_swap (op_desc_t * op_desc)
	{
	assert (OP_ID == OP_XCHG);

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

	return 0;
	}


// Input & output

static int op_port (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 2);
	op_var_t * to   = &op_desc->var_to;
	op_var_t * from = &op_desc->var_from;

	op_var_t port;
	op_var_t data;
	memset (&port, 0, sizeof (op_var_t));
	memset (&data, 0, sizeof (op_var_t));

	switch (OP_ID)
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

	return 0;
	}


// Arithmetic & logic

static int op_calc_1 (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);
	op_var_t * var = &op_desc->var_to;

	op_var_t temp;
	memset (&temp, 0, sizeof (op_var_t));

	val_get (var, &temp);
	assert (temp.type == VT_IMM);
	word_t v = temp.val.w;

	word_t id = OP_ID;
	switch (id)
		{
		case OP_NOT:
			v = ~v;
			break;

		case OP_MUL:
			if (temp.w)
				{
				assert (0);  // not implemented
				/*
				word_t ax = reg16_get (REG_AX);
				word_t dx = reg16_get (REG_DX);
				addr_t t = ((addr_t) dx) * 0x10000 + (addr_t) ax;
				reg16_set (REG_AX, (word_t) (t / (addr_t) v));
				reg16_set (REG_DX, (word_t) (t % (addr_t) v));
				*/
				}
			else
				{
				byte_t al = reg8_get (REG_AL);
				reg16_set (REG_AX, ((word_t) al) * v);
				}

			break;

		case OP_IMUL:
			if (temp.w)
				{
				word_t ax = reg16_get (REG_AX);
				word_t dx = reg16_get (REG_DX);
				long t = ((long) (short) dx) * (long) 0x10000 + (long) (short) ax;
				reg16_set (REG_AX, (word_t) (t / (long) (short) v));
				reg16_set (REG_DX, (word_t) (t % (long) (short) v));
				}
			else
				{
				byte_t al = reg8_get (REG_AL);
				short s = ((short) (char) al) * (short) v;
				reg16_set (REG_AX, (word_t) s);
				}

			break;


		case OP_DIV:
			assert (v > 0);

			if (temp.w)
				{
				word_t ax = reg16_get (REG_AX);
				word_t dx = reg16_get (REG_DX);
				dword_t t = ((dword_t) dx) * 0x10000 + (dword_t) ax;
				reg16_set (REG_AX, (word_t) (t / (dword_t) v));
				reg16_set (REG_DX, (word_t) (t % (dword_t) v));
				}
			else
				{
				word_t ax = reg16_get (REG_AX);
				reg8_set (REG_AL, (byte_t) (ax / v));
				reg8_set (REG_AH, (byte_t) (ax % v));
				}

			break;

		default:
			printf ("fatal: unknown op %x\n", id);
			assert (0);

		}

	// TODO: set flags

	if (id == OP_NOT)
		{
		temp.val.w = v;
		val_set (var, &temp);
		}

	return 0;
	}


// TODO: to be moved to emu-proc ?

static word_t alu_calc_2 (word_t op, byte_t w, word_t a, word_t b)
	{
	word_t r = 0;

	word_t cf = 0;
	word_t of = 0;

	word_t co;  // carry out
	word_t ci;  // carry in

	word_t m = w ? 0x8000: 0x0080;

	switch (op)
		{
		case OP_ADC:
			r += (word_t) flag_get (FLAG_CF);
			// no break

		case OP_ADD:
			r += a + b;

			co = (a & b) | ((a ^ b) & ~r);
			ci = a ^ b ^ r;

			cf = co & m;
			of = ci & m;
			break;

		case OP_SBB:
			r -= (word_t) flag_get (FLAG_CF);
			// no break

		case OP_SUB:
		case OP_CMP:
			r += a - b;

			co = (~a & b) | (~(a ^ b) & r);
			ci = a ^ b ^ r;

			cf = co & m;
			of = ci & m;
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

	cf = cf ? 1 : 0;
	of = cf ^ (of ? 1 : 0);

	flag_set (FLAG_CF, cf);
	flag_set (FLAG_OF, of);

	flag_set (FLAG_ZF, r ? 0 : 1);
	flag_set (FLAG_SF, (r & m) ? 1 : 0);

	return r;
	}


static int op_calc_2 (op_desc_t * op_desc)
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

	word_t op = OP_ID;

	word_t a = temp1.val.w;
	word_t b = temp2.val.w;
	word_t r = alu_calc_2 (op, temp1.w, a, b);

	if (op != OP_TEST && op != OP_CMP)
		{
		temp1.val.w = r;
		val_set (to, &temp1);
		}

	return 0;
	}


// Increment & decrement

static int op_inc_dec (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);
	op_var_t * var = &op_desc->var_to;

	op_var_t temp;
	memset (&temp, 0, sizeof (op_var_t));

	val_get (var, &temp);
	assert (temp.type == VT_IMM);
	word_t r = temp.val.w;

	switch (OP_ID)
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

	return 0;
	}


// Shift & rotate

static int op_shift_rot (op_desc_t * op_desc)
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

	word_t d;  // previous carry
	word_t c = flag_get (FLAG_CF);  // carry
	word_t q;

	word_t id = OP_ID;
	switch (id)
		{
		case OP_RCL:
		case OP_ROL:
		case OP_SHL:
			while (t--)
				{
				d = c;
				c = a & (w ? 0x8000 : 0x0080);
				c = c ? 1 : 0;

				a  <<= 1;
				if (id == OP_RCL) a |= d;
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

		case OP_RCR:
		case OP_ROR:
		case OP_SAR:
		case OP_SHR:
			while (t--)
				{
				d = c;
				c = a & 0x0001;
				c = c ? 1 : 0;

				a  >>= 1;
				if (id == OP_RCR) a |= d ? (w ? 0x8000 : 0x0080) : 0;
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

	return 0;
	}


// Push & pop

static int op_push (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);
	op_var_t * var = &op_desc->var_to;
	assert (var->w);

	op_var_t temp;
	memset (&temp, 0, sizeof (op_var_t));

	val_get (var, &temp);
	assert (temp.w);
	stack_push (temp.val.w);

	return 0;
	}

static int op_pop (op_desc_t * op_desc)
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

	return 0;
	}


static int op_pushf (op_desc_t * op_desc)
	{
	assert (!op_desc->var_count);
	stack_push (reg16_get (REG_FL));
	return 0;
	}

static int op_popf (op_desc_t * op_desc)
	{
	assert (!op_desc->var_count);
	reg16_set (REG_FL, stack_pop ());
	return 0;
	}


// 80186 / 80188 PUSHA / POPA

static int op_pusha (op_desc_t * op_desc)
	{
	assert (!op_desc->var_count);
	byte_t r;

	for (r = 0; r != 8; r++)
		stack_push (reg16_get (r));

	return 0;
	}

static int op_popa (op_desc_t * op_desc)
	{
	assert (!op_desc->var_count);
	byte_t r;

	for (r = 7; r != 0xFF; r--)
		{
		if (r == REG_SP)
			{
			stack_pop ();  // ignore pushed SP
			}
		else
			{
			reg16_set (r, stack_pop ());
			}
		}

	return 0;
	}


// Jump & call

static int op_jump_call (op_desc_t * op_desc)
	{
	word_t op = OP_ID;
	assert (op == OP_JMP || op == OP_CALL || op == OP_JMPF || op == OP_CALLF);

	assert (op_desc->var_count == 1);
	op_var_t * var = &(op_desc->var_to);

	byte_t t = var->type;
	assert (t == VT_REG || t == VT_LOC || t == VT_MEM);

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
		assert (t != VT_REG);
		assert (op == OP_JMPF || op == OP_CALLF);

		// Far address is absolute

		ip = temp.val.w;
		seg_set (SEG_CS, temp.seg);
		}
	else
		{
		assert (op == OP_JMP || op == OP_CALL);

		if (t == VT_REG || t == VT_MEM)
			{
			// Register or indirect is absolute

			ip = temp.val.w;
			}
		else
			{
			// VT_LOC is relative

			ip = (word_t) ((short) ip + temp.val.s);
			}
		}

	reg16_set (REG_IP, ip);

	return 0;
	}


// Interrupt

int exec_int (byte_t i)
	{
	int err = -1;

	// Check interrupt vector first

	addr_t vect = ((addr_t) i) << 2;
	word_t ip = mem_read_word (vect);
	word_t cs = mem_read_word (vect + 2);

	if (ip != 0xFFFF && cs != 0xFFFF)
		{
		// Emulate if vector initialized

		stack_push (reg16_get (REG_FL));
		stack_push (seg_get (SEG_CS));
		stack_push (reg16_get (REG_IP));

		reg16_set (REG_IP, ip);
		seg_set (SEG_CS, cs);

		flag_set (FLAG_TF, 0);
		flag_set (FLAG_IF, 0);

		err = 0;
		}
	else
		{
		// No vector initialized
		// Use emulator default handler

		err = int_hand (i);
		}

	return err;
	}


static int op_int (op_desc_t * op_desc)
	{
	int err = -1;

	byte_t i = 0;

	switch (OP_ID)
		{
		case OP_INT:
			assert (op_desc->var_count == 1);
			op_var_t * var = &op_desc->var_to;
			assert (var->type == VT_IMM);
			assert (!var->w);
			i = var->val.b;
			break;

		case OP_INT3:
			assert (op_desc->var_count == 0);
			i = 0x03;
			break;

		default:
			assert (0);

		}

	err = exec_int (i);
	return err;
	}


// Return

static int op_return (op_desc_t * op_desc)
	{
	word_t op = OP_ID;
	assert (op == OP_RET || op == OP_RETF || op == OP_IRET);

	// TODO: implement stack unwind

	assert (!op_desc->var_count);

	reg16_set (REG_IP, stack_pop ());

	if (op == OP_RETF || op == OP_IRET)
		{
		seg_set (SEG_CS, stack_pop ());
		}

	if (op == OP_IRET)
		{
		reg16_set (REG_FL, stack_pop ());
		}

	return 0;
	}


// Jump conditional

// TODO: Optimize with LUT on mask & value

static int op_jump_cond (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);
	op_var_t * var = &op_desc->var_to;
	assert (var->type == VT_LOC);
	assert (!var->far);

	byte_t f;

	switch (OP_ID)
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

		case OP_JNL:
			f = flag_get (FLAG_SF) == flag_get (FLAG_OF);
			break;

		case OP_JL:
			f = flag_get (FLAG_SF) != flag_get (FLAG_OF);
			break;

		case OP_JNG:
			f = (flag_get (FLAG_SF) != flag_get (FLAG_OF)) || flag_get (FLAG_ZF);
			break;

		case OP_JG:
			f = (flag_get (FLAG_SF) == flag_get (FLAG_OF)) && !flag_get (FLAG_ZF);
			break;

		default:
			assert (0);
		}

	if (f)
		{
		word_t ip = (word_t) ((short) reg16_get (REG_IP) + var->val.s);
		reg16_set (REG_IP, ip);
		}

	return 0;
	}


// Segment override with SEG prefix

static int op_seg (op_desc_t * op_desc)
	{
	assert (OP_ID == OP_SEG);
	assert (_seg_reg == 0xFF);
	assert (_seg_stat == 0);

	assert (op_desc->var_count == 1);
	op_var_t * var = &op_desc->var_to;
	assert (var->type == VT_SEG);
	assert (var->val.r < SEG_MAX);
	_seg_reg = var->val.r;
	_seg_stat = 1;

	return 0;
	}


// Strings

static int op_repeat (op_desc_t * op_desc)
	{
	assert (_rep_op == OP_NULL);
	assert (_rep_stat == 0);
	word_t id = OP_ID;
	assert (id == OP_REPZ || id == OP_REPNZ);
	_rep_op = id;
	_rep_stat = 1;

	return 0;
	}


static int op_string (op_desc_t * op_desc)
	{
	// TODO: simplify with W flag at operation level

	while (1)
		{
		// Repeat prefix

		word_t cx;

		if (_rep_stat == 2)
			{
			_rep_stat = 3;
			}

		if (_rep_stat == 3)
			{
			word_t cx = reg16_get (REG_CX);
			if (!cx)
				{
				rep_reset ();
				break;
				}

			reg16_set (REG_CX, --cx);
			}

		word_t id = OP_ID;
		byte_t w = op_desc->w2;

		word_t d = flag_get (FLAG_DF) ? -1 : 1;
		d <<= w;

		word_t seg;
		word_t si,di;
		word_t a;
		word_t b;

		if (id == OP_LODS || id == OP_MOVS || id == OP_CMPS)
			{
			seg = seg_over_get (NULL);
			si = reg16_get (REG_SI);

			if (w)
				{
				a = mem_read_word (addr_seg_off (seg, si));
				if (id == OP_LODS) reg16_set (REG_AX, a);
				}
			else
				{
				a = (word_t) mem_read_byte (addr_seg_off (seg, si));
				if (id == OP_LODS) reg8_set (REG_AL, (byte_t) a);
				}

			reg16_set (REG_SI, si + d);
			}

		if (id == OP_STOS || id == OP_MOVS || id == OP_CMPS)
			{
			seg = seg_get (SEG_ES);
			di = reg16_get (REG_DI);

			if (id == OP_STOS || id == OP_MOVS)
				{
				if (w)
					{
					if (id == OP_STOS) a = reg16_get (REG_AX);
					mem_write_word (addr_seg_off (seg, di), a);
					}
				else
					{
					if (id == OP_STOS) a = (word_t) reg8_get (REG_AL);
					mem_write_byte (addr_seg_off (seg, di), (byte_t) a);
					}
				}

			if (id == OP_CMPS)
				{
				if (w)
					{
					b = mem_read_word (addr_seg_off (seg, di));
					}
				else
					{
					b = (word_t) mem_read_byte (addr_seg_off (seg, di));
					}
				}

			reg16_set (REG_DI, di + d);
			}

		if (id == OP_CMPS)
			{
			alu_calc_2 (OP_CMP, w, a, b);  // forget result - just for flags

			if (_rep_stat == 3)
				{
				byte_t z = flag_get (FLAG_ZF);

				// TODO: looks like an XOR would be nice here

				if ((_rep_op == OP_REPZ && !z) || (_rep_op == OP_REPNZ && z))
					{
					rep_reset ();
					}
				}
			}

		break;
		}

	return 0;
	}


// Conversion

static int op_convert (op_desc_t * op_desc)
	{
	byte_t ah;
	word_t dx;

	switch (OP_ID)
		{
		case OP_CBW:
			ah = (reg8_get (REG_AL) & 0x80) ? 0xFF : 0;
			reg8_set (REG_AH, ah);
			break;

		case OP_CWD:
			dx = (reg16_get (REG_AX) & 0x8000) ? 0xFFFF: 0;
			reg16_set (REG_DX, dx);
			break;

		default:
			assert (0);

		}

	return 0;
	}

// Flags

static int op_flag (op_desc_t * op_desc)
	{
	switch (OP_ID)
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

	return 0;
	}


// Flags and accumulator

static int op_flag_acc (op_desc_t * op_desc)
	{
	word_t fl;
	byte_t ah;

	switch (OP_ID)
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

	return 0;
	}


// Halt

static int op_halt (op_desc_t * op_desc)
	{
	assert (OP_ID == OP_HLT);
	assert (0);
	return 0;
	}

// No operation

static int op_nop (op_desc_t * op_desc)
	{
	assert (OP_ID == OP_NOP);
	return 0;
	}


// Loop

static int op_loop (op_desc_t * op_desc)
	{
	assert (op_desc->var_count == 1);

	op_var_t * var = &op_desc->var_to;
	assert (var->type == VT_LOC);
	assert (!var->far);

	word_t cx = reg16_get (REG_CX);
	reg16_set (REG_CX, --cx);

	byte_t j = 0;  // jump flag

	word_t id = OP_ID;
	switch (id)
		{
		case OP_LOOP:
			j = (cx != 0);
			break;

		case OP_LOOPNZ:
			j = (cx != 0) && !flag_get (FLAG_ZF);
			break;

		case OP_LOOPZ:
			j = (cx != 0) && flag_get (FLAG_ZF);
			break;

		default:
			assert (0);
		}

	if (j)
		{
		word_t ip = (word_t) ((short) reg16_get (REG_IP) + var->val.s);
		reg16_set (REG_IP, ip);
		}

	return 0;
	}


// Table of operation handlers

typedef int (* op_hand_t) (op_desc_t * op_desc);

struct op_id_hand_s
	{
	word_t id;
	op_hand_t hand;
	};

typedef struct op_id_hand_s op_id_hand_t;


static op_id_hand_t _id_hand_tab [] = {
	{ OP_MOV, op_move_load },
	{ OP_LEA, op_move_load },
	{ OP_LDS, op_move_load },

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

	{ OP_PUSH,  op_push  },
	{ OP_POP,   op_pop   },
	{ OP_PUSHF, op_pushf },
	{ OP_POPF,  op_popf  },
	{ OP_PUSHA, op_pusha },
	{ OP_POPA,  op_popa  },

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

	{ OP_REPNZ, op_repeat },
	{ OP_REPZ,  op_repeat },

	{ OP_MOVS, op_string },
	{ OP_CMPS, op_string },
	{ OP_STOS, op_string },
	{ OP_LODS, op_string },
	{ OP_SCAS, op_string },

/*
#define OP_AAA   0x0D00
#define OP_AAD   0x0D01
#define OP_AAM   0x0D02
#define OP_AAS   0x0D03
*/
	{ OP_CBW, op_convert },
	{ OP_CWD, op_convert },

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

	{ OP_LOOP,   op_loop },
	{ OP_LOOPNZ, op_loop },
	{ OP_LOOPZ,  op_loop },

	{ OP_SEG, op_seg },

	{ OP_NOP, op_nop },

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
*/

	{ 0, NULL }
	};


// Execute operation

static word_t _last_id = 0;
static op_hand_t _last_hand = NULL;

// Iteration counter to measure execution optimization
int exec_iter_count = 0;

int op_exec (op_desc_t * op_desc)
	{
	int err = -1;

	word_t id1 = OP_ID;

	while (1)
		{
		op_hand_t hand1 = NULL;

		// Optimization for repeated instruction

		if (id1 == _last_id && _last_hand)
			{
			hand1 = _last_hand;
			}
		else
			{
			// Table lookup

			op_id_hand_t * desc = _id_hand_tab;

			while (1)
				{
				word_t id2 = desc->id;
				op_hand_t hand2 = desc->hand;
				if (!id2 && !hand2)
					{
					printf ("fatal: no handler for op %hxh\n", id1);
					assert (0);
					break;
					}

				if (id2 == id1)
					{
					_last_id = id2;
					_last_hand = hand2;

					hand1 = hand2;
					break;
					}

				desc++;

				exec_iter_count++;
				}
			}

		if (hand1)
			{
			err = (*hand1) (op_desc);
			if (err) break;

			// REP and SEG prefix states

			if (_rep_stat == 2 && _seg_stat != 1)
				{
				assert (0);  // orphan REP prefix
				rep_reset ();
				}

			if (_seg_stat == 2 && _rep_stat != 1)
				{
				assert (0);  // orphan SEG prefix
				seg_reset ();
				}

			if (_rep_stat == 1) _rep_stat = 2;
			if (_seg_stat == 1) _seg_stat = 2;

			err = 0;
			}

		break;
		}

	return err;
	}
