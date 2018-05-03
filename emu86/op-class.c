// LIB86 - 80x86 library
// Operation classes

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "op-id.h"
#include "op-id-name.h"

#include "op-class.h"


// Opcode helpers

byte_t * op_code_base;
word_t op_code_seg;
word_t op_code_off;

byte_t op_code_null = 0;

char op_code_str [3 * OPCODE_MAX + 2];
byte_t op_code_pos;


static byte_t fetch_byte ()
	{
	byte_t b = *(op_code_base + (op_code_seg << 4) + op_code_off++);

	sprintf (op_code_str + op_code_pos, "%.2X ", b);
	op_code_pos += 3;

	return b;
	}

static word_t fetch_word ()
	{
	word_t w = (word_t) fetch_byte ();
	w |= (word_t) fetch_byte () << 8;
	return w;
	}


// Register class

static char * reg8_names  [] = { "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH" };
static char * reg16_names [] = { "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI" };
static char * seg_names   [] = { "ES", "CS", "SS", "DS" };

static void print_reg (byte_t type, byte_t num)
	{
	switch (type)
		{
		case RT_REG8:
			print_string (reg8_names [num]);
			break;

		case RT_REG16:
			print_string (reg16_names [num]);
			break;

		case RT_SEG:
			print_string (seg_names [num]);
			break;
		}
	}


// Address class

typedef byte_t rm_flags_t;

static rm_flags_t _rm_flags [] = {
	AF_BX | AF_SI,
	AF_BX | AF_DI,
	AF_BP | AF_SI,
	AF_BP | AF_DI,
	AF_SI,
	AF_DI,
	AF_BP,
	AF_BX
	};

static void scan_mod_rm (byte_t w, byte_t mod, byte_t rm, op_var_t * var)
	{
	var->w = w;

	if (mod == 3)
		{
		var->type = VT_REG;
		var->val.r = rm;
		}
	else
		{
		var->type = VT_MEM;

		short s = 0;
		byte_t flags = _rm_flags [rm];

		switch (mod)
			{
			case 0:  // no displacement
				if (rm == 6)  // special case [BP] -> [disp16]
					{
					s = (short) fetch_word ();
					flags = AF_DISP;
					}

				break;

			case 1:  // signed byte displacement
				s = (short) (char) fetch_byte ();
				flags |= AF_DISP;
				break;

			case 2:  // signed word displacement
				s = (short) fetch_word ();
				flags |= AF_DISP;
				break;

			}

		var->flags = flags;
		var->val.s = s;
		}
	}


static void print_mem (byte_t flags, short rel)
	{
	byte_t reg;

	reg = 0;
	putchar ('[');

	if (flags & AF_BX)
		{
		print_string ("BX");
		reg = 1;
		}

	if (flags & AF_BP)
		{
		print_string ("BP");
		reg = 1;
		}

	if (flags & AF_SI)
		{
		if (reg) putchar ('+');
		print_string ("SI");
		reg = 1;
		}

	if (flags & AF_DI)
		{
		if (reg) putchar ('+');
		print_string ("DI");
		reg = 1;
		}

	if (flags & AF_DISP)
		{
		if (reg)
			{
			print_rel (1, rel);  // with prefix
			}
		else
			{
			printf ("%hXh", (word_t) rel);
			}
		}

	putchar (']');
	}


// Variable class

static void print_var (op_var_t * var)
	{
	switch (var->type)
		{
		case VT_IMM:
			if (var->s)
				{
				print_rel (0, var->val.s);
				break;
				}

			if (var->w)
				{
				printf ("%.4Xh", var->val.w);
				break;
				}

			printf ("%.2Xh", var->val.b);
			break;

		case VT_REG:
			print_reg (var->w ? RT_REG16 : RT_REG8, var->val.r);
			break;

		case VT_SEG:
			print_reg (RT_SEG, var->val.r);
			break;

		case VT_MEM:
			print_mem (var->flags, var->val.s);
			break;

		case VT_LOC:
			if (var->far)
				{
				// Far address is absolute

				printf ("%.4Xh",var->seg);
				putchar (':');
				printf ("%.4Xh",var->val.w);
				break;
				}

			// Near address is relative

			printf ("%.4Xh", (word_t) ((short) op_code_off + var->val.s));
			break;

		}
	}


// Operation classes

void print_op (op_desc_t * op_desc)
	{
	char *name = op_id_to_name (OP_ID);
	if (!name) name = "???";
	print_column (name, OPNAME_MAX + 2);

	byte_t count = op_desc->var_count;

	// Special case for string operations

	if (!count && (OP_ID >= OP_STRING0) && (OP_ID < OP_STRING0 + 8))
		{
		printf (op_desc->w2 ? "WORD" : "BYTE");
		}

	// Common cases

	if (count >= 1)
		{
		print_var (&(op_desc->var_to));
		}

	if (count >= 2)
		{
		putchar (',');
		print_var (&(op_desc->var_from));
		}
	}


static int class_imm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;

	op_var_t * var_imm = &(op->var_to);

	var_imm->type  = VT_IMM;
	var_imm->s = 0;

	if (flags & CF_1)
		{
		var_imm->w = 0;
		var_imm->val.b = fetch_byte ();
		}

	if (flags & CF_2)
		{
		var_imm->w = 1;
		var_imm->val.w = fetch_word ();
		}

	return 0;
	}


static int class_off (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;

	op_var_t * var = &(op->var_to);

	var->type = VT_LOC;
	var->far = 0;

	if (flags & CF_1)
		{
		var->val.s = (short) (char) fetch_byte ();
		}

	if (flags & CF_2)
		{
		var->val.s = (short) fetch_word ();
		}

	return 0;
	}


static int class_off_seg (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;

	op_var_t * var = &(op->var_to);

	var->type = VT_LOC;
	var->far = 1;

	var->val.w = fetch_word ();
	var->seg = fetch_word ();

	return 0;
	}


static int class_in_out (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;

	op_var_t * var_acc;
	op_var_t * var_port;

	if (op->d)
		{
		var_port = &(op->var_to);
		var_acc = &(op->var_from);
		}

	else
		{
		var_acc = &(op->var_to);
		var_port = &(op->var_from);
		}

	var_acc->type = VT_REG;
	var_acc->w = op->w2;
	var_acc->val.r = 0;  // AX or AL

	if (op->v1)
		{
		var_port->type = VT_REG;
		var_port->w = 1;
		var_port->val.r = 2;  // DX
		}

	else
		{
		var_port->type = VT_IMM;
		var_port->w = 1;
		var_port->s = 0;
		var_port->val.w = (word_t) fetch_byte ();
		}

	return 0;
	}


static int class_reg (byte_t flags, op_desc_t * op)
	{
	op_var_t * var_reg;

	if (flags & CF_A)  // implicit accumulator
		{
		op->var_count = 2;

		op_var_t * var_acc = &(op->var_to);

		var_acc->type = VT_REG;
		var_acc->w = 1;
		var_acc->val.r = 0;  // AX

		var_reg = &(op->var_from);
		}

	else
		{
		op->var_count = 1;

		var_reg = &(op->var_to);
		}

	var_reg->type = VT_REG;
	var_reg->w = 1;
	var_reg->val.r = op->reg1;

	return 0;
	}


static int class_seg (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;

	op_var_t * var_seg = &(op->var_to);

	var_seg->type = VT_SEG;
	var_seg->w = 1;
	var_seg->val.r = op->seg1;

	return 0;
	}


static int class_w_imm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;

	op_var_t * var_acc = &(op->var_to);
	op_var_t * var_imm = &(op->var_from);

	var_acc->type = VT_REG;
	var_acc->w = op->w2;
	var_acc->val.r = 0;  // AX or AL

	var_imm->type  = VT_IMM;
	var_imm->w = op->w2;
	var_imm->s = 0;

	if (op->w2)
		{
		var_imm->val.w = fetch_word ();
		}
	else
		{
		var_imm->val.b = fetch_byte ();
		}

	return 0;
	}


static int class_d_w_addr (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;

	op_var_t * var_acc;
	op_var_t * var_addr;

	if (!op->d)  // special case for accumulator
		{
		var_acc  = &(op->var_to);
		var_addr = &(op->var_from);
		}
	else
		{
		var_addr = &(op->var_to);
		var_acc  = &(op->var_from);
		}

	var_acc->type = VT_REG;
	var_acc->val.r = 0;  // AX or AL
	var_acc->w = op->w2;

	var_addr->type  = VT_MEM;
	var_addr->w = op->w2;
	var_addr->flags |= AF_DISP;
	var_addr->val.w = fetch_word ();

	return 0;
	}


static int class_w_reg_imm (op_desc_t * op)
	{
	op->var_count = 2;

	op_var_t * var_reg = &(op->var_to);
	op_var_t * var_imm = &(op->var_from);

	var_reg->type = VT_REG;
	var_reg->val.r = op->reg1;
	var_reg->w = op->w1;

	var_imm->type = VT_IMM;
	var_imm->w = op->w1;
	var_imm->s = 0;

	if (op->w1)
		{
		var_imm->val.w = fetch_word ();
		}
	else
		{
		var_imm->val.b = fetch_byte ();
		}

	return 0;
	}


// TODO: common mod_rm pattern

static int class_mod_rm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;

	op_var_t * var = &(op->var_to);
	var->w = 1;

	if (flags & CF_F) var->far = 1;

	scan_mod_rm (1, op->mod, op->rm, var);

	return 0;
	}


static int class_w_mod_rm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;

	op_var_t * var_rm = &(op->var_to);

	scan_mod_rm (op->w2, op->mod, op->rm, var_rm);

	if (flags & CF_V)  // variable count
		{
		op->var_count = 2;

		op_var_t * var_num = &(op->var_from);

		var_num->w = 0;

		if (op->v2)
			{
			var_num->type = VT_REG;
			var_num->val.r = 1;  // CL
			}
		else
			{
			var_num->type  = VT_IMM;
			var_num->s = 0;
			var_num->val.b = 1;
			}
		}

	return 0;
	}


static int class_w_mod_rm_imm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;

	op_var_t * var_rm = &(op->var_to);
	op_var_t * var_imm = &(op->var_from);

	scan_mod_rm (op->w2, op->mod, op->rm, var_rm);

	// Immediate value follows the MOD-RM displacement

	var_imm->type  = VT_IMM;
	var_imm->w = op->w2;

	if (op->w2)
		{
		if ((flags & CF_S) && op->s)
			{
			var_imm->s = 1;
			var_imm->val.s = (short) (char) fetch_byte ();
			}
		else
			{
			var_imm->s = 0;
			var_imm->val.w = fetch_word ();
			}
		}
	else
		{
		var_imm->val.b = fetch_byte ();
		}

	return 0;
	}


static int class_w_mod_rm_count (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;

	op_var_t * var_rm = &(op->var_to);
	op_var_t * var_imm = &(op->var_from);

	scan_mod_rm (op->w2, op->mod, op->rm, var_rm);

	// Immediate value follows the MOD-RM displacement

	var_imm->type  = VT_IMM;
	var_imm->w = 0;
	var_imm->s = 0;
	var_imm->val.b = fetch_byte ();

	return 0;
	}


static int class_w_mod_reg_rm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;

	op_var_t * var_reg;
	op_var_t * var_rm;

	byte_t d = (flags & CF_D) ? op->d : 1;  // default when no direction

	if (d)
		{
		var_reg = &(op->var_to);
		var_rm  = &(op->var_from);
		}
	else
		{
		var_rm  = &(op->var_to);
		var_reg = &(op->var_from);
		}

	var_reg->type = VT_REG;
	var_reg->w = op->w2;
	var_reg->val.r = op->reg2;

	scan_mod_rm (op->w2, op->mod, op->rm, var_rm);

	return 0;
	}


static int class_mod_reg_rm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;

	op_var_t * var_reg = &(op->var_to);
	op_var_t * var_rm  = &(op->var_from);

	var_reg->type = VT_REG;
	var_reg->w = 1;
	var_reg->val.r = op->reg2;

	if (flags & CF_F) var_rm->far = 1;

	scan_mod_rm (1, op->mod, op->rm, var_rm);

	return 0;
	}


static int class_mod_seg_rm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;

	op_var_t * var_seg;
	op_var_t * var_rm;

	byte_t d = (flags & CF_D) ? op->d : 1;  // default when no direction

	if (d)
		{
		var_seg = &(op->var_to);
		var_rm  = &(op->var_from);
		}
	else
		{
		var_rm  = &(op->var_to);
		var_seg = &(op->var_from);
		}

	var_seg->type = VT_SEG;
	var_seg->w = 1;
	var_seg->val.r = op->seg2;

	scan_mod_rm (1, op->mod, op->rm, var_rm);

	return 0;
	}


// Fetch opcodes and decode common bit fields

static byte_t fetch_code_1 (op_desc_t * op_desc)
	{
	byte_t code = fetch_byte ();

	// Common bit fields

	op_desc->seg1 = (code & 0x18) >> 3;
	op_desc->v1   = (code & 0x08) >> 3;
	op_desc->w1   = (code & 0x08) >> 3;
	op_desc->reg1 =  code & 0x07;
	op_desc->d    = (code & 0x02) >> 1;
	op_desc->s    = (code & 0x02) >> 1;
	op_desc->v2   = (code & 0x02) >> 1;
	op_desc->w2   =  code & 0x01;

	return code;
	}

static byte_t fetch_code_2 (op_desc_t * op_desc)
	{
	byte_t code = fetch_byte ();

	// Common bit fields

	op_desc->mod  = (code & 0xC0) >> 6;
	op_desc->reg2 = (code & 0x38) >> 3;
	op_desc->seg2 = (code & 0x18) >> 3;
	op_desc->rm   =  code & 0x07;

	return code;
	}


static int class_1_00h (byte_t code, op_desc_t * op_desc)
	{
	int err = -1;

	while (1)
		{
		if (!(code & 0x04))
			{
			OP_ID = OP_CALC2 + ((code & 0x38) >> 3);
			code = fetch_code_2 (op_desc);
			err = class_w_mod_reg_rm (CF_D, op_desc);

			// Suspicious null opcodes (ADD [BX+SI],AL)

			op_code_null = (OP_ID == OP_ADD && !code) ? 1 : 0;
			break;
			}

		if (!(code & 0x02))
			{
			OP_ID = OP_CALC2 + ((code & 0x38) >> 3);
			err = class_w_imm (0, op_desc);
			break;
			}

		if (!(code & 0x20))
			{
			OP_ID = OP_STACK1 + (code & 0x01);  // TODO: POP CS allowed for 8086 / forbidden for 80186
			err = class_seg (0, op_desc);
			break;
			}

		if (!(code & 0x01))
			{
			OP_ID = OP_SEG;
			err = class_seg (0, op_desc);
			break;
			}

		OP_ID = OP_ADJUST1 + op_desc->seg1;
		err = 0;
		break;
		}

	return err;
	}


static int class_1_40h (byte_t code, op_desc_t * op_desc)
	{
	int err = -1;

	switch (code & 0x30)
		{
		case 0x00:
			OP_ID = OP_STEP1 + ((code & 0x08) >> 3);
			err = class_reg (0, op_desc);
			break;

		case 0x10:
			OP_ID = OP_STACK1 + ((code & 0x08) >> 3);
			err = class_reg (0, op_desc);
			break;

		case 0x20:
			if (code & 0x0E)
				{
				// Unknown opcodes for 8086
				// TODO: complete with 80186 opcodes
				err = -1;
				break;
				}

			OP_ID = OP_STACK2 + op_desc->w2;
			err = 0;
			break;

		case 0x30:
			OP_ID = OP_JUMP + (code & 0x0F);
			err = class_off (CF_1, op_desc);
			break;

		}

	return err;
	}


static int class_1_80h (byte_t code, op_desc_t * op_desc)
	{
	int err = -1;

	byte_t code_2 = 0;

	switch (code & 0x30)
		{
		case 0x00:
			code_2 = fetch_code_2 (op_desc);

			if (!(code & 0x04))
				{
				if (!(code & 0x08))
					{
					OP_ID = OP_CALC2 + op_desc->reg2;
					err = class_w_mod_rm_imm (CF_S, op_desc);  // TODO: S not compliant for all operations
					break;
					}

				OP_ID = OP_MOV;
				err = class_w_mod_reg_rm (CF_D, op_desc);
				break;
				}

			if (!(code & 0x08))
				{
				if (!(code & 0x02))
					{
					OP_ID = OP_TEST;
					err = class_w_mod_reg_rm (0, op_desc);
					break;
					}

				OP_ID = OP_XCHG;
				err = class_w_mod_reg_rm (0, op_desc);
				break;
				}

			if (!(code & 0x01))
				{
				OP_ID = OP_MOV;
				err = class_mod_seg_rm (CF_D, op_desc);
				break;
				}

			if (!(code & 0x02))
				{
				OP_ID = OP_LEA;
				err = class_mod_reg_rm (0, op_desc);
				break;
				}

			OP_ID = OP_POP;
			err = class_mod_rm (0, op_desc);  // TODO: class_w_mod_rm & w=1 ?
			break;

		case 0x10:
			if (!(code & 0x08))
				{
				if (code == 0x90)
					{
					OP_ID = OP_NOP;
					err = 0;
					break;
					}

				OP_ID = OP_XCHG;
				err = class_reg (CF_A, op_desc);
				break;
				}

			switch (op_desc->reg1)
				{
				case 0x00:
				case 0x01:
					OP_ID = OP_CONVERT1 + op_desc->w2;
					err = 0;
					break;

				case 0x02:
					OP_ID = OP_CALLF;
					err = class_off_seg (0, op_desc);
					break;

				case 0x03:
					OP_ID = OP_WAIT;
					err = 0;
					break;

				case 0x04:
				case 0x05:
					OP_ID = OP_STACK3 + op_desc->w2;
					err = 0;
					break;

				case 0x06:
				case 0x07:
					OP_ID = OP_FLAGS1 + op_desc->w2;
					err = 0;
					break;

				}

			break;

		case 0x20:
			if ((code & 0x0C) == 0x00)
				{
				OP_ID = OP_MOV;
				err = class_d_w_addr (0, op_desc);
				break;
				}

			if ((code & 0x0E) == 0x08)
				{
				OP_ID = OP_TEST;
				err = class_w_imm (0, op_desc);
				break;
				}

			OP_ID = OP_STRING0 + ((code & 0x0E) >> 1);
			err = 0;
			break;

		case 0x30:
			OP_ID = OP_MOV;
			err = class_w_reg_imm (op_desc);
			break;

		}

	return err;
	}


static int class_1_C0h (byte_t code, op_desc_t * op_desc)
	{
	int err = -1;

	byte_t code_2 = 0;

	switch (code & 0x30)
		{
		case 0x00:
			if (!(code & 0x08))
				{
				if (!(code & 0x04))
					{
					if (!(code & 0x02))
						{
						code_2 = fetch_code_2 (op_desc);
						OP_ID = OP_BITS + op_desc->reg2;
						err = class_w_mod_rm_count (0, op_desc);
						break;
						}

					OP_ID = OP_RET;
					if (!(code & 0x01))
						{
						err = class_imm (CF_2, op_desc);
						break;
						}

					err = 0;
					break;
					}

				code_2 = fetch_code_2 (op_desc);

				if (!(code & 0x02))
					{
					OP_ID = OP_LOAD1 + op_desc->w2;
					err = class_mod_reg_rm (CF_F, op_desc);
					break;
					}

				OP_ID = OP_MOV;
				err = class_w_mod_rm_imm (0, op_desc);
				break;
				}

			if (!(code & 0x04))
				{
				if (!(code & 0x02))
					{
					// TODO: ENTER (C8h) and LEAVE (C9h)
					err = -1;
					break;
					}

				OP_ID = OP_RETF;

				if (!(code & 0x01))
					{
					err = class_imm (CF_2, op_desc);
					break;
					}

				err = 0;
				break;
				}

			OP_ID = OP_INTS + (code & 0x03);
			if ((code & 0x03) == 0x01)
				{
				err = class_imm (CF_1, op_desc);
				break;
				}

			err = 0;
			break;

		case 0x10:
			if (!(code & 0x08))
				{
				if (!(code & 0x04))
					{
					code_2 = fetch_code_2 (op_desc);
					OP_ID = OP_BITS + op_desc->reg2;
					err = class_w_mod_rm (CF_V, op_desc);
					break;
					}

				if (!(code & 0x02))
					{
					OP_ID = OP_ADJUST2 + op_desc->w2;
					err = 0;
					break;
					}

				OP_ID = OP_MISC1 + op_desc->w2;
				err = 0;
				break;
				}

			OP_ID = OP_ESC;  // TODO: extract (reg1) (reg2)
			code_2 = fetch_code_2 (op_desc);
			err = class_mod_rm (0, op_desc);
			break;

		case 0x20:
			if (!(code & 0x04))
				{
				if (!(code & 0x08))
					{
					OP_ID = OP_LOOP1 + (code & 0x03);
					err = class_off (CF_1, op_desc);
					break;
					}

				if (!(code & 0x02))
					{
					OP_ID = OP_FLOW1 + op_desc->w2;
					err = class_off (CF_2, op_desc);
					break;
					}

				if (!(code & 0x01))
					{
					OP_ID = OP_JMPF;
					err = class_off_seg (0, op_desc);
					break;
					}

				OP_ID = OP_JMP;  // TODO: use far flag
				err = class_off (CF_1, op_desc);
				break;
				}

			OP_ID = OP_PORT1 + op_desc->d;
			err = class_in_out (0, op_desc);
			break;

		case 0x30:
			if (!(code & 0x08))
				{
				if ((code & 0x06) < 0x06)
					{
					if (code == 0xF1)
						{
						// F1h reserved
						err = -1;
						break;
						}

					OP_ID = OP_PREFIX + op_desc->reg1;
					err = 0;
					break;
					}

				code_2 = fetch_code_2 (op_desc);
				if (!(code_2 & 0x30))
					{
					OP_ID = OP_TEST;
					err = class_w_mod_rm_imm (0, op_desc);
					break;
					}

				OP_ID = OP_CALC1 + op_desc->reg2;
				err = class_w_mod_rm (0, op_desc);
				break;
				}

			if ((code & 0x06) < 0x06)
				{
				OP_ID = OP_FLAGS2 + op_desc->reg1;
				err = 0;
				break;
				}

			code_2 = fetch_code_2 (op_desc);
			switch (op_desc->reg2)
				{
				case 0x00:
				case 0x01:
					OP_ID = OP_INC + (op_desc->reg2 & 0x01);
					err = class_w_mod_rm (0, op_desc);
					break;

				// TODO: add far flags to operation ?

				case 0x02:
					OP_ID = OP_CALL;
					err = class_mod_rm (0, op_desc);
					break;

				case 0x03:
					OP_ID = OP_CALLF;
					err = class_mod_rm (CF_F, op_desc);
					break;

				case 0x04:
					OP_ID = OP_JMP;
					err = class_mod_rm (0, op_desc);
					break;

				case 0x05:
					OP_ID = OP_JMPF;
					err = class_mod_rm (CF_F, op_desc);
					break;

				case 0x06:
					OP_ID = OP_PUSH;
					err = class_mod_rm (0, op_desc);  // TODO: class_w_mod_rm & w=1 ?
					break;

				case 0x07:
					// No pop instruction here
					err = -1;
					break;

				}

			break;

		}

	return err;
	}


int op_decode (op_desc_t * op_desc)
	{
	int err = -1;

	while (1)
		{
		memset (op_code_str, 0, sizeof op_code_str);
		op_code_pos = 0;

		byte_t code = fetch_code_1 (op_desc);

		// Optimized decoding based on bit masks

		switch (code & 0xC0)
			{
			case 0x00:
				err = class_1_00h (code, op_desc);
				break;

			case 0x40:
				err = class_1_40h (code, op_desc);
				break;

			case 0x80:
				err = class_1_80h (code, op_desc);
				break;

			case 0xC0:
				err = class_1_C0h (code, op_desc);
				break;

			}

		break;
		}

	return err;
	}
