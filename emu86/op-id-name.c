
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
	{ OP_ADC,   "ADC"   },
	{ OP_ADD,   "ADD"   },
	{ OP_AND,   "AND"   },
	{ OP_CALL,  "CALL"  },
	{ OP_CALLF, "CALLF" },
	{ OP_CBW,   "CBW"   },
	{ OP_CLC,   "CLC"   },
	{ OP_CLD,   "CLD"   },
	{ OP_CLI,   "CLI"   },
	{ OP_CMP,   "CMP"   },
	{ OP_CMPS,  "CMPS"  },  // TODO: how to display operation W flag ?
	{ OP_DEC,   "DEC"   },
	{ OP_DIV,   "DIV"   },
	{ OP_HLT,   "HLT"   },
	{ OP_IDIV,  "IDIV"  },
	{ OP_IMUL,  "IMUL"  },
	{ OP_IN,    "IN"    },
	{ OP_INC,   "INC"   },
    { OP_INT,   "INT"   },
    { OP_INT3,  "INT3"  },
    { OP_INTO,  "INTO"  },
    { OP_IRET,  "IRET"  },
    { OP_JMP,   "JMP"   },
    { OP_JMPF,  "JMPF"  },
	{ OP_JA,    "JA"    },
    { OP_JB,    "JB"    },
	{ OP_JG,    "JG"    },
	{ OP_JL,    "JL"    },
    { OP_JO,    "JO"    },
	{ OP_JNA,   "JNA"   },
    { OP_JNB,   "JNB"   },
	{ OP_JNG,   "JNG"   },
	{ OP_JNL,   "JNL"   },
    { OP_JNO,   "JNO"   },
	{ OP_JNP,   "JNP"   },
	{ OP_JNS,   "JNS"   },
   	{ OP_JNZ,   "JNZ"   },
	{ OP_JP,    "JP"    },
	{ OP_JS,    "JS"    },
    { OP_JZ,    "JZ"    },
	{ OP_LAHF,  "LAHF"  },
	{ OP_LEA,   "LEA"   },
	{ OP_LDS,   "LDS"   },
	{ OP_LES,   "LES"   },
	{ OP_LODS,  "LODS"  },
	{ OP_LOOP,   "LOOP"   },
	{ OP_LOOPNZ, "LOOPNZ" },
	{ OP_LOOPZ,  "LOOPZ"  },
	{ OP_MOV,   "MOV"   },
	{ OP_MOVS,  "MOVS" },
	{ OP_MUL,   "MUL"   },
	{ OP_NEG,   "NEG"   },
	{ OP_NOP,   "NOP"   },
	{ OP_NOT,   "NOT"   },
	{ OP_OR,    "OR"    },
	{ OP_OUT,   "OUT"   },
    { OP_POP,   "POP"   },
    { OP_POPA,  "POPA"   },
    { OP_POPF,  "POPF"  },
    { OP_PUSH,  "PUSH"  },
    { OP_PUSHA, "PUSHA" },
    { OP_PUSHF, "PUSHF" },
	{ OP_RCL,   "RCL"   },
	{ OP_RCR,   "RCR"   },
	{ OP_REPNZ, "REPNZ" },
	{ OP_REPZ,  "REPZ"  },
    { OP_RET,   "RET"   },
	{ OP_ROL,   "ROL"   },
	{ OP_ROR,   "ROR"   },
    { OP_RETF,  "RETF"  },
	{ OP_SAHF,  "SAHF"  },
	{ OP_SAL,   "SAL"   },
	{ OP_SAR,   "SAR"   },
	{ OP_SBB,   "SBB"   },
	{ OP_SCAS,  "SCAS" },
	{ OP_SEG,   "SEG"   },
	{ OP_SHL,   "SHL"   },
	{ OP_SHR,   "SHR"   },
	{ OP_STC,   "STC"   },
	{ OP_STD,   "STD"   },
	{ OP_STI,   "STI"   },
	{ OP_STOS,  "STOS" },
	{ OP_SUB,   "SUB"   },
	{ OP_TEST,  "TEST"  },
	{ OP_XCHG,  "XCHG"  },
	{ OP_XOR,   "XOR"   },
	{ OP_NULL,  NULL    }
	};


char *op_id_to_name (word_t op_id)
	{
	char *name = NULL;
	op_id_name_t *op = id_name_tab;
	word_t id;

	while ((id = op->id) != OP_NULL)
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
