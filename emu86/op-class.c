// DIS86 - 80x86 disassembler
// Operation classes

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "op-id.h"
#include "op-id-name.h"

#include "op-class.h"
#include "op-exec.h"


// Opcode helpers

byte_t * op_code_base;
word_t op_code_cs;
word_t op_code_ip;

char op_code_str [3 * OPCODE_MAX];
byte_t op_code_pos;

// TODO: merge CS & IP code pointers

static byte_t fetch_byte ()
	{
	byte_t b = *(op_code_base + (op_code_cs << 4) + op_code_ip++);
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


// Register classes

static char * reg8_names  [] = { "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH" };
static char * reg16_names [] = { "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI" };
static char * seg_names   [] = { "ES", "CS", "SS", "DS" };

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
	var->size = w ? VS_WORD : VS_BYTE;

	if (mod == 3)
		{
		var->type = VT_REG;
		var->val.r = rm;
		}
	else
		{
		var->type = VT_INDEX;

		word_t w = 0;
		short rel = 0;
		byte_t flags = _rm_flags [rm];

		switch (mod)
			{
			case 0:  // no displacement
				if (rm == 6)  // special case [BP] -> [disp16]
					{
					rel = (short) fetch_word ();
					flags = AF_DISP;
					}

				break;

			case 1:  // signed byte displacement
				w = (word_t) fetch_byte ();
				if (w & 0x0080) w |= 0xFF00;
				rel = (short) w;
				flags |= AF_DISP;
				break;

			case 2:  // signed word displacement
				rel = (short) fetch_word ();
				flags |= AF_DISP;
				break;

			}

		var->flags = flags;
		var->val.s = rel;
		}
	}

static void print_addr (byte_t flags, short rel)
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
		if (reg & (rel >= 0)) putchar ('+');
		printf ("%i", rel);
		}

	putchar (']');
	}


// Variable class

static void print_var (op_var_t * var)
	{
	byte_t rt;

	switch (var->type)
		{
		case VT_IMM:
			if (var->size == VS_BYTE) printf ("%.2X",var->val.w);
			if (var->size == VS_WORD) printf ("%.4X",var->val.w);
			break;

		case VT_REG:
			rt = (var->size == VS_BYTE) ? RT_REG8 : RT_REG16;
			print_reg (rt, var->val.r);
			break;

		case VT_SEG:
			print_reg (RT_SEG, var->val.r);
			break;

		case VT_INDEX:
			print_addr (var->flags, var->val.s);
			break;

		case VT_NEAR:
			printf ("%.4X", op_code_ip + var->val.s);
			break;

		case VT_FAR:
			printf ("%.4X",var->seg);
			putchar (':');
			printf ("%.4X",var->val.w);
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
	return 0;
	}


static int class_imm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;

	op_var_t * var_imm = &(op->var_to);
	var_imm->type  = VT_IMM;

	if (flags & CF_1)
		{
		var_imm->size = VS_BYTE;
		var_imm->val.b = fetch_byte ();
		}

	if (flags & CF_2)
		{
		var_imm->size = VS_WORD;
		var_imm->val.w = fetch_word ();
		}

	return 0;
	}


static int class_dist (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;

	op_var_t * var_dist = &(op->var_to);
	var_dist->type  = VT_NEAR;

	if (flags & CF_1)
		{
		var_dist->val.s = (short) fetch_byte ();
		}

	if (flags & CF_2)
		{
		var_dist->val.s = (short) fetch_word ();
		}

	return 0;
	}


static int class_off_seg (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;

	op_var_t * var_far = &(op->var_to);
	var_far->type  = VT_FAR;

	var_far->val.w = fetch_word ();
	var_far->seg = fetch_word ();

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
	var_acc->val.r = 0;  // AX or AL

	if (op->v1)
		{
		var_port->type  = VT_REG;
		var_port->size = VS_WORD;
		var_port->val.r = 2;  // DX
		}

	else
		{
		var_port->type = VT_IMM;
		var_port->size = VS_BYTE;
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
		var_acc->type  = VT_REG;
		var_acc->size  = VS_WORD;
		var_acc->val.r = 0;  // AX

		var_reg = &(op->var_from);
		}

	else
		{
		op->var_count = 1;

		var_reg = &(op->var_to);
		}

	var_reg->type  = VT_REG;
	var_reg->size  = VS_WORD;
	var_reg->val.r = op->reg1;

	return 0;
	}


static int class_seg (byte_t flags, op_desc_t * op)
	{
	op->var_count = 1;

	op_var_t * var_seg = &(op->var_to);

	var_seg->type  = VT_SEG;
	var_seg->val.r = op->seg1;

	return 0;
	}


static int class_w_imm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;

	op_var_t * var_acc = &(op->var_to);
	op_var_t * var_imm = &(op->var_from);

	var_acc->type  = VT_REG;
	var_acc->val.r = 0;  // AX or AL

	var_imm->type  = VT_IMM;

	if (op->w2)
		{
		var_acc->size  = VS_WORD;
		var_imm->val.w = fetch_word ();
		}

	else
		{
		var_acc->size  = VS_BYTE;
		var_imm->val.b = fetch_byte ();
		}

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
	var_acc->val.r = 0;  // AX or AL
	var_acc->size  = op->w2 ? VS_WORD : VS_BYTE;

	var_addr->type  = VT_NEAR;
	var_addr->flags = 0;
	var_addr->val.s = (short) fetch_word ();

	return 0;
	}


static int class_w_reg_imm (byte_t flags, op_desc_t * op)
	{
	op->var_count = 2;

	op_var_t * var_reg = &(op->var_to);
	op_var_t * var_imm = &(op->var_from);

	var_reg->type  = VT_REG;
	var_reg->val.r = op->reg1;

	var_imm->type   = VT_IMM;

	if (op->w1)
		{
		var_reg->size  = VS_WORD;
		var_imm->size  = VS_WORD;
		var_imm->val.w = fetch_word ();
		}
	else
		{
		var_reg->size  = VS_BYTE;
		var_imm->size  = VS_BYTE;
		var_imm->val.b = fetch_byte ();
		}

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
			var_num->val.r = 1; // CL
			}
		else
			{
			var_num->type = VT_IMM;
			var_num->size = VS_BYTE;
			var_num->val.w = 1;
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

	if (op->w2)
		{
		var_imm->size  = VS_WORD;

		if ((flags & CF_S) && op->s)
			{
			word_t w = (word_t) fetch_byte ();
			if (w & 0x0080) w |= 0xFF00;
			var_imm->val.w = w;
			}

		else
			{
			var_imm->val.w = fetch_word ();
			}
		}

	else
		{
		var_imm->size  = VS_BYTE;
		var_imm->val.b = fetch_byte (1);
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
	var_reg->val.r = op->reg2;

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
	var_reg->val.r = op->reg2;

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
	var_seg->val.r = op->seg2;

	scan_mod_rm (1, op->mod, op->rm, var_rm);
	return 0;
	}


// Second byte code table

static class_desc_t class_2_80h [] = {
	{ 0x38, 0x00, 1, NULL, class_w_mod_rm_imm, CF_S,  OP_ADD   },
	{ 0x38, 0x08, 1, NULL, class_w_mod_rm_imm, 0,     OP_OR    },
	{ 0x38, 0x10, 1, NULL, class_w_mod_rm_imm, CF_S,  OP_ADC   },
	{ 0x38, 0x18, 1, NULL, class_w_mod_rm_imm, CF_S,  OP_SBB   },
	{ 0x38, 0x20, 1, NULL, class_w_mod_rm_imm, 0,     OP_AND   },
	{ 0x38, 0x28, 1, NULL, class_w_mod_rm_imm, CF_S,  OP_SUB   },
	{ 0x38, 0x30, 1, NULL, class_w_mod_rm_imm, 0,     OP_XOR   },
	{ 0x38, 0x38, 1, NULL, class_w_mod_rm_imm, CF_S,  OP_CMP   },
	{ 0x00, 0x00, 0, NULL, NULL,               0,     0        }
	};

static class_desc_t class_2_D0h [] = {
	{ 0x38, 0x00, 1, NULL, class_w_mod_rm,     CF_V,  OP_ROL   },
	{ 0x38, 0x08, 1, NULL, class_w_mod_rm,     CF_V,  OP_ROR   },
	{ 0x38, 0x10, 1, NULL, class_w_mod_rm,     CF_V,  OP_RCL   },
	{ 0x38, 0x18, 1, NULL, class_w_mod_rm,     CF_V,  OP_RCR   },
	{ 0x38, 0x20, 1, NULL, class_w_mod_rm,     CF_V,  OP_SHL   },
	{ 0x38, 0x28, 1, NULL, class_w_mod_rm,     CF_V,  OP_SHR   },
	{ 0x38, 0x30, 1, NULL, class_w_mod_rm,     CF_V,  OP_SAL   },
	{ 0x38, 0x38, 1, NULL, class_w_mod_rm,     CF_V,  OP_SAR   },
	{ 0x00, 0x00, 0, NULL, NULL,               0,     0        }
	};

static class_desc_t class_2_F6h [] = {
	{ 0x38, 0x00, 1, NULL, class_w_mod_rm_imm, 0,     OP_TEST  },
	{ 0x38, 0x10, 1, NULL, class_w_mod_rm,     0,     OP_NOT   },
	{ 0x38, 0x18, 1, NULL, class_w_mod_rm,     0,     OP_NEG   },
	{ 0x38, 0x20, 1, NULL, class_w_mod_rm,     0,     OP_MUL   },
	{ 0x38, 0x28, 1, NULL, class_w_mod_rm,     0,     OP_IMUL  },
	{ 0x38, 0x30, 1, NULL, class_w_mod_rm,     0,     OP_DIV   },
	{ 0x38, 0x38, 1, NULL, class_w_mod_rm,     0,     OP_IDIV  },
	{ 0x00, 0x00, 0, NULL, NULL,               0,     0        }
	};

static class_desc_t class_2_FEh [] = {
	{ 0x38, 0x00, 1, NULL, class_w_mod_rm,     0,     OP_INC   },
	{ 0x38, 0x08, 1, NULL, class_w_mod_rm,     0,     OP_DEC   },
	{ 0x00, 0x00, 0, NULL, NULL,               0,     0        }
	};

static class_desc_t class_2_FFh [] = {
	{ 0x38, 0x10, 1, NULL, class_mod_rm,       0,     OP_CALL  },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x38, 0x18, 1, NULL, class_mod_rm,       0,     OP_CALLF },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x38, 0x20, 1, NULL, class_mod_rm,       0,     OP_JMP   },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x38, 0x28, 1, NULL, class_mod_rm,       0,     OP_JMPF  },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x38, 0x30, 1, NULL, class_mod_rm,       0,     OP_PUSH  },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x00, 0x00, 0, NULL, NULL,               0,     0        }
	};


// First byte code table

static class_desc_t _class_1 [] = {
	{ 0xFC, 0x00, 2, NULL,        class_w_mod_reg_rm, CF_D,  OP_ADD    },
	{ 0xFC, 0x08, 2, NULL,        class_w_mod_reg_rm, CF_D,  OP_OR     },
	{ 0xFC, 0x10, 2, NULL,        class_w_mod_reg_rm, CF_D,  OP_ADC    },
	{ 0xFC, 0x18, 2, NULL,        class_w_mod_reg_rm, CF_D,  OP_SBB    },
	{ 0xFC, 0x20, 2, NULL,        class_w_mod_reg_rm, CF_D,  OP_AND    },
	{ 0xFC, 0x28, 2, NULL,        class_w_mod_reg_rm, CF_D,  OP_SUB    },
	{ 0xFC, 0x30, 2, NULL,        class_w_mod_reg_rm, CF_D,  OP_XOR    },
	{ 0xFC, 0x38, 2, NULL,        class_w_mod_reg_rm, CF_D,  OP_CMP    },

	{ 0xFE, 0x04, 1, NULL,        class_w_imm,        0,     OP_ADD    },
	{ 0xFE, 0x0C, 1, NULL,        class_w_imm,        0,     OP_OR     },
	{ 0xFE, 0x14, 1, NULL,        class_w_imm,        0,     OP_ADC    },
	{ 0xFE, 0x1C, 1, NULL,        class_w_imm,        0,     OP_SBB    },
	{ 0xFE, 0x24, 1, NULL,        class_w_imm,        0,     OP_AND    },
	{ 0xFE, 0x2C, 1, NULL,        class_w_imm,        0,     OP_SUB    },
	{ 0xFE, 0x34, 1, NULL,        class_w_imm,        0,     OP_XOR    },
	{ 0xFE, 0x3C, 1, NULL,        class_w_imm,        0,     OP_CMP    },

	{ 0xE7, 0x06, 1, NULL,        class_seg,          0,     OP_PUSH   },
	{ 0xE7, 0x07, 1, NULL,        class_seg,          0,     OP_POP    },

	{ 0xE7, 0x26, 1, NULL,        class_seg,          0,     OP_SEG    },

	{ 0xFF, 0x27, 1, NULL,        class_void,         0,     OP_DAA    },
	{ 0xFF, 0x2F, 1, NULL,        class_void,         0,     OP_DAS    },
	{ 0xFF, 0x37, 1, NULL,        class_void,         0,     OP_AAA    },
	{ 0xFF, 0x3F, 1, NULL,        class_void,         0,     OP_AAS    },

	{ 0xF8, 0x40, 1, NULL,        class_reg,          0,     OP_INC    },
	{ 0xF8, 0x48, 1, NULL,        class_reg,          0,     OP_DEC    },

	{ 0xF8, 0x50, 1, NULL,        class_reg,          0,     OP_PUSH   },
	{ 0xF8, 0x58, 1, NULL,        class_reg,          0,     OP_POP    },

	{ 0xFF, 0x70, 1, NULL,        class_dist,         CF_1,  OP_JO     },
	{ 0xFF, 0x71, 1, NULL,        class_dist,         CF_1,  OP_JNO    },
	{ 0xFF, 0x72, 1, NULL,        class_dist,         CF_1,  OP_JB     },
	{ 0xFF, 0x73, 1, NULL,        class_dist,         CF_1,  OP_JNB    },
	{ 0xFF, 0x74, 1, NULL,        class_dist,         CF_1,  OP_JZ     },
	{ 0xFF, 0x75, 1, NULL,        class_dist,         CF_1,  OP_JNZ    },
	{ 0xFF, 0x76, 1, NULL,        class_dist,         CF_1,  OP_JNA    },
	{ 0xFF, 0x77, 1, NULL,        class_dist,         CF_1,  OP_JA     },
	{ 0xFF, 0x78, 1, NULL,        class_dist,         CF_1,  OP_JS     },
	{ 0xFF, 0x79, 1, NULL,        class_dist,         CF_1,  OP_JNS    },
	{ 0xFF, 0x7A, 1, NULL,        class_dist,         CF_1,  OP_JP     },
	{ 0xFF, 0x7B, 1, NULL,        class_dist,         CF_1,  OP_JNP    },
	{ 0xFF, 0x7C, 1, NULL,        class_dist,         CF_1,  OP_JL     },
	{ 0xFF, 0x7D, 1, NULL,        class_dist,         CF_1,  OP_JNL    },
	{ 0xFF, 0x7E, 1, NULL,        class_dist,         CF_1,  OP_JNG    },
	{ 0xFF, 0x7F, 1, NULL,        class_dist,         CF_1,  OP_JG     },

	{ 0xFC, 0x80, 2, class_2_80h, NULL,               0,     0         },

	{ 0xFE, 0x84, 2, NULL,        class_w_mod_reg_rm, 0,     OP_TEST   },
	{ 0xFE, 0x86, 2, NULL,        class_w_mod_reg_rm, 0,     OP_XCHG   },

	{ 0xFC, 0x88, 2, NULL,        class_w_mod_reg_rm, CF_D,  OP_MOV    },

	{ 0xFD, 0x8C, 2, NULL,        class_mod_seg_rm,   CF_D,  OP_MOV    },
	{ 0xFF, 0x8D, 2, NULL,        class_mod_reg_rm,   0,     OP_LEA    },
//	{ 0xFD, 0x8E, 2, NULL,        class_mod_seg_rm,   CF_D,  OP_MOV    },
	{ 0xFF, 0x8F, 2, NULL,        class_mod_rm,       0,     OP_POP    },  // TODO: class_w_mod_rm & w=1 ?

	{ 0xF8, 0x90, 1, NULL,        class_reg,          CF_A,  OP_XCHG   },

	{ 0xFF, 0x98, 1, NULL,        class_void,         0,     OP_CBW    },
	{ 0xFF, 0x99, 1, NULL,        class_void,         0,     OP_CWD    },
//	{ 0xFF, 0x9A, 1, NULL,        NULL,               0,     0         },
	{ 0xFF, 0x9B, 1, NULL,        class_void,         0,     OP_WAIT   },

	{ 0xFF, 0x9C, 1, NULL,        class_void,         0,     OP_PUSHF  },
	{ 0xFF, 0x9D, 1, NULL,        class_void,         0,     OP_POPF   },

	{ 0xFF, 0x9E, 1, NULL,        class_void,         0,     OP_SAHF   },
	{ 0xFF, 0x9F, 1, NULL,        class_void,         0,     OP_LAHF   },

	{ 0xFF, 0x9A, 1, NULL,        class_off_seg,      0,     OP_CALLF  },
	{ 0xFF, 0xEA, 1, NULL,        class_off_seg,      0,     OP_JMPF   },

	{ 0xFC, 0xA0, 1, NULL,        class_w_addr,       CF_D,  OP_MOV    },

	{ 0xFE, 0xA8, 1, NULL,        class_w_imm,        0,     OP_TEST   },

	{ 0xFE, 0xA4, 1, NULL,        class_void,         0,     OP_MOVSB  },
	{ 0xFE, 0xA5, 1, NULL,        class_void,         0,     OP_MOVSW  },
	{ 0xFE, 0xA6, 1, NULL,        class_void,         0,     OP_CMPSB  },
	{ 0xFE, 0xA7, 1, NULL,        class_void,         0,     OP_CMPSW  },
	{ 0xFE, 0xAA, 1, NULL,        class_void,         0,     OP_STOSB  },
	{ 0xFE, 0xAB, 1, NULL,        class_void,         0,     OP_STOSW  },
	{ 0xFE, 0xAC, 1, NULL,        class_void,         0,     OP_LODSB  },
	{ 0xFE, 0xAD, 1, NULL,        class_void,         0,     OP_LODSW  },
	{ 0xFE, 0xAE, 1, NULL,        class_void,         0,     OP_SCASB  },
	{ 0xFE, 0xAF, 1, NULL,        class_void,         0,     OP_SCASW  },

	{ 0xF0, 0xB0, 1, NULL,        class_w_reg_imm,    0,     OP_MOV    },

	{ 0xFF, 0xC2, 1, NULL,        class_imm,          CF_2,  OP_RET    },
	{ 0xFF, 0xC3, 1, NULL,        class_void,         0,     OP_RET    },

	{ 0xFF, 0xC4, 2, NULL,        class_mod_reg_rm,   0,     OP_LES    },
	{ 0xFF, 0xC5, 2, NULL,        class_mod_reg_rm,   0,     OP_LDS    },
	{ 0xFE, 0xC6, 2, NULL,        class_w_mod_rm_imm, 0,     OP_MOV    },

	{ 0xFF, 0xCA, 1, NULL,        class_imm,          CF_2,  OP_RETF   },
	{ 0xFF, 0xCB, 1, NULL,        class_void,         0,     OP_RETF   },

	{ 0xFF, 0xCC, 1, NULL,        class_void,         0,     OP_INT3   },
	{ 0xFF, 0xCD, 1, NULL,        class_imm,          CF_1,  OP_INT    },
	{ 0xFF, 0xCE, 1, NULL,        class_void,         0,     OP_INTO   },

	{ 0xFF, 0xCF, 1, NULL,        class_void,         0,     OP_IRET   },

	{ 0xFC, 0xD0, 2, class_2_D0h, NULL,               0,     0         },

	{ 0xFF, 0xD4, 1, NULL,        class_void,         0,     OP_AAM    },
	{ 0xFF, 0xD5, 1, NULL,        class_void,         0,     OP_AAD    },
	{ 0xFF, 0xD7, 1, NULL,        class_void,         0,     OP_XLAT   },
	{ 0xF8, 0xD8, 1, NULL,        class_mod_rm,       0,     OP_ESC    },  // TODO: extract xxx xxx

	{ 0xFF, 0xE0, 1, NULL,        class_dist,         CF_1,  OP_LOOPNZ },
	{ 0xFF, 0xE1, 1, NULL,        class_dist,         CF_1,  OP_LOOPZ  },
	{ 0xFF, 0xE2, 1, NULL,        class_dist,         CF_1,  OP_LOOP   },
	{ 0xFF, 0xE3, 1, NULL,        class_dist,         CF_1,  OP_JCXZ   },

	{ 0xF6, 0xE4, 1, NULL,        class_in_out,       0,     OP_IN     },
	{ 0xF6, 0xE6, 1, NULL,        class_in_out,       0,     OP_OUT    },

	{ 0xFF, 0xE8, 1, NULL,        class_dist,         CF_2,  OP_CALL   },
	{ 0xFF, 0xE9, 1, NULL,        class_dist,         CF_2,  OP_JMP    },
	{ 0xFF, 0xEB, 1, NULL,        class_dist,         CF_1,  OP_JMP    },
//	{ 0xF6, 0xEC, 1, NULL,        class_in_out,       0,     OP_IN     },
//	{ 0xF6, 0xEE, 1, NULL,        class_in_out,       0,     OP_OUT    },

	{ 0xFF, 0xF0, 1, NULL,        class_void,         0,     OP_LOCK   },
	{ 0xFF, 0xF2, 1, NULL,        class_void,         0,     OP_REPNZ  },  // TODO: zero flag ?
	{ 0xFF, 0xF3, 1, NULL,        class_void,         0,     OP_REPZ   },  // TODO: zero flag ?
	{ 0xFF, 0xF4, 1, NULL,        class_void,         0,     OP_HLT    },

	{ 0xFF, 0xF5, 1, NULL,        class_void,         0,     OP_CMC    },
	{ 0xFE, 0xF6, 2, class_2_F6h, NULL,               0,     0         },
	{ 0xFF, 0xF8, 1, NULL,        class_void,         0,     OP_CLC    },
	{ 0xFF, 0xF9, 1, NULL,        class_void,         0,     OP_STC    },
	{ 0xFF, 0xFA, 1, NULL,        class_void,         0,     OP_CLI    },
	{ 0xFF, 0xFB, 1, NULL,        class_void,         0,     OP_STI    },
	{ 0xFF, 0xFC, 1, NULL,        class_void,         0,     OP_CLD    },
	{ 0xFF, 0xFD, 1, NULL,        class_void,         0,     OP_STD    },

	{ 0xFF, 0xFE, 2, class_2_FEh, NULL,               0,     0         },
	{ 0xFF, 0xFF, 2, class_2_FFh, NULL,               0,     0         },

	{ 0x00, 0x00, 0, NULL,        NULL,               0,     0         }
	};


static class_desc_t * class_find (class_desc_t * tab, byte_t op)
	{
	class_desc_t * desc = tab;

	while (1)
		{
		if (!desc->len)
			{
			desc = NULL; // unknown opcode
			break;
			}

		if ((op & desc->mask) == desc->code) break;
		desc++;
		}

	return desc;
	}


int op_decode (op_desc_t * op_desc)
	{
	int err = -1;

	while (1)
		{
		memset (op_code_str, 0, sizeof op_code_str);
		op_code_pos = 0;

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

		class_desc_t * class_desc = class_find (_class_1, code);
		if (!class_desc)
			{
			assert (0); // unknown opcode
			break;
			}

		if (class_desc->len > 1)
			{
			byte_t code = fetch_byte ();

			// Common bit fields

			op_desc->mod  = (code & 0xC0) >> 6;
			op_desc->reg2 = (code & 0x38) >> 3;
			op_desc->seg2 = (code & 0x18) >> 3;
			op_desc->rm   =  code & 0x07;

			class_desc_t * sub = class_desc->sub;
			if (sub)
				{
				class_desc = class_find (sub, code);
				if (!class_desc)
					{
					assert (0); // unknown opcode
					break;
					}
				}
			}

		op_desc->op_id = class_desc->op_id;

		class_hand_t hand = class_desc->class_hand;
		assert (hand);
		err = (*hand) (class_desc->class_flags, op_desc);
		break;
		}

	return err;
	}
