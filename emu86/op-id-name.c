
#include <stdlib.h>  // for NULL constant
//#include <assert.h>

#include "op-id.h"
#include "op-id-name.h"


struct op_id_name_s
	{
	word_t id;
	char *name;
	};

typedef struct op_id_name_s op_id_name_t;

static op_id_name_t id_name_tab [] = {
	{ OP_NULL,  "???"   },
	{ OP_CLC,   "CLC"   },
	{ OP_CLD,   "CLD"   },
	{ OP_CLI,   "CLI"   },
	{ OP_HLT,   "HLT"   },
	{ OP_LAHF,  "LAHF"  },
	{ OP_LOOP,  "LOOP"  },
	{ OP_MOV,   "MOV"   },
	{ OP_LEA,   "LEA"   },
	{ OP_XCHG,  "XCHG"  },
	{ OP_IN,    "IN"    },
	{ OP_OUT,   "OUT"   },
	{ OP_ADD,   "ADD"   },
	{ OP_OR,    "OR"    },
	{ OP_ADC,   "ADC"   },
	{ OP_SBB,   "SBB"   },
	{ OP_AND,   "AND"   },
	{ OP_SUB,   "SUB"   },
	{ OP_XOR,   "XOR"   },
	{ OP_CMP,   "CMP"   },
	{ OP_TEST,  "TEST"  },
	{ OP_NOT,   "NOT"   },
	{ OP_NEG,   "NEG"   },
	{ OP_MUL,   "MUL"   },
	{ OP_IMUL,  "IMUL"  },
	{ OP_DIV,   "DIV"   },
	{ OP_IDIV,  "IDIV"  },
	{ OP_INC,   "INC"   },
	{ OP_DEC,   "DEC"   },
	{ OP_ROL,   "ROL"   },
	{ OP_ROR,   "ROR"   },
	{ OP_RCL,   "RCL"   },
	{ OP_RCR,   "RCR"   },
	{ OP_SHL,   "SHL"   },
	{ OP_SHR,   "SHR"   },
	{ OP_SAL,   "SAL"   },
	{ OP_SAR,   "SAR"   },
    { OP_PUSH,  "PUSH"  },
    { OP_POP,   "POP"   },
    { OP_PUSHF, "PUSHF" },
    { OP_POPF,  "POPF"  },
    { OP_JMP,   "JMP"   },
    { OP_JMPF,  "JMPF"  },
    { OP_CALL,  "CALL"  },
    { OP_CALLF, "CALLF" },
    { OP_INT,   "INT"   },
    { OP_INT3,  "INT3"  },
    { OP_INTO,  "INTO"  },
    { OP_RET,   "RET"   },
    { OP_RETF,  "RETF"  },
    { OP_IRET,  "IRET"  },
    { OP_JO,    "JO"    },
    { OP_JNO,   "JNO"   },
    { OP_JB,    "JB"    },
    { OP_JNB,   "JNB"   },
    { OP_JZ,    "JZ"    },
   	{ OP_JNZ,   "JNZ"   },
	{ OP_JNA,   "JNA"   },
	{ OP_JA,    "JA"    },
	{ OP_JS,    "JS"    },
	{ OP_JNS,   "JNS"   },
	{ OP_JP,    "JP"    },
	{ OP_JNP,   "JNP"   },
	{ OP_JL,    "JL"    },
	{ OP_JNL,   "JNL"   },
	{ OP_JG,    "JG"    },
	{ OP_JNG,   "JNG"   },
	{ OP_MOVSB, "MOVSB" },
	{ OP_MOVSW, "MOVSW" },
	{ OP_CMPSB, "CMPSB" },
	{ OP_CMPSW, "CMPSW" },
	{ OP_STOSB, "STOSB" },
	{ OP_STOSW, "STOSW" },
	{ OP_LODSB, "LODSB" },
	{ OP_LODSW, "LODSW" },
	{ OP_SAHF,  "SAHF"  },
	{ OP_SCASB, "SCASB" },
	{ OP_SCASW, "SCASW" },
	{ OP_STC,   "STC"   },
	{ OP_STD,   "STD"   },
	{ OP_STI,   "STI"   },
	{ 0,        NULL    }
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
