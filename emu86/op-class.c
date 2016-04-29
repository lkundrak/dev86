// DIS86 - 80x86 disassembler
// Operation classes

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "op-class.h"
#include "op-exec.h"
#include "op-id-name.h"


// Opcode helpers

byte_t * op_code_base;
word_t op_code_cs;
word_t op_code_ip;

char op_code_str [3 * OPCODE_MAX];
byte_t op_code_pos;

static byte_t fetch_code ()
	{
	byte_t code = *(op_code_base + (op_code_cs << 4) + op_code_ip++);
	sprintf (op_code_str + op_code_pos, "%.2X ", code);
	op_code_pos += 3;
	return code;
	}


// Register classes

static char * reg8_names  [] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
static char * reg16_names [] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
static char * seg_names   [] = { "es", "cs", "ss", "ds" };

static print_reg (byte_t type, byte_t num)
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


// Address classes

static word_t scan_abs (byte_t size)
	{
	assert (size > 0);
	word_t abs = fetch_code ();
	if (size > 1)
		{
		abs = abs + (fetch_code () << 8);
		}

	assert (size < 3);
	return abs;
	}

/*
	switch (size)
		{
		case 1:  // byte
			var->size = VS_BYTE;
			var->val.abs = *((byte_t *) ip);
			break;

		case 2:  // word
			var->size = VS_WORD;
			var->val.abs = *((word_t *) ip);
			break;

		default:
			assert (0);  // missing size flag in class descriptor

		}
*/

static short scan_rel (byte_t size)
	{
	assert (size > 0);
	short rel = (short) fetch_code ();
	if (size > 1)
		{
		rel = rel + ((short) fetch_code () * 256);
		}

	assert (size < 3);
	return rel;
	}

/*
	switch (size)
		{
		case 1:  // byte
			var->size = VS_BYTE;
			var->val.rel = *((char *) ip);
			break;

		case 2:  // word
			var->size = VS_WORD;
			var->val.rel = *((short *) ip);
			break;

		default:
			assert (0);  // missing size flag in class descriptor

		}
*/

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
	var->size = w ? VS_WORD : VS_BYTE;

	if (mod == 3)
		{
		var->type = VT_REG;
		var->val.reg = rm;
		}
	else
		{
		var->type = VT_INDEX;

		short rel = 0;
		byte_t flags = _rm_flags [rm];

		switch (mod)
			{
			case 0:  // no displacement
				if (rm == 6)  // special case [BP] -> [disp16]
					{
					rel = scan_rel (2);
					flags = AF_DISP;
					}

				break;

			case 1:  // signed byte displacement
				rel = scan_rel (1);
				flags |= AF_DISP;
				break;

			case 2:  // signed word displacement
				rel = scan_rel (2);
				flags |= AF_DISP;
				break;

			}

		var->flags = flags;
		var->val.rel = rel;
		}
	}

static void print_addr (byte_t flags, short rel)
	{
	byte_t reg;

	reg = 0;
	putchar ('[');

	if (flags & AF_BX)
		{
		print_string ("bx");
		reg = 1;
		}

	if (flags & AF_BP)
		{
		print_string ("bp");
		reg = 1;
		}

	if (flags & AF_SI)
		{
		if (reg) putchar ('+');
		print_string ("si");
		reg = 1;
		}

	if (flags & AF_DI)
		{
		if (reg) putchar ('+');
		print_string ("di");
		reg = 1;
		}

	if (flags & AF_DISP)
		{
		if (reg & (rel >= 0)) putchar ('+');
		printf ("%i", rel);
		}

	putchar (']');
	}


// Variable classe

static void print_var (op_var_t * var)
	{
	byte_t rt;

	switch (var->type)
		{
		case VT_IMM:
			if (var->size == VS_BYTE) printf ("%.2X",var->val.abs);
			if (var->size == VS_WORD) printf ("%.4X",var->val.abs);
			break;

		case VT_REG:
			rt = (var->size == VS_BYTE) ? RT_REG8 : RT_REG16;
			print_reg (rt, var->val.reg);
			break;

		case VT_SEG:
			print_reg (RT_SEG, var->val.reg);
			break;

		case VT_INDEX:
			print_addr (var->flags, var->val.rel);
			break;

		case VT_NEAR:
			printf ("%.4X", op_code_ip + var->val.rel);
			break;

		case VT_FAR:
			printf ("%.4X",var->seg);
			putchar (':');
			printf ("%.4X",var->val.abs);
			break;

		}
	}


// Operation classes

void op_print (op_desc_t * op_desc)
	{
	char *name = op_id_to_name (op_desc->op_id);
	if (!name) name = "???";
	print_column (name, OPNAME_MAX + 2);

	byte_t count = op_desc->var_count;
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


static int class_void (byte_t flags, op_desc_t * op)
	{
	byte_t size = 0;

	if (flags & CF_1) size = 1;
	if (flags & CF_2)
		{
		// TODO: dummy opcode 2 for AAM & AAD ?
		fetch_code ();
		size = 2;
		}

	assert (size);
	return 0;
	}


static int class_imm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;
	op_var_t * var_imm = &(op->var_to);
	var_imm->type  = VT_IMM;

	byte_t size = 0;
	if (flags & CF_1)
		{
		size = 1;
		var_imm->size = VS_BYTE;
		}

	if (flags & CF_2)
		{
		size = 2;
		var_imm->size = VS_WORD;
		}

	assert (size);
	var_imm->val.abs = scan_abs (size);
	return 0;
	}


static int class_off_seg (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;
	op_var_t * var_far = &(op->var_to);
	var_far->type  = VT_FAR;

	var_far->val.abs = scan_abs (2);
	var_far->seg = scan_abs (2);

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

	var_acc->type  = VT_REG;
	var_acc->size  = op->w2 ? VS_WORD : VS_BYTE;
	var_acc->val.reg = 0;  // AX or AL

	if (op->v1)
		{
		var_port->type  = VT_REG;
		var_port->size = VS_WORD;
		var_port->val.reg = 2;  // DX
		}
	else
		{
		var_port->type = VT_IMM;
		var_port->size = VS_BYTE;
		var_port->val.abs = scan_abs (1);
		}

	return 0;
	}


static int class_dist (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;
	op_var_t * var_dist = &(op->var_to);

	var_dist->type  = VT_NEAR;

	byte_t size = 0;
	if (flags & CF_1) size = 1;
	if (flags & CF_2) size = 2;
	assert (size);

	var_dist->val.rel = scan_rel (size);
	return 0;
	}


static int class_reg (byte_t flags, op_desc_t * op)
	{
	op_var_t * var_reg;

	if (flags & CF_A)  // implicit accumulator
		{
		op->var_count = 2;

		op_var_t * var_acc = &(op->var_to);
		var_acc->type  = VT_REG;
		var_acc->size  = VS_WORD;
		var_acc->val.reg = 0;  // AX

		var_reg = &(op->var_from);
		}
	else
		{
		op->var_count = 1;
		var_reg = &(op->var_to);
		}

	var_reg->type  = VT_REG;
	var_reg->size  = VS_WORD;
	var_reg->val.reg = op->reg1;

	return 0;
	}


static int class_seg (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;
	op_var_t * var_seg = &(op->var_to);

	var_seg->type  = VT_SEG;
	var_seg->val.reg = op->seg1;

	return 0;
	}


static int class_w_imm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;

	op_var_t * var_acc = &(op->var_to);
	op_var_t * var_imm = &(op->var_from);

	var_acc->type  = VT_REG;
	var_acc->size  = op->w2 ? VS_WORD : VS_BYTE;
	var_acc->val.reg = 0;  // AX or AL

	var_imm->type  = VT_IMM;

	byte_t size = op->w2 ? 2 : 1;
	var_imm->val.abs = scan_abs (size);

	return 0;
	}


static int class_w_addr (byte_t flags, op_desc_t * op)
	{
	byte_t d = 0;  // default when no direction
	if (flags & CF_D) d = op->d;

	op->var_count = 2;
	op_var_t * var_acc;
	op_var_t * var_addr;

	if (d)
		{
		var_acc  = &(op->var_to);
		var_addr = &(op->var_from);
		}
	else
		{
		var_addr = &(op->var_to);
		var_acc  = &(op->var_from);
		}

	var_acc->type  = VT_REG;
	var_acc->size  = op->w2 ? VS_WORD : VS_BYTE;
	var_acc->val.reg = 0;  // AX or AL

	var_addr->type  = VT_NEAR;
	var_addr->flags = 0;

	byte_t size = op->w2 ? 2 : 1;
	var_addr->val.rel = scan_abs (size);

	return 0;
	}


static int class_w_reg_imm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;
	op_var_t * var_reg = &(op->var_to);
	op_var_t * var_imm = &(op->var_from);

	var_reg->type  = VT_REG;
	var_reg->size  = op->w1 ? VS_WORD : VS_BYTE;
	var_reg->val.reg = op->reg1;

	var_imm->type   = VT_IMM;

	byte_t size = op->w1 ? 2 : 1;
	var_imm->val.abs = scan_abs (size);

	return 0;
	}


// TODO: common mod_rm pattern

static int class_mod_rm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;
	op_var_t * var_rm = &(op->var_to);
	scan_mod_rm (1, op->mod, op->rm, var_rm);
	return 0;
	}


static int class_w_mod_rm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;
	op_var_t * var_rm = &(op->var_to);
	scan_mod_rm (op->w2, op->mod, op->rm, var_rm);

	if (flags & CF_V)
		{
		op->var_count = 2;
		op_var_t * var_num = &(op->var_from);

		if (op->v2)
			{
			var_num->type = VT_REG;
			var_num->size = VS_BYTE;
			var_num->val.reg = 1; // CL
			}
		else
			{
			var_num->type = VT_IMM;
			var_num->size = VS_BYTE;
			var_num->val.abs = 1;
			}
		}

	return 0;
	}


static int class_w_mod_rm_imm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;
	op_var_t * var_rm = &(op->var_to);
	op_var_t * var_imm = &(op->var_from);

	var_imm->type  = VT_IMM;
	var_imm->size  = op->w2 ? VS_WORD : VS_BYTE;

	if ((flags & CF_S) && op->s)
		{
		var_imm->val.rel = scan_rel (1);
		}
	else
		{
		byte_t size = op->w2 ? 2 : 1;
		var_imm->val.abs = scan_abs (size);
		}

	scan_mod_rm (op->w2, op->mod, op->rm, var_rm);
	return 0;
	}


static int class_w_mod_reg_rm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;
	op_var_t * var_reg;
	op_var_t * var_rm;

	byte_t d = 1;  // default when no direction
	if (flags & CF_D) d = op->d;

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
	var_reg->size = op->w2 ? VS_WORD : VS_BYTE;
	var_reg->val.reg = op->reg2;

	scan_mod_rm (op->w2, op->mod, op->rm, var_rm);
	return 0;
	}


static int class_mod_reg_rm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;
	op_var_t * var_reg = &(op->var_to);
	op_var_t * var_rm  = &(op->var_from);

	var_reg->type  = VT_REG;
	var_reg->size  = VS_WORD;
	var_reg->val.reg = op->reg2;

	scan_mod_rm (1, op->mod, op->rm, var_rm);
	return 0;
	}


static int class_mod_seg_rm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;
	op_var_t * var_seg;
	op_var_t * var_rm;

	byte_t d = 1;  // default when no direction
	if (flags & CF_D) op->d;
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

	var_seg->type  = VT_SEG;
	var_seg->val.reg = op->seg2;

	scan_mod_rm (1, op->mod, op->rm, var_rm);
	return 0;
	}


class_desc_t * class_find (class_desc_t * tab, byte_t op)
	{
	class_desc_t * desc = tab;

	while (1)
		{
		if (!desc->class_hand)
			{
			desc = NULL; // unknown opcode
			break;
			}

		if ((op & desc->mask) == desc->code) break;
		desc++;
		}

	return desc;
	}


static int class_hand_2 (byte_t flags, op_desc_t * op_desc)
	{
	int err = 0;

	while (1)
		{
		class_desc_t * tab = (class_desc_t *) (op_desc->op_hand);
		assert (tab);

		byte_t code = fetch_code ();

		op_desc->mod  = (code & 0xC0) >> 6;
		op_desc->reg2 = (code & 0x38) >> 3;
		op_desc->seg2 = (code & 0x18) >> 3;
		op_desc->rm   =  code & 0x07;

		class_desc_t * class_desc = class_find (tab, code);
		if (!class_desc)
			{
			err = -1;  // unknown opcode
			break;
			}

		op_desc->name     = class_desc->name;
		op_desc->op_hand  = class_desc->op_hand;
		op_desc->op_id    = class_desc->op_id;

		class_hand_t hand = class_desc->class_hand;
		assert (hand);
		err = (*hand) (class_desc->class_flags, op_desc);
		break;
		}

	return err;
	}


static class_desc_t class_2_80h [] = {
	{ 0x38, 0x00, class_w_mod_rm_imm, CF_S,   NULL,     op_calc_2,     OP_ADD   },
	{ 0x38, 0x08, class_w_mod_rm_imm, 0,      NULL,     op_calc_2,     OP_OR    },
	{ 0x38, 0x10, class_w_mod_rm_imm, CF_S,   NULL,     op_calc_2,     OP_ADC   },
	{ 0x38, 0x18, class_w_mod_rm_imm, CF_S,   NULL,     op_calc_2,     OP_SBB   },
	{ 0x38, 0x20, class_w_mod_rm_imm, 0,      NULL,     op_calc_2,     OP_AND   },
	{ 0x38, 0x28, class_w_mod_rm_imm, CF_S,   NULL,     op_calc_2,     OP_SUB   },
	{ 0x38, 0x30, class_w_mod_rm_imm, 0,      NULL,     op_calc_2,     OP_XOR   },
	{ 0x38, 0x38, class_w_mod_rm_imm, CF_S,   NULL,     op_calc_2,     OP_CMP   },
	{ 0x00, 0x00, NULL,               0,      NULL,     NULL,          0        }
	};

static class_desc_t class_2_88h [] = {
	{ 0x00, 0x00, class_w_mod_reg_rm, CF_D,   NULL,     op_move_load,  OP_MOV   },
	{ 0x00, 0x00, NULL,               0,      NULL,     NULL,          0        }
	};

static class_desc_t class_2_8Dh [] = {
	{ 0x00, 0x00, class_mod_reg_rm,   0,      NULL,     op_move_load,  OP_LEA   },
	{ 0x00, 0x00, NULL,               0,      NULL,     NULL,          0        }
	};

static class_desc_t class_2_D0h [] = {
	{ 0x38, 0x00, class_w_mod_rm,     CF_V,   NULL,     op_shift_rot,  OP_ROL   },
	{ 0x38, 0x08, class_w_mod_rm,     CF_V,   NULL,     op_shift_rot,  OP_ROR   },
	{ 0x38, 0x10, class_w_mod_rm,     CF_V,   NULL,     op_shift_rot,  OP_RCL   },
	{ 0x38, 0x18, class_w_mod_rm,     CF_V,   NULL,     op_shift_rot,  OP_RCR   },
	{ 0x38, 0x20, class_w_mod_rm,     CF_V,   NULL,     op_shift_rot,  OP_SHL   },
	{ 0x38, 0x28, class_w_mod_rm,     CF_V,   NULL,     op_shift_rot,  OP_SHR   },
	{ 0x38, 0x30, class_w_mod_rm,     CF_V,   NULL,     op_shift_rot,  OP_SAL   },
	{ 0x38, 0x38, class_w_mod_rm,     CF_V,   NULL,     op_shift_rot,  OP_SAR   },
	{ 0x00, 0x00, NULL,               0,      NULL,     NULL,          0        }
	};

static class_desc_t class_2_F6h [] = {
	{ 0x38, 0x00, class_w_mod_rm_imm, 0,      NULL,     op_calc_2,     OP_TEST  },
	{ 0x38, 0x10, class_w_mod_rm,     0,      NULL,     op_calc_1,     OP_NOT   },
	{ 0x38, 0x18, class_w_mod_rm,     0,      NULL,     op_calc_1,     OP_NEG   },
	{ 0x38, 0x20, class_w_mod_rm,     0,      NULL,     op_calc_1,     OP_MUL   },
	{ 0x38, 0x28, class_w_mod_rm,     0,      NULL,     op_calc_1,     OP_IMUL  },
	{ 0x38, 0x30, class_w_mod_rm,     0,      NULL,     op_calc_1,     OP_DIV   },
	{ 0x38, 0x38, class_w_mod_rm,     0,      NULL,     op_calc_1,     OP_IDIV  },
	{ 0x00, 0x00, NULL,               0,      NULL,     NULL,          0        }
	};

static class_desc_t class_2_FEh [] = {
	{ 0x38, 0x00, class_w_mod_rm,     0,      NULL,     op_inc_dec,    OP_INC   },
	{ 0x38, 0x08, class_w_mod_rm,     0,      NULL,     op_inc_dec,    OP_DEC   },
	{ 0x00, 0x00, NULL,               0,      NULL,     NULL,          0        }
	};

static class_desc_t class_2_FFh [] = {
	{ 0x38, 0x10, class_mod_rm,       0,      NULL,     op_jump_call,  OP_CALL  },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x38, 0x18, class_mod_rm,       0,      NULL,     op_jump_call,  OP_CALLF },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x38, 0x20, class_mod_rm,       0,      NULL,     op_jump_call,  OP_JMP   },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x38, 0x28, class_mod_rm,       0,      NULL,     op_jump_call,  OP_JMPF  },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x38, 0x30, class_mod_rm,       0,      NULL,     op_push,       OP_PUSH  },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x00, 0x00, NULL,               0,      NULL,     NULL,          0        }
	};

static class_desc_t class_1 [] = {

	{ 0xFC, 0x00, class_w_mod_reg_rm, CF_D,   NULL,     op_calc_2,     OP_ADD   },
	{ 0xFC, 0x08, class_w_mod_reg_rm, CF_D,   NULL,     op_calc_2,     OP_OR    },
	{ 0xFC, 0x10, class_w_mod_reg_rm, CF_D,   NULL,     op_calc_2,     OP_ADC   },
	{ 0xFC, 0x18, class_w_mod_reg_rm, CF_D,   NULL,     op_calc_2,     OP_SBB   },
	{ 0xFC, 0x20, class_w_mod_reg_rm, CF_D,   NULL,     op_calc_2,     OP_AND   },
	{ 0xFC, 0x28, class_w_mod_reg_rm, CF_D,   NULL,     op_calc_2,     OP_SUB   },
	{ 0xFC, 0x30, class_w_mod_reg_rm, CF_D,   NULL,     op_calc_2,     OP_XOR   },
	{ 0xFC, 0x38, class_w_mod_reg_rm, CF_D,   NULL,     op_calc_2,     OP_CMP   },

	{ 0xFE, 0x04, class_w_imm,        0,      NULL,     op_calc_2,     OP_ADD   },
	{ 0xFE, 0x0C, class_w_imm,        0,      NULL,     op_calc_2,     OP_OR    },
	{ 0xFE, 0x14, class_w_imm,        0,      NULL,     op_calc_2,     OP_ADC   },
	{ 0xFE, 0x1C, class_w_imm,        0,      NULL,     op_calc_2,     OP_SBB   },
	{ 0xFE, 0x24, class_w_imm,        0,      NULL,     op_calc_2,     OP_AND   },
	{ 0xFE, 0x2C, class_w_imm,        0,      NULL,     op_calc_2,     OP_SUB   },
	{ 0xFE, 0x34, class_w_imm,        0,      NULL,     op_calc_2,     OP_XOR   },
	{ 0xFE, 0x3C, class_w_imm,        0,      NULL,     op_calc_2,     OP_CMP   },

	{ 0xE7, 0x06, class_seg,          0,      NULL,     op_push,       OP_PUSH  },
	{ 0xE7, 0x07, class_seg,          0,      NULL,     op_pop,        OP_POP   },

	{ 0xE7, 0x26, class_seg,          0,      "SEG",    NULL,  0  },

	{ 0xFF, 0x27, class_void,         CF_1,   "DAA",    NULL,  0  },
	{ 0xFF, 0x2F, class_void,         CF_1,   "DAS",    NULL,  0  },
	{ 0xFF, 0x37, class_void,         CF_1,   "AAA",    NULL,  0  },
	{ 0xFF, 0x3F, class_void,         CF_1,   "AAS",    NULL,  0  },

	{ 0xF8, 0x40, class_reg,          0,      NULL,     op_inc_dec,    OP_INC   },
	{ 0xF8, 0x48, class_reg,          0,      NULL,     op_inc_dec,    OP_DEC   },

	{ 0xF8, 0x50, class_reg,          0,      NULL,     op_push,       OP_PUSH  },
	{ 0xF8, 0x58, class_reg,          0,      NULL,     op_pop,        OP_POP   },

	{ 0xFF, 0x70, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JO    },
	{ 0xFF, 0x71, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JNO   },
	{ 0xFF, 0x72, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JB    },
	{ 0xFF, 0x73, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JNB   },
	{ 0xFF, 0x74, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JZ    },
	{ 0xFF, 0x75, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JNZ   },
	{ 0xFF, 0x76, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JNA   },
	{ 0xFF, 0x77, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JA    },
	{ 0xFF, 0x78, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JS    },
	{ 0xFF, 0x79, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JNS   },
	{ 0xFF, 0x7A, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JP    },
	{ 0xFF, 0x7B, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JNP   },
	{ 0xFF, 0x7C, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JL    },
	{ 0xFF, 0x7D, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JNL   },
	{ 0xFF, 0x7E, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JNG   },
	{ 0xFF, 0x7F, class_dist,         CF_1,   NULL,     op_jump_cond,  OP_JG    },

	{ 0xFC, 0x80, class_hand_2,       0,      NULL,     class_2_80h,   0        },

	{ 0xFE, 0x84, class_w_mod_reg_rm, 0,      NULL,     op_calc_2,     OP_TEST  },
	{ 0xFE, 0x86, class_w_mod_reg_rm, 0,      NULL,     op_swap,       OP_XCHG  },

	{ 0xFC, 0x88, class_hand_2,       CF_D,   NULL,     class_2_88h,   0        },

	{ 0xFD, 0x8C, class_mod_seg_rm,   CF_D,   NULL,     op_move_load,  OP_MOV   },
	{ 0xFF, 0x8D, class_hand_2,       0,      NULL,     class_2_8Dh,   0        },
//	{ 0xFD, 0x8E, class_mod_seg_rm,   CF_D,   NULL,     op_move_load,  OP_MOV   },
	{ 0xFF, 0x8F, class_mod_rm,       0,      NULL,     op_pop,        OP_POP   },  // TODO: class_w_mod_rm & w=1 ?

	{ 0xF8, 0x90, class_reg,          CF_A,   NULL,     op_swap,       OP_XCHG  },

	{ 0xFF, 0x98, class_void,         CF_1,   "CBW",    NULL,  0  },
	{ 0xFF, 0x99, class_void,         CF_1,   "CWD",    NULL,  0  },
//	{ 0xFF, 0x9A, NULL,               0,      "???",    NULL,  0  },
	{ 0xFF, 0x9B, class_void,         CF_1,   "WAIT",   NULL,  0  },

	{ 0xFF, 0x9C, class_void,         CF_1,   NULL,     op_pushf,      OP_PUSHF },
	{ 0xFF, 0x9D, class_void,         CF_1,   NULL,     op_popf,       OP_POPF  },

	{ 0xFF, 0x9E, class_void,         CF_1,   "SAHF",   NULL,  0  },
	{ 0xFF, 0x9F, class_void,         CF_1,   "LAHF",   NULL,  0  },

	{ 0xFF, 0x9A, class_off_seg,      0,      NULL,     op_jump_call,  OP_CALLF },
	{ 0xFF, 0xEA, class_off_seg,      0,      NULL,     op_jump_call,  OP_JMPF  },

	{ 0xFC, 0xA0, class_w_addr,       CF_D,   NULL,     op_move_load,  OP_MOV   },

	{ 0xFE, 0xA8, class_w_imm,        0,      NULL,     op_calc_2,     OP_TEST  },

	{ 0xFE, 0xA4, class_void,         CF_1,   NULL,     op_string,     OP_MOVSB },
	{ 0xFE, 0xA5, class_void,         CF_1,   NULL,     op_string,     OP_MOVSW },
	{ 0xFE, 0xA6, class_void,         CF_1,   NULL,     op_string,     OP_CMPSB },
	{ 0xFE, 0xA7, class_void,         CF_1,   NULL,     op_string,     OP_CMPSW },
	{ 0xFE, 0xAA, class_void,         CF_1,   NULL,     op_string,     OP_STOSB },
	{ 0xFE, 0xAB, class_void,         CF_1,   NULL,     op_string,     OP_STOSW },
	{ 0xFE, 0xAC, class_void,         CF_1,   NULL,     op_string,     OP_LODSB },
	{ 0xFE, 0xAD, class_void,         CF_1,   NULL,     op_string,     OP_LODSW },
	{ 0xFE, 0xAE, class_void,         CF_1,   NULL,     op_string,     OP_SCASB },
	{ 0xFE, 0xAF, class_void,         CF_1,   NULL,     op_string,     OP_SCASW },

	{ 0xF0, 0xB0, class_w_reg_imm,    0,      NULL,     op_move_load,  OP_MOV    },

	{ 0xFF, 0xC2, class_imm,          CF_2,   NULL,     op_return,     OP_RET    },
	{ 0xFF, 0xC3, class_void,         CF_1,   NULL,     op_return,     OP_RET    },

	{ 0xFF, 0xC4, class_mod_reg_rm,   0,      "LES",    NULL,          0        },
	{ 0xFF, 0xC5, class_mod_reg_rm,   0,      "LDS",    NULL,          0        },
	{ 0xFE, 0xC6, class_w_mod_rm_imm, 0,      NULL,     op_move_load,  OP_MOV   },

	{ 0xFF, 0xCA, class_imm,          CF_2,   NULL,     op_return,     OP_RETF  },
	{ 0xFF, 0xCB, class_void,         CF_1,   NULL,     op_return,     OP_RETF  },

	{ 0xFF, 0xCC, class_void,         CF_1,   NULL,     op_int,        OP_INT3  },
	{ 0xFF, 0xCD, class_imm,          CF_1,   NULL,     op_int,        OP_INT   },
	{ 0xFF, 0xCE, class_void,         CF_1,   NULL,     op_int,        OP_INTO  },

	{ 0xFF, 0xCF, class_void,         CF_1,   NULL,     op_return,     OP_IRET  },

	{ 0xFC, 0xD0, class_hand_2,       0,      NULL,     class_2_D0h,   0        },

	{ 0xFF, 0xD4, class_void,         CF_2,   "AAM",    NULL,          0  },
	{ 0xFF, 0xD5, class_void,         CF_2,   "AAD",    NULL,          0  },
	{ 0xFF, 0xD7, class_void,         CF_1,   "XLAT",   NULL,          0  },
	{ 0xF8, 0xD8, class_mod_rm,       0,      "ESC ",   NULL,          0  },  // TODO: extract xxx xxx

	{ 0xFF, 0xE0, class_dist,         CF_1,   "LOOPNZ", NULL,          0  },
	{ 0xFF, 0xE1, class_dist,         CF_1,   "LOOPZ",  NULL,          0  },
	{ 0xFF, 0xE2, class_dist,         CF_1,   "LOOP",   NULL,          0  },
	{ 0xFF, 0xE3, class_dist,         CF_1,   "JCXZ",   NULL,          0  },

	{ 0xF6, 0xE4, class_in_out,       0,      NULL,     op_port,       OP_IN    },
	{ 0xF6, 0xE6, class_in_out,       0,      NULL,     op_port,       OP_OUT   },

	{ 0xFF, 0xE8, class_dist,         CF_2,   NULL,     op_jump_call,  OP_CALL  },
	{ 0xFF, 0xE9, class_dist,         CF_2,   NULL,     op_jump_call,  OP_JMP   },
	{ 0xFF, 0xEB, class_dist,         CF_1,   NULL,     op_jump_call,  OP_JMP   },
//	{ 0xF6, 0xEC, class_in_out,       0,      NULL,     op_port,       OP_IN    },
//	{ 0xF6, 0xEE, class_in_out,       0,      NULL,     op_port,       OP_OUT   },

	{ 0xFF, 0xF0, class_void,         CF_1,   "LOCK",   NULL,  0  },
	{ 0xFF, 0xF2, class_void,         CF_1,   "REPNZ",  NULL,  0  },  // TODO: zero flag ?
	{ 0xFF, 0xF3, class_void,         CF_1,   "REPZ",   NULL,  0  },  // TODO: zero flag ?
	{ 0xFF, 0xF4, class_void,         CF_1,   "HLT",    NULL,  0  },

	{ 0xFF, 0xF5, class_void,         CF_1,   "CMC",    NULL,  0  },
	{ 0xFE, 0xF6, class_hand_2,       0,      NULL,     class_2_F6h,   0        },
	{ 0xFF, 0xF8, class_void,         CF_1,   "CLC",    NULL,  0  },
	{ 0xFF, 0xF9, class_void,         CF_1,   "STC",    NULL,  0  },
	{ 0xFF, 0xFA, class_void,         CF_1,   "CLI",    NULL,  0  },
	{ 0xFF, 0xFB, class_void,         CF_1,   "STI",    NULL,  0  },
	{ 0xFF, 0xFC, class_void,         CF_1,   "CLD",    NULL,  0  },
	{ 0xFF, 0xFD, class_void,         CF_1,   "STD",    NULL,  0  },

	{ 0xFF, 0xFE, class_hand_2,       0,      NULL,     class_2_FEh,   0        },
	{ 0xFF, 0xFF, class_hand_2,       0,      NULL,     class_2_FFh,   0        },

	{ 0x00, 0x00, NULL,               0,      NULL,     NULL,          0        }
	};


static int class_hand_1 (byte_t flags, op_desc_t *op_desc)
	{
	int err = 0;

	while (1)
		{
		byte_t code = fetch_code ();

		// common bit fields

		op_desc->seg1 = (code & 0x18) >> 3;
		op_desc->v1   = (code & 0x08) >> 3;
		op_desc->w1   = (code & 0x08) >> 3;
		op_desc->reg1 =  code & 0x07;
		op_desc->d    = (code & 0x02) >> 1;
		op_desc->s    = (code & 0x02) >> 1;
		op_desc->v2   = (code & 0x02) >> 1;
		op_desc->w2   =  code & 0x01;

		class_desc_t * class_desc = class_find (class_1, code);
		if (!class_desc)
			{
			err = -1;  // unknown opcode
			break;
			}

		op_desc->name     = class_desc->name;
		op_desc->op_hand  = class_desc->op_hand;
		op_desc->op_id    = class_desc->op_id;

		class_hand_t class_hand = class_desc->class_hand;
		assert (class_hand);
		err = (*class_hand) (class_desc->class_flags, op_desc);
		break;
		}

	return err;
	}


int op_decode (op_desc_t * op_desc)
	{
	memset (op_code_str, 0, sizeof op_code_str);
	op_code_pos = 0;

	return class_hand_1 (0, op_desc);
	}

