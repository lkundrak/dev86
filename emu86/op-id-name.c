
#include <stdlib.h>
#include <assert.h>

#include "op-id-name.h"

struct op_id_name_s
	{
	word_t id;
	char *name;
	};

typedef struct op_id_name_s op_id_name_t;

static op_id_name_t id_name_tab [] = {
	{ OP_MOV,   "mov"   },
	{ OP_LEA,   "lea"   },
	{ OP_XCHG,  "xchg"  },
	{ OP_IN,    "in"    },
	{ OP_OUT,   "out" },
	{ OP_ADD,   "add" },
	{ OP_OR,    "or" },
	{ OP_ADC,   "adc" },
	{ OP_SBB,   "sbb" },
	{ OP_AND,   "and" },
	{ OP_SUB,   "sub" },
	{ OP_XOR,   "xor" },
	{ OP_CMP,   "cmp" },
	{ OP_TEST,  "test" },
	{ OP_NOT,   "not" },
	{ OP_NEG,   "neg" },
	{ OP_MUL,   "mul" },
	{ OP_IMUL,  "imul" },
	{ OP_DIV,   "div" },
	{ OP_IDIV,  "idiv" },
	{ OP_INC,   "inc" },
	{ OP_DEC,   "dec" },
	{ OP_ROL,   "rol" },
	{ OP_ROR,   "ror" },
	{ OP_RCL,   "rcl" },
	{ OP_RCR,   "rcr" },
	{ OP_SHL,   "shl" },
	{ OP_SHR,   "shr" },
	{ OP_SAL,   "sal" },
	{ OP_SAR,   "sar" },
    { OP_PUSH,  "push" },
    { OP_POP,   "pop" },
    { OP_PUSHF, "pushf" },
    { OP_POPF,  "popf" },
    { OP_JMP,   "jmp" },
    { OP_JMPF,  "jmpf" },
    { OP_CALL,  "call" },
    { OP_CALLF, "callf" },
    { OP_INT,   "int" },
    { OP_INT3,  "int3" },
    { OP_INTO,  "into" },
    { OP_RET,   "ret" },
    { OP_RETF,  "retf" },
    { OP_IRET,  "iret" },
    { OP_JO,    "jo" },
    { OP_JNO,   "jno" },
    { OP_JB,    "jb" },
    { OP_JNB,   "jnb" },
    { OP_JZ,    "jz" },
   	{ OP_JNZ,   "jnz" },
	{ OP_JNA,   "jna" },
	{ OP_JA,    "ja" },
	{ OP_JS,    "js" },
	{ OP_JNS,   "jns" },
	{ OP_JP,    "jp" },
	{ OP_JNP,   "jnp" },
	{ OP_JL,    "jl" },
	{ OP_JNL,   "jnl" },
	{ OP_JNG,   "jng" },
	{ OP_JG,    "jg" },
	{ OP_MOVSB, "movsb" },
	{ OP_MOVSW, "movsw" },
	{ OP_CMPSB, "cmpsb" },
	{ OP_CMPSW, "cmpsw" },
	{ OP_STOSB, "stosb" },
	{ OP_STOSW, "stosw" },
	{ OP_LODSB, "lodsb" },
	{ OP_LODSW, "lodsw" },
	{ OP_SCASB, "scasb" },
	{ OP_SCASW, "scasw" },
	{ 0xFFFF,   NULL    }
	};

char *op_id_to_name (word_t op_id)
	{
	char *name = NULL;
	op_id_name_t *op = id_name_tab;
	word_t id;

	while ((id = op->id) != 0xFFFF)
		{
		if (id == op_id)
			{
			name = op->name;
			break;
			}

		op++;
		}

	return name;
	}
