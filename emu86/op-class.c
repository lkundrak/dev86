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


static int class_w_reg_imm (byte_t flags, op_desc_t * op)
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


// Second byte code table

// TODO: reduce table size with operation bit field (0x38 mask)
// TODO: reduce code with MOD-RM as new table field

static class_desc_t class_2_80h [] = {
	{ 0x38, 0x00, 1, NULL, class_w_mod_rm_imm, CF_S,  OP_ADD   },
	{ 0x38, 0x08, 1, NULL, class_w_mod_rm_imm, CF_S,  OP_OR    },  // S not compliant with Intel doc
	{ 0x38, 0x10, 1, NULL, class_w_mod_rm_imm, CF_S,  OP_ADC   },
	{ 0x38, 0x18, 1, NULL, class_w_mod_rm_imm, CF_S,  OP_SBB   },
	{ 0x38, 0x20, 1, NULL, class_w_mod_rm_imm, CF_S,  OP_AND   },  // S not compliant with Intel doc
	{ 0x38, 0x28, 1, NULL, class_w_mod_rm_imm, CF_S,  OP_SUB   },
	{ 0x38, 0x30, 1, NULL, class_w_mod_rm_imm, CF_S,  OP_XOR   },  // S not compliant with Intel doc
	{ 0x38, 0x38, 1, NULL, class_w_mod_rm_imm, CF_S,  OP_CMP   },
	{ 0x00, 0x00, 0, NULL, NULL,               0,     0        }
	};

static class_desc_t class_2_C0h [] = {
	{ 0x38, 0x00, 1, NULL, class_w_mod_rm_count,     0,  OP_ROL   },
	{ 0x38, 0x08, 1, NULL, class_w_mod_rm_count,     0,  OP_ROR   },
	{ 0x38, 0x10, 1, NULL, class_w_mod_rm_count,     0,  OP_RCL   },
	{ 0x38, 0x18, 1, NULL, class_w_mod_rm_count,     0,  OP_RCR   },
	{ 0x38, 0x20, 1, NULL, class_w_mod_rm_count,     0,  OP_SHL   },
	{ 0x38, 0x28, 1, NULL, class_w_mod_rm_count,     0,  OP_SHR   },
	{ 0x38, 0x30, 1, NULL, class_w_mod_rm_count,     0,  OP_SAL   },  // TODO: SHL = SAL
	{ 0x38, 0x38, 1, NULL, class_w_mod_rm_count,     0,  OP_SAR   },
	{ 0x00, 0x00, 0, NULL, NULL,                     0,  0        }
	};

static class_desc_t class_2_D0h [] = {
	{ 0x38, 0x00, 1, NULL, class_w_mod_rm,     CF_V,  OP_ROL   },
	{ 0x38, 0x08, 1, NULL, class_w_mod_rm,     CF_V,  OP_ROR   },
	{ 0x38, 0x10, 1, NULL, class_w_mod_rm,     CF_V,  OP_RCL   },
	{ 0x38, 0x18, 1, NULL, class_w_mod_rm,     CF_V,  OP_RCR   },
	{ 0x38, 0x20, 1, NULL, class_w_mod_rm,     CF_V,  OP_SHL   },
	{ 0x38, 0x28, 1, NULL, class_w_mod_rm,     CF_V,  OP_SHR   },
	{ 0x38, 0x30, 1, NULL, class_w_mod_rm,     CF_V,  OP_SAL   },  // TODO: SHL = SAL
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
	{ 0x38, 0x10, 1, NULL, class_mod_rm,       0,     OP_CALL  },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x38, 0x18, 1, NULL, class_mod_rm,       CF_F,  OP_CALLF },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x38, 0x20, 1, NULL, class_mod_rm,       0,     OP_JMP   },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x38, 0x28, 1, NULL, class_mod_rm,       CF_F,  OP_JMPF  },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x38, 0x30, 1, NULL, class_mod_rm,       0,     OP_PUSH  },  // TODO: class_w_mod_rm & w=1 ?
	{ 0x00, 0x00, 0, NULL, NULL,               0,     0        }
	};


// First byte code tables

static class_desc_t _class_1_80h [] = {

	{ 0xFC, 0x80, 2, class_2_80h, NULL,               0,     0         },

	{ 0xFE, 0x84, 2, NULL,        class_w_mod_reg_rm, 0,     OP_TEST   },
	{ 0xFE, 0x86, 2, NULL,        class_w_mod_reg_rm, 0,     OP_XCHG   },

	{ 0xFC, 0x88, 2, NULL,        class_w_mod_reg_rm, CF_D,  OP_MOV    },

	{ 0xFD, 0x8C, 2, NULL,        class_mod_seg_rm,   CF_D,  OP_MOV    },
	{ 0xFF, 0x8D, 2, NULL,        class_mod_reg_rm,   0,     OP_LEA    },
	{ 0xFF, 0x8F, 2, NULL,        class_mod_rm,       0,     OP_POP    },  // TODO: class_w_mod_rm & w=1 ?

	{ 0xF8, 0x90, 1, NULL,        class_reg,          CF_A,  OP_XCHG   },

	{ 0xFF, 0x98, 1, NULL,        class_void,         0,     OP_CBW    },
	{ 0xFF, 0x99, 1, NULL,        class_void,         0,     OP_CWD    },
	{ 0xFF, 0x9B, 1, NULL,        class_void,         0,     OP_WAIT   },

	{ 0xFF, 0x9C, 1, NULL,        class_void,         0,     OP_PUSHF  },
	{ 0xFF, 0x9D, 1, NULL,        class_void,         0,     OP_POPF   },

	{ 0xFF, 0x9E, 1, NULL,        class_void,         0,     OP_SAHF   },
	{ 0xFF, 0x9F, 1, NULL,        class_void,         0,     OP_LAHF   },

	{ 0xFF, 0x9A, 1, NULL,        class_off_seg,      0,     OP_CALLF  },
	{ 0xFF, 0xEA, 1, NULL,        class_off_seg,      0,     OP_JMPF   },

	{ 0xFC, 0xA0, 1, NULL,        class_d_w_addr,     0,     OP_MOV    },

	{ 0xFE, 0xA8, 1, NULL,        class_w_imm,        0,     OP_TEST   },

	{ 0xFF, 0xA4, 1, NULL,        class_void,         0,     OP_MOVSB  },  // TODO: flag W at operation level
	{ 0xFF, 0xA5, 1, NULL,        class_void,         0,     OP_MOVSW  },
	{ 0xFF, 0xA6, 1, NULL,        class_void,         0,     OP_CMPSB  },
	{ 0xFF, 0xA7, 1, NULL,        class_void,         0,     OP_CMPSW  },
	{ 0xFF, 0xAA, 1, NULL,        class_void,         0,     OP_STOSB  },
	{ 0xFF, 0xAB, 1, NULL,        class_void,         0,     OP_STOSW  },
	{ 0xFF, 0xAC, 1, NULL,        class_void,         0,     OP_LODSB  },
	{ 0xFF, 0xAD, 1, NULL,        class_void,         0,     OP_LODSW  },
	{ 0xFF, 0xAE, 1, NULL,        class_void,         0,     OP_SCASB  },
	{ 0xFF, 0xAF, 1, NULL,        class_void,         0,     OP_SCASW  },

	{ 0xF0, 0xB0, 1, NULL,        class_w_reg_imm,    0,     OP_MOV    },

	{ 0x00, 0x00, 0, NULL,        NULL,               0,     0         }
	};

static class_desc_t _class_1_C0h [] = {

	{ 0xFE, 0xC0, 2, class_2_C0h, NULL,               0,     0         },

	{ 0xFF, 0xC2, 1, NULL,        class_imm,          CF_2,  OP_RET    },
	{ 0xFF, 0xC3, 1, NULL,        class_void,         0,     OP_RET    },

	{ 0xFF, 0xC4, 2, NULL,        class_mod_reg_rm,   CF_F,  OP_LES    },
	{ 0xFF, 0xC5, 2, NULL,        class_mod_reg_rm,   CF_F,  OP_LDS    },
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

	{ 0xFF, 0xE0, 1, NULL,        class_off,         CF_1,  OP_LOOPNZ },
	{ 0xFF, 0xE1, 1, NULL,        class_off,         CF_1,  OP_LOOPZ  },
	{ 0xFF, 0xE2, 1, NULL,        class_off,         CF_1,  OP_LOOP   },
	{ 0xFF, 0xE3, 1, NULL,        class_off,         CF_1,  OP_JCXZ   },

	{ 0xF6, 0xE4, 1, NULL,        class_in_out,       0,     OP_IN     },
	{ 0xF6, 0xE6, 1, NULL,        class_in_out,       0,     OP_OUT    },

	{ 0xFF, 0xE8, 1, NULL,        class_off,         CF_2,  OP_CALL   },
	{ 0xFF, 0xE9, 1, NULL,        class_off,         CF_2,  OP_JMP    },
	{ 0xFF, 0xEB, 1, NULL,        class_off,         CF_1,  OP_JMP    },

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

	{ 0xFE, 0xFE, 2, class_2_FEh, NULL,               0,     0         },

	{ 0x00, 0x00, 0, NULL,        NULL,               0,     0         }
	};


// Iteration counter to measure decoding optimization
int class_iter_count = 0;

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

		class_iter_count++;
		}

	return desc;
	}


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
		if ((code & 0x04) == 0)
			{
			op_desc->op_id = OP_CALC2 + ((code & 0x38) >> 3);
			code = fetch_code_2 (op_desc);
			err = class_w_mod_reg_rm (CF_D, op_desc);
			break;
			}

		// code & 0x04 == 1
		if ((code & 0x02) == 0)
			{
			op_desc->op_id = OP_CALC2 + ((code & 0x38) >> 3);
			err = class_w_imm (0, op_desc);
			break;
			}

		// code & 0x02 == 1
		if ((code & 0x20) == 0)
			{
			op_desc->op_id = OP_STACK1 + (code & 0x01);
			err = class_seg (0, op_desc);
			break;
			}

		// code & 0x20 == 1
		if ((code & 0x01) == 0)
			{
			op_desc->op_id = OP_SEG;
			err = class_seg (0, op_desc);
			break;
			}

		// code & 0x01 == 1
		op_desc->op_id = OP_ADJUST1 + ((code & 0x18) >> 3);
		err = class_void (0, op_desc);
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
			op_desc->op_id = OP_STEP1 + ((code & 0x08) >> 3);
			err = class_reg (0, op_desc);
			break;

		case 0x10:
			op_desc->op_id = OP_STACK1 + ((code & 0x08) >> 3);
			err = class_reg (0, op_desc);
			break;

		case 0x20:
			if (code & 0x0E)
				{
				// Unknown opcodes for 80x86
				err = -1;
				break;
				}

			// code & 0x0E = 0
			op_desc->op_id = OP_STACK2 + (code & 0x01);
			err = class_void (0, op_desc);  // TODO: remove class_void()
			break;

		case 0x30:
			op_desc->op_id = OP_JUMP1 + (code & 0x0F);
			err = class_off (CF_1, op_desc);
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

		// Optimized decoding

		class_desc_t * class_desc = NULL;

		switch (code & 0xC0)
			{
			case 0x00:
				err = class_1_00h (code, op_desc);
				break;

			case 0x40:
				err = class_1_40h (code, op_desc);
				break;

			case 0x80:
				class_desc = class_find (_class_1_80h, code);
				break;

			case 0xC0:
				class_desc = class_find (_class_1_C0h, code);
				break;

			}

		if (!class_desc) break;  // unknown opcode

		// Table decoding (non optimized)

		if (class_desc->len >= 2)
			{
			byte_t code = fetch_code_2 (op_desc);

			class_desc_t * sub = class_desc->sub;
			if (sub)
				{
				class_desc = class_find (sub, code);
				if (!class_desc) break;  // unknown opcode
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
