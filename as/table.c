/* table.c - keyword tables and symbol table lookup for assembler */

#include "const.h"
#include "type.h"
#include "globvar.h"
#include "opcode.h"
#include "scan.h"

#ifdef STDC_HEADERS_MISSING
int memcmp P((const void *s1, const void *s2, unsigned n));
#else
#undef NULL
#include <string.h>
#endif

#define hconv(ch) ((unsigned char) (ch) - 0x41)	/* better form for hashing */

#ifdef DEBUG
unsigned nhash;
unsigned nlookup;
unsigned nsym;
unsigned nx[30];
FORWARD void printchain P((void));
#endif

/* --- start of keywords --- */

/* registers */
/* the register code (internal to assembler) is given in 1 byte */
/* the "opcode" field is not used */

PUBLIC char regs[] =
{
#ifdef I80386
    2, 'B', 'P', BPREG, 0,
    2, 'B', 'X', BXREG, 0,
    2, 'D', 'I', DIREG, 0,
    2, 'S', 'I', SIREG, 0,

    3, 'E', 'A', 'X', EAXREG, 0,
    3, 'E', 'B', 'P', EBPREG, 0,
    3, 'E', 'B', 'X', EBXREG, 0,
    3, 'E', 'C', 'X', ECXREG, 0,
    3, 'E', 'D', 'I', EDIREG, 0,
    3, 'E', 'D', 'X', EDXREG, 0,
    3, 'E', 'S', 'I', ESIREG, 0,
    3, 'E', 'S', 'P', ESPREG, 0,

    2, 'A', 'X', AXREG, 0,
    2, 'C', 'X', CXREG, 0,
    2, 'D', 'X', DXREG, 0,
    2, 'S', 'P', SPREG, 0,

    2, 'A', 'H', AHREG, 0,
    2, 'A', 'L', ALREG, 0,
    2, 'B', 'H', BHREG, 0,
    2, 'B', 'L', BLREG, 0,
    2, 'C', 'H', CHREG, 0,
    2, 'C', 'L', CLREG, 0,
    2, 'D', 'H', DHREG, 0,
    2, 'D', 'L', DLREG, 0,

    2, 'C', 'S', CSREG, 0,
    2, 'D', 'S', DSREG, 0,
    2, 'E', 'S', ESREG, 0,
    2, 'F', 'S', FSREG, 0,
    2, 'G', 'S', GSREG, 0,
    2, 'S', 'S', SSREG, 0,

    3, 'C', 'R', '0', CR0REG, 0,
    3, 'C', 'R', '2', CR2REG, 0,
    3, 'C', 'R', '3', CR3REG, 0,
    3, 'D', 'R', '0', DR0REG, 0,
    3, 'D', 'R', '1', DR1REG, 0,
    3, 'D', 'R', '2', DR2REG, 0,
    3, 'D', 'R', '3', DR3REG, 0,
    3, 'D', 'R', '6', DR6REG, 0,
    3, 'D', 'R', '7', DR7REG, 0,
    3, 'T', 'R', '3', TR3REG, 0,
    3, 'T', 'R', '4', TR4REG, 0,
    3, 'T', 'R', '5', TR5REG, 0,
    3, 'T', 'R', '6', TR6REG, 0,
    3, 'T', 'R', '7', TR7REG, 0,

    2, 'S', 'T', ST0REG, 0,
#endif /* I80386 */

#ifdef MC6809
    1, 'A', AREG, 0,
    1, 'B', BREG, 0,
    2, 'C', 'C', CCREG, 0,
    1, 'D', DREG, 0,
    2, 'D', 'P', DPREG, 0,
    2, 'P', 'C', PCREG, 0,
    3, 'P', 'C', 'R', PCREG, 0,
    1, 'S', SREG, 0,
    1, 'U', UREG, 0,
    1, 'X', XREG, 0,
    1, 'Y', YREG, 0,
#endif
    0				/* end of register list */
};

#ifdef I80386

/* type sizes */
/* the "opcode" field gives the type size */

PUBLIC char typesizes[] =
{
    4, 'B', 'Y', 'T', 'E', BYTEOP, 1,
    5, 'D', 'W', 'O', 'R', 'D', DWORDOP, 4,
    5, 'F', 'W', 'O', 'R', 'D', FWORDOP, 6,
    3, 'F', 'A', 'R', FAROP, 0,
    3, 'P', 'T', 'R', PTROP, 0,
    5, 'P', 'W', 'O', 'R', 'D', PWORDOP, 6,
    5, 'Q', 'W', 'O', 'R', 'D', QWORDOP, 8,
    5, 'T', 'B', 'Y', 'T', 'E', TBYTEOP, 10,
    4, 'W', 'O', 'R', 'D', WORDOP, 2,
    0				/* end of typesize list */
};

#endif

/* ops */
/* the routine number is given in 1 byte */
/* the opcode is given in 1 byte (it is not used for pseudo-ops) */

PUBLIC char ops[] =
{
    /* pseudo-ops. The "opcode" field is unused and padded with a null byte */
    /* conditionals - must be first */
    4, 'E', 'L', 'S', 'E', ELSEOP, 0,
    6, 'E', 'L', 'S', 'E', 'I', 'F', ELSEIFOP, 0,
    7, 'E', 'L', 'S', 'E', 'I', 'F', 'C', ELSEIFCOP, 0,
    5, 'E', 'N', 'D', 'I', 'F', ENDIFOP, 0,
    2, 'I', 'F', IFOP, 0,
    3, 'I', 'F', 'C', IFCOP, 0,

    /* unconditionals */
    6, '.', 'A', 'L', 'I', 'G', 'N', ALIGNOP, 0,
    6, '.', 'A', 'S', 'C', 'I', 'I', FCCOP, 0,
    6, '.', 'A', 'S', 'C', 'I', 'Z', ASCIZOP, 0,
    5, '.', 'B', 'L', 'K', 'B', RMBOP, 0,
    5, '.', 'B', 'L', 'K', 'W', BLKWOP, 0,
    5, 'B', 'L', 'O', 'C', 'K', BLOCKOP, 0,
    4, '.', 'B', 'S', 'S', BSSOP, 0,
    5, '.', 'B', 'Y', 'T', 'E', FCBOP, 0,
    4, 'C', 'O', 'M', 'M', COMMOP, 0,
    5, '.', 'C', 'O', 'M', 'M', COMMOP1, 0,
    5, '.', 'D', 'A', 'T', 'A', DATAOP, 0,
    6, '.', 'D', 'A', 'T', 'A', '1', FCBOP, 0,
    6, '.', 'D', 'A', 'T', 'A', '2', FDBOP, 0,
    6, '.', 'D', 'A', 'T', 'A', '4', FQBOP, 0,
    2, 'D', 'B', FCBOP, 0,
    2, 'D', 'D', FQBOP, 0,
    7, '.', 'D', 'E', 'F', 'I', 'N', 'E', EXPORTOP, 0,
    2, 'D', 'W', FDBOP, 0,
    3, 'E', 'N', 'D', PROCEOFOP, 0,
    4, 'E', 'N', 'D', 'B', ENDBOP, 0,
    5, 'E', 'N', 'T', 'E', 'R', ENTEROP, 0,
    5, 'E', 'N', 'T', 'R', 'Y', ENTRYOP, 0,
    3, 'E', 'Q', 'U', EQUOP, 0,
    5, '.', 'E', 'V', 'E', 'N', EVENOP, 0,
    6, 'E', 'X', 'P', 'O', 'R', 'T', EXPORTOP, 0,
    6, 'E', 'X', 'T', 'E', 'R', 'N', IMPORTOP, 0,
    7, '.', 'E', 'X', 'T', 'E', 'R', 'N', IMPORTOP, 0,
    5, 'E', 'X', 'T', 'R', 'N', IMPORTOP, 0,
    4, 'F', 'A', 'I', 'L', FAILOP, 0,
    5, '.', 'F', 'A', 'I', 'L', FAILOP, 0,
    3, 'F', 'C', 'B', FCBOP, 0,
    3, 'F', 'C', 'C', FCCOP, 0,
    3, 'F', 'D', 'B', FDBOP, 0,
    3, 'G', 'E', 'T', GETOP, 0,
    6, '.', 'G', 'L', 'O', 'B', 'L', GLOBLOP, 0,
    5, 'I', 'D', 'E', 'N', 'T', IDENTOP, 0,
    6, 'I', 'M', 'P', 'O', 'R', 'T', IMPORTOP, 0,
    7, 'I', 'N', 'C', 'L', 'U', 'D', 'E', GETOP, 0,
    5, 'L', 'C', 'O', 'M', 'M', LCOMMOP, 0,
    6, '.', 'L', 'C', 'O', 'M', 'M', LCOMMOP1, 0,
    5, '.', 'L', 'I', 'S', 'T', LISTOP, 0,
    3, 'L', 'O', 'C', LOCOP, 0,
    5, '.', 'L', 'O', 'N', 'G', FQBOP, 0,
    8, '.', 'M', 'A', 'C', 'L', 'I', 'S', 'T', MACLISTOP, 0,
    5, 'M', 'A', 'C', 'R', 'O', MACROOP, 0,
    4, '.', 'M', 'A', 'P', MAPOP, 0,
    3, 'O', 'R', 'G', ORGOP, 0,
    4, '.', 'O', 'R', 'G', ORGOP, 0,
    6, 'P', 'U', 'B', 'L', 'I', 'C', EXPORTOP, 0,
    3, 'R', 'M', 'B', RMBOP, 0,
    4, '.', 'R', 'O', 'M', DATAOP, 0,
    5, '.', 'S', 'E', 'C', 'T', SECTOP, 0,
    3, 'S', 'E', 'T', SETOP, 0,
    5, 'S', 'E', 'T', 'D', 'P', SETDPOP, 0,
    6, '.', 'S', 'H', 'O', 'R', 'T', FDBOP, 0,
    6, '.', 'S', 'P', 'A', 'C', 'E', RMBOP, 0,
    5, '.', 'T', 'E', 'X', 'T', TEXTOP, 0,
    5, 'U', 'S', 'E', '1', '6', USE16OP, 0,
    5, 'U', 'S', 'E', '3', '2', USE32OP, 0,
    5, '.', 'W', 'A', 'R', 'N', WARNOP, 0,
    5, '.', 'W', 'O', 'R', 'D', FDBOP, 0,
    6, '.', 'Z', 'E', 'R', 'O', 'W', BLKWOP, 0,

    /* hardware ops. The opcode field is now used */
#ifdef I80386
    3, 'A', 'A', 'A', INHER, 0x37,
    3, 'A', 'A', 'D', INHER_A, 0xD5,
    3, 'A', 'A', 'M', INHER_A, 0xD4,
    3, 'A', 'A', 'S', INHER, 0x3F,
    3, 'A', 'D', 'C', GROUP1, 0x10,
    3, 'A', 'D', 'D', GROUP1, 0x00,
    3, 'A', 'N', 'D', GROUP1, 0x20,
    4, 'A', 'R', 'P', 'L', EwGw, 0x63,
    3, 'B', 'C', 'C', BCC, 0x73,
    3, 'B', 'C', 'S', BCC, 0x72,
    3, 'B', 'E', 'Q', BCC, 0x74,
    3, 'B', 'G', 'E', BCC, 0x7D,
    3, 'B', 'G', 'T', BCC, 0x7F,
    3, 'B', 'H', 'I', BCC, 0x77,
    4, 'B', 'H', 'I', 'S', BCC, 0x73,
    3, 'B', 'L', 'E', BCC, 0x7E,
    3, 'B', 'L', 'O', BCC, 0x72,
    4, 'B', 'L', 'O', 'S', BCC, 0x76,
    3, 'B', 'L', 'T', BCC, 0x7C,
    3, 'B', 'M', 'I', BCC, 0x78,
    3, 'B', 'N', 'E', BCC, 0x75,
    5, 'B', 'O', 'U', 'N', 'D', GvMa, 0x62,
    3, 'B', 'P', 'C', BCC, 0x7B,
    3, 'B', 'P', 'L', BCC, 0x79,
    3, 'B', 'P', 'S', BCC, 0x7A,
    2, 'B', 'R', CALL, JMP_OPCODE,
    3, 'B', 'V', 'C', BCC, 0x71,
    3, 'B', 'V', 'S', BCC, 0x70,
    4, 'C', 'A', 'L', 'L', CALL, JSR_OPCODE,
    5, 'C', 'A', 'L', 'L', 'F', CALLI, 0x9A,
    5, 'C', 'A', 'L', 'L', 'I', CALLI, 0x9A,
    3, 'C', 'B', 'W', INHER16, 0x98,
    3, 'C', 'L', 'C', INHER, 0xF8,
    3, 'C', 'L', 'D', INHER, 0xFC,
    3, 'C', 'L', 'I', INHER, 0xFA,
    3, 'C', 'M', 'C', INHER, 0xF5,
    3, 'C', 'M', 'P', GROUP1, CMP_OPCODE_BASE,
    4, 'C', 'M', 'P', 'S', INHER, CMPSW_OPCODE,
    5, 'C', 'M', 'P', 'S', 'B', INHER, CMPSB_OPCODE,
    5, 'C', 'M', 'P', 'S', 'D', INHER32, CMPSW_OPCODE,
    5, 'C', 'M', 'P', 'S', 'W', INHER16, CMPSW_OPCODE,
    4, 'C', 'M', 'P', 'W', INHER16, CMPSW_OPCODE,
    4, 'C', 'S', 'E', 'G', INHER, 0x2E,
    3, 'C', 'W', 'D', INHER16, 0x99,
    4, 'C', 'W', 'D', 'E', INHER32, 0x98,
    3, 'C', 'D', 'Q', INHER32, 0x99,
    3, 'D', 'A', 'A', INHER, 0x27,
    3, 'D', 'A', 'S', INHER, 0x2F,
    4, 'D', 'S', 'E', 'G', INHER, 0x3E,
    3, 'D', 'E', 'C', INCDEC, 0x08,
    3, 'D', 'I', 'V', DIVMUL, 0x30,
    5, 'E', 'N', 'T', 'E', 'R', ENTER, 0xC8,
    4, 'E', 'S', 'E', 'G', INHER, 0x26,
    4, 'F', 'S', 'E', 'G', INHER, 0x64,
    4, 'G', 'S', 'E', 'G', INHER, 0x65,
    3, 'H', 'L', 'T', INHER, 0xF4,
    4, 'I', 'D', 'I', 'V', DIVMUL, 0x38,
    4, 'I', 'M', 'U', 'L', IMUL, 0x28,
    2, 'I', 'N', IN, 0xEC,
    3, 'I', 'N', 'C', INCDEC, 0x00,
    3, 'I', 'N', 'S', INHER, 0x6D,
    4, 'I', 'N', 'S', 'B', INHER, 0x6C,
    4, 'I', 'N', 'S', 'D', INHER32, 0x6D,
    4, 'I', 'N', 'S', 'W', INHER16, 0x6D,
    3, 'I', 'N', 'T', INT, 0xCD,
    4, 'I', 'N', 'T', 'O', INHER, 0xCE,
    3, 'I', 'N', 'W', IN, 0xED,
    4, 'I', 'R', 'E', 'T', INHER16, 0xCF,
    5, 'I', 'R', 'E', 'T', 'D', INHER32, 0xCF,
    1, 'J', CALL, JMP_SHORT_OPCODE,
    2, 'J', 'A', JCC, 0x77,
    3, 'J', 'A', 'E', JCC, 0x73,
    2, 'J', 'B', JCC, 0x72,
    3, 'J', 'B', 'E', JCC, 0x76,
    2, 'J', 'C', JCC, 0x72,
    4, 'J', 'C', 'X', 'E', JCXZ, 0x2,
    4, 'J', 'C', 'X', 'Z', JCXZ, 0x2,
    5, 'J', 'E', 'C', 'X', 'E', JCXZ, 0x4,
    5, 'J', 'E', 'C', 'X', 'Z', JCXZ, 0x4,
    2, 'J', 'E', JCC, 0x74,
    2, 'J', 'G', JCC, 0x7F,
    3, 'J', 'G', 'E', JCC, 0x7D,
    2, 'J', 'L', JCC, 0x7C,
    3, 'J', 'L', 'E', JCC, 0x7E,
    3, 'J', 'M', 'P', CALL, JMP_SHORT_OPCODE,
    4, 'J', 'M', 'P', 'F', CALLI, 0xEA,
    4, 'J', 'M', 'P', 'I', CALLI, 0xEA,
    3, 'J', 'N', 'A', JCC, 0x76,
    4, 'J', 'N', 'A', 'E', JCC, 0x72,
    3, 'J', 'N', 'B', JCC, 0x73,
    4, 'J', 'N', 'B', 'E', JCC, 0x77,
    3, 'J', 'N', 'C', JCC, 0x73,
    3, 'J', 'N', 'E', JCC, 0x75,
    3, 'J', 'N', 'G', JCC, 0x7E,
    4, 'J', 'N', 'G', 'E', JCC, 0x7C,
    3, 'J', 'N', 'L', JCC, 0x7D,
    4, 'J', 'N', 'L', 'E', JCC, 0x7F,
    3, 'J', 'N', 'O', JCC, 0x71,
    3, 'J', 'N', 'P', JCC, 0x7B,
    3, 'J', 'N', 'S', JCC, 0x79,
    3, 'J', 'N', 'Z', JCC, 0x75,
    2, 'J', 'O', JCC, 0x70,
    2, 'J', 'P', JCC, 0x7A,
    3, 'J', 'P', 'E', JCC, 0x7A,
    3, 'J', 'P', 'O', JCC, 0x7B,
    2, 'J', 'S', JCC, 0x78,
    2, 'J', 'Z', JCC, 0x74,
    4, 'L', 'A', 'H', 'F', INHER, 0x9F,
    3, 'L', 'D', 'S', GvMp, 0xC5,
    3, 'L', 'E', 'A', LEA, 0x8D,
    5, 'L', 'E', 'A', 'V', 'E', INHER, 0xC9,
    3, 'L', 'E', 'S', GvMp, 0xC4,
    4, 'L', 'O', 'C', 'K', INHER, 0xF0,
    4, 'L', 'O', 'D', 'B', INHER, 0xAC,
    4, 'L', 'O', 'D', 'S', INHER, 0xAD,
    5, 'L', 'O', 'D', 'S', 'B', INHER, 0xAC,
    5, 'L', 'O', 'D', 'S', 'D', INHER32, 0xAD,
    5, 'L', 'O', 'D', 'S', 'W', INHER16, 0xAD,
    4, 'L', 'O', 'D', 'W', INHER16, 0xAD,
    4, 'L', 'O', 'O', 'P', JCC, 0xE2,
    5, 'L', 'O', 'O', 'P', 'E', JCC, 0xE1,
    6, 'L', 'O', 'O', 'P', 'N', 'E', JCC, 0xE0,
    6, 'L', 'O', 'O', 'P', 'N', 'Z', JCC, 0xE0,
    5, 'L', 'O', 'O', 'P', 'Z', JCC, 0xE1,
    3, 'M', 'O', 'V', MOV, 0x88,
    4, 'M', 'O', 'V', 'S', INHER, MOVSW_OPCODE,
    5, 'M', 'O', 'V', 'S', 'B', INHER, MOVSB_OPCODE,
    5, 'M', 'O', 'V', 'S', 'D', INHER32, MOVSW_OPCODE,
    5, 'M', 'O', 'V', 'S', 'W', INHER16, MOVSW_OPCODE,
    4, 'M', 'O', 'V', 'W', INHER16, MOVSW_OPCODE,
    3, 'M', 'U', 'L', DIVMUL, 0x20,
    3, 'N', 'E', 'G', NEGNOT, 0x18,
    3, 'N', 'O', 'P', INHER, 0x90,
    3, 'N', 'O', 'T', NEGNOT, 0x10,
    2, 'O', 'R', GROUP1, 0x08,
    3, 'O', 'U', 'T', OUT, 0xEE,
    4, 'O', 'U', 'T', 'S', INHER, 0x6F,
    5, 'O', 'U', 'T', 'S', 'B', INHER, 0x6E,
    5, 'O', 'U', 'T', 'S', 'D', INHER32, 0x6F,
    5, 'O', 'U', 'T', 'S', 'W', INHER16, 0x6F,
    4, 'O', 'U', 'T', 'W', OUT, 0xEF,
    3, 'P', 'O', 'P', PUSHPOP, POP_OPCODE,
    4, 'P', 'O', 'P', 'A', INHER16, 0x61,
    5, 'P', 'O', 'P', 'A', 'D', INHER32, 0x61,
    4, 'P', 'O', 'P', 'F', INHER16, 0x9D,
    5, 'P', 'O', 'P', 'F', 'D', INHER32, 0x9D,
    4, 'P', 'U', 'S', 'H', PUSHPOP, PUSH_OPCODE,
    5, 'P', 'U', 'S', 'H', 'A', INHER16, 0x60,
    6, 'P', 'U', 'S', 'H', 'A', 'D', INHER32, 0x60,
    5, 'P', 'U', 'S', 'H', 'F', INHER16, 0x9C,
    6, 'P', 'U', 'S', 'H', 'F', 'D', INHER32, 0x9C,
    3, 'R', 'C', 'L', GROUP2, 0x10,
    3, 'R', 'C', 'R', GROUP2, 0x18,
    3, 'R', 'O', 'L', GROUP2, 0x00,
    3, 'R', 'O', 'R', GROUP2, 0x08,
    3, 'R', 'E', 'P', INHER, 0xF3,
    4, 'R', 'E', 'P', 'E', INHER, 0xF3,
    5, 'R', 'E', 'P', 'N', 'E', INHER, 0xF2,
    5, 'R', 'E', 'P', 'N', 'Z', INHER, 0xF2,
    4, 'R', 'E', 'P', 'Z', INHER, 0xF3,
    3, 'R', 'E', 'T', RET, 0xC3,
    4, 'R', 'E', 'T', 'F', RET, 0xCB,
    4, 'R', 'E', 'T', 'I', RET, 0xCB,
    4, 'S', 'A', 'H', 'F', INHER, 0x9E,
    3, 'S', 'A', 'L', GROUP2, 0x20,
    3, 'S', 'A', 'R', GROUP2, 0x38,
    3, 'S', 'B', 'B', GROUP1, 0x18,
    4, 'S', 'C', 'A', 'B', INHER, 0xAE,
    4, 'S', 'C', 'A', 'S', INHER, 0xAF,
    5, 'S', 'C', 'A', 'S', 'B', INHER, 0xAE,
    5, 'S', 'C', 'A', 'S', 'D', INHER32, 0xAF,
    5, 'S', 'C', 'A', 'S', 'W', INHER16, 0xAF,
    4, 'S', 'C', 'A', 'W', INHER16, 0xAF,
    3, 'S', 'E', 'G', SEG, 0x06,
    3, 'S', 'H', 'L', GROUP2, 0x20,
    3, 'S', 'H', 'R', GROUP2, 0x28,
    4, 'S', 'S', 'E', 'G', INHER, 0x36,
    3, 'S', 'T', 'C', INHER, 0xF9,
    3, 'S', 'T', 'D', INHER, 0xFD,
    3, 'S', 'T', 'I', INHER, 0xFB,
    4, 'S', 'T', 'O', 'B', INHER, 0xAA,
    4, 'S', 'T', 'O', 'S', INHER, 0xAB,
    5, 'S', 'T', 'O', 'S', 'B', INHER, 0xAA,
    5, 'S', 'T', 'O', 'S', 'D', INHER32, 0xAB,
    5, 'S', 'T', 'O', 'S', 'W', INHER16, 0xAB,
    4, 'S', 'T', 'O', 'W', INHER16, 0xAB,
    3, 'S', 'U', 'B', GROUP1, 0x28,
    4, 'T', 'E', 'S', 'T', TEST, 0x84,
    4, 'W', 'A', 'I', 'T', INHER, WAIT_OPCODE,
    4, 'X', 'C', 'H', 'G', XCHG, 0x86,
    4, 'X', 'L', 'A', 'T', INHER, 0xD7,
    5, 'X', 'L', 'A', 'T', 'B', INHER, 0xD7,
    3, 'X', 'O', 'R', GROUP1, 0x30,

    /* floating point */
    5, 'F', '2', 'X', 'M', '1', F_INHER, 0x70,
    4, 'F', 'A', 'B', 'S', F_INHER, 0x61,
    4, 'F', 'A', 'D', 'D', F_M4_M8_STST, 0x00,
    5, 'F', 'A', 'D', 'D', 'P', F_STST, 0x60,
    4, 'F', 'B', 'L', 'D', F_M10, 0x74,
    5, 'F', 'B', 'S', 'T', 'P', F_M10, 0x76,
    4, 'F', 'C', 'H', 'S', F_INHER, 0x60,
    5, 'F', 'C', 'L', 'E', 'X', F_W_INHER, 0xE2,
    4, 'F', 'C', 'O', 'M', F_M4_M8_OPTST, 0x02,
    5, 'F', 'C', 'O', 'M', 'P', F_M4_M8_OPTST, 0x03,
    6, 'F', 'C', 'O', 'M', 'P', 'P', F_INHER, 0x19,
    4, 'F', 'C', 'O', 'S', F_INHER, 0x7F,
    7, 'F', 'D', 'E', 'C', 'S', 'T', 'P', F_INHER, 0x76,
    5, 'F', 'D', 'I', 'S', 'I', F_W_INHER, 0xE1,
    4, 'F', 'D', 'I', 'V', F_M4_M8_STST, 0x06,
    5, 'F', 'D', 'I', 'V', 'P', F_STST, 0x67,
    5, 'F', 'D', 'I', 'V', 'R', F_M4_M8_STST, 0x07,
    6, 'F', 'D', 'I', 'V', 'R', 'P', F_STST, 0x66,
    4, 'F', 'E', 'N', 'I', F_W_INHER, 0xE0,
    5, 'F', 'F', 'R', 'E', 'E', F_ST, 0x50,
    5, 'F', 'I', 'A', 'D', 'D', F_M2_M4, 0x20,
    5, 'F', 'I', 'C', 'O', 'M', F_M2_M4, 0x22,
    6, 'F', 'I', 'C', 'O', 'M', 'P', F_M2_M4, 0x23,
    5, 'F', 'I', 'D', 'I', 'V', F_M2_M4, 0x26,
    6, 'F', 'I', 'D', 'I', 'V', 'R', F_M2_M4, 0x27,
    4, 'F', 'I', 'L', 'D', F_M2_M4_M8, 0x30,
    5, 'F', 'I', 'M', 'U', 'L', F_M2_M4, 0x21,
    7, 'F', 'I', 'N', 'C', 'S', 'T', 'P', F_INHER, 0x77,
    5, 'F', 'I', 'N', 'I', 'T', F_W_INHER, 0xE3,
    4, 'F', 'I', 'S', 'T', F_M2_M4, 0x32,
    5, 'F', 'I', 'S', 'T', 'P', F_M2_M4_M8, 0x33,
    5, 'F', 'I', 'S', 'U', 'B', F_M2_M4, 0x24,
    6, 'F', 'I', 'S', 'U', 'B', 'R', F_M2_M4, 0x25,
    3, 'F', 'L', 'D', F_M4_M8_M10_ST, 0x10,
    4, 'F', 'L', 'D', '1', F_INHER, 0x68,
    6, 'F', 'L', 'D', 'L', '2', 'E', F_INHER, 0x6A,
    6, 'F', 'L', 'D', 'L', '2', 'T', F_INHER, 0x69,
    5, 'F', 'L', 'D', 'C', 'W', F_M2, 0x15,
    6, 'F', 'L', 'D', 'E', 'N', 'V', F_M, 0x14,
    6, 'F', 'L', 'D', 'L', 'G', '2', F_INHER, 0x6C,
    6, 'F', 'L', 'D', 'L', 'N', '2', F_INHER, 0x6D,
    5, 'F', 'L', 'D', 'P', 'I', F_INHER, 0x6B,
    4, 'F', 'L', 'D', 'Z', F_INHER, 0x6E,
    4, 'F', 'M', 'U', 'L', F_M4_M8_STST, 0x01,
    5, 'F', 'M', 'U', 'L', 'P', F_STST, 0x61,
    6, 'F', 'N', 'C', 'L', 'E', 'X', F_INHER, 0xE2,
    6, 'F', 'N', 'D', 'I', 'S', 'I', F_INHER, 0xE1,
    5, 'F', 'N', 'E', 'N', 'I', F_INHER, 0xE0,
    6, 'F', 'N', 'I', 'N', 'I', 'T', F_INHER, 0xE3,
    4, 'F', 'N', 'O', 'P', F_INHER, 0x50,
    6, 'F', 'N', 'S', 'A', 'V', 'E', F_M, 0x56,
    6, 'F', 'N', 'S', 'T', 'C', 'W', F_M2, 0x17,
    7, 'F', 'N', 'S', 'T', 'E', 'N', 'V', F_M, 0x16,
    6, 'F', 'N', 'S', 'T', 'S', 'W', F_M2_AX, 0x57,
    6, 'F', 'P', 'A', 'T', 'A', 'N', F_INHER, 0x73,
    5, 'F', 'P', 'R', 'E', 'M', F_INHER, 0x78,
    6, 'F', 'P', 'R', 'E', 'M', '1', F_INHER, 0x75,
    5, 'F', 'P', 'T', 'A', 'N', F_INHER, 0x72,
    7, 'F', 'R', 'N', 'D', 'I', 'N', 'T', F_INHER, 0x7C,
    6, 'F', 'R', 'S', 'T', 'O', 'R', F_M, 0x54,
    5, 'F', 'S', 'A', 'V', 'E', F_W_M, 0x56,
    6, 'F', 'S', 'C', 'A', 'L', 'E', F_INHER, 0x7D,
    6, 'F', 'S', 'E', 'T', 'P', 'M', F_INHER, 0xE4,
    4, 'F', 'S', 'I', 'N', F_INHER, 0x7E,
    7, 'F', 'S', 'I', 'N', 'C', 'O', 'S', F_INHER, 0x7B,
    5, 'F', 'S', 'Q', 'R', 'T', F_INHER, 0x7A,
    3, 'F', 'S', 'T', F_M4_M8_ST, FST_ENCODED,
    5, 'F', 'S', 'T', 'C', 'W', F_W_M2, 0x17,
    6, 'F', 'S', 'T', 'E', 'N', 'V', F_W_M, 0x16,
    4, 'F', 'S', 'T', 'P', F_M4_M8_M10_ST, FSTP_ENCODED,
    5, 'F', 'S', 'T', 'S', 'W', F_W_M2_AX, 0x57,
    4, 'F', 'S', 'U', 'B', F_M4_M8_STST, 0x04,
    5, 'F', 'S', 'U', 'B', 'P', F_STST, 0x65,
    5, 'F', 'S', 'U', 'B', 'R', F_M4_M8_STST, 0x05,
    6, 'F', 'S', 'U', 'B', 'R', 'P', F_STST, 0x64,
    4, 'F', 'T', 'S', 'T', F_INHER, 0x64,
    5, 'F', 'U', 'C', 'O', 'M', F_OPTST, 0x54,
    6, 'F', 'U', 'C', 'O', 'M', 'P', F_OPTST, 0x55,
    7, 'F', 'U', 'C', 'O', 'M', 'P', 'P', F_INHER, 0xA9,
    5, 'F', 'W', 'A', 'I', 'T', INHER, WAIT_OPCODE,
    4, 'F', 'X', 'A', 'M', F_INHER, 0x65,
    4, 'F', 'X', 'C', 'H', F_OPTST, 0x11,
    7, 'F', 'X', 'T', 'R', 'A', 'C', 'T', F_INHER, 0x74,
    5, 'F', 'Y', 'L', '2', 'X', F_INHER, 0x71,
    7, 'F', 'Y', 'L', '2', 'X', 'P', '1', F_INHER, 0x79,
#endif /* I80386 */

#ifdef MC6809
    3, 'A', 'B', 'X', INHER, 0x3A,
    4, 'A', 'D', 'C', 'A', ALL, 0x89,
    4, 'A', 'D', 'C', 'B', ALL, 0xC9,
    4, 'A', 'D', 'D', 'A', ALL, 0x8B,
    4, 'A', 'D', 'D', 'B', ALL, 0xCB,
    4, 'A', 'D', 'D', 'D', ALL, 0xC3,
    4, 'A', 'N', 'D', 'A', ALL, 0x84,
    4, 'A', 'N', 'D', 'B', ALL, 0xC4,
    5, 'A', 'N', 'D', 'C', 'C', IMMED, 0x1C,
    3, 'A', 'S', 'L', ALTER, 0x08,
    4, 'A', 'S', 'L', 'A', INHER, 0x48,
    4, 'A', 'S', 'L', 'B', INHER, 0x58,
    3, 'A', 'S', 'R', ALTER, 0x07,
    4, 'A', 'S', 'R', 'A', INHER, 0x47,
    4, 'A', 'S', 'R', 'B', INHER, 0x57,
    3, 'B', 'C', 'C', SHORT, 0x24,
    3, 'B', 'C', 'S', SHORT, 0x25,
    3, 'B', 'E', 'Q', SHORT, 0x27,
    3, 'B', 'G', 'E', SHORT, 0x2C,
    3, 'B', 'G', 'T', SHORT, 0x2E,
    3, 'B', 'H', 'I', SHORT, 0x22,
    3, 'B', 'H', 'S', SHORT, 0x24,
    4, 'B', 'I', 'T', 'A', ALL, 0X85,
    4, 'B', 'I', 'T', 'B', ALL, 0XC5,
    3, 'B', 'L', 'E', SHORT, 0x2F,
    3, 'B', 'L', 'O', SHORT, 0x25,
    3, 'B', 'L', 'S', SHORT, 0x23,
    3, 'B', 'L', 'T', SHORT, 0x2D,
    3, 'B', 'M', 'I', SHORT, 0x2B,
    3, 'B', 'N', 'E', SHORT, 0x26,
    3, 'B', 'P', 'L', SHORT, 0x2A,
    3, 'B', 'R', 'A', SHORT, 0x20,
    4, 'L', 'B', 'R', 'A', LONG, 0x16,
    3, 'B', 'R', 'N', SHORT, 0x21,
    3, 'B', 'S', 'R', SHORT, 0x8D,
    4, 'L', 'B', 'S', 'R', LONG, 0x17,
    3, 'B', 'V', 'C', SHORT, 0x28,
    3, 'B', 'V', 'S', SHORT, 0x29,
    3, 'C', 'L', 'R', ALTER, 0x0F,
    4, 'C', 'L', 'R', 'A', INHER, 0x4F,
    4, 'C', 'L', 'R', 'B', INHER, 0x5F,
    4, 'C', 'M', 'P', 'A', ALL, 0x81,
    4, 'C', 'M', 'P', 'B', ALL, 0xC1,
    4, 'C', 'M', 'P', 'X', ALL, 0x8C,
    3, 'C', 'O', 'M', ALTER, 0x03,
    4, 'C', 'O', 'M', 'A', INHER, 0x43,
    4, 'C', 'O', 'M', 'B', INHER, 0x53,
    4, 'C', 'W', 'A', 'I', IMMED, 0x3C,
    3, 'D', 'A', 'A', INHER, 0x19,
    3, 'D', 'E', 'C', ALTER, 0x0A,
    4, 'D', 'E', 'C', 'A', INHER, 0x4A,
    4, 'D', 'E', 'C', 'B', INHER, 0x5A,
    4, 'E', 'O', 'R', 'A', ALL, 0x88,
    4, 'E', 'O', 'R', 'B', ALL, 0xC8,
    3, 'E', 'X', 'G', SWAP, 0x1E,
    3, 'I', 'N', 'C', ALTER, 0x0C,
    4, 'I', 'N', 'C', 'A', INHER, 0x4C,
    4, 'I', 'N', 'C', 'B', INHER, 0x5C,
    3, 'J', 'M', 'P', ALTER, 0x0E,
    3, 'J', 'S', 'R', ALTER, 0x8D,
    3, 'L', 'D', 'A', ALL, 0x86,
    3, 'L', 'D', 'B', ALL, 0xC6,
    3, 'L', 'D', 'D', ALL, 0xCC,
    3, 'L', 'D', 'U', ALL, 0xCE,
    3, 'L', 'D', 'X', ALL, 0x8E,
    4, 'L', 'E', 'A', 'S', INDEXD, 0x32,
    4, 'L', 'E', 'A', 'U', INDEXD, 0x33,
    4, 'L', 'E', 'A', 'X', INDEXD, 0x30,
    4, 'L', 'E', 'A', 'Y', INDEXD, 0x31,
    3, 'L', 'S', 'L', ALTER, 0x08,
    4, 'L', 'S', 'L', 'A', INHER, 0x48,
    4, 'L', 'S', 'L', 'B', INHER, 0x58,
    3, 'L', 'S', 'R', ALTER, 0x04,
    4, 'L', 'S', 'R', 'A', INHER, 0x44,
    4, 'L', 'S', 'R', 'B', INHER, 0x54,
    3, 'M', 'U', 'L', INHER, 0x3D,
    3, 'N', 'E', 'G', ALTER, 0x00,
    4, 'N', 'E', 'G', 'A', INHER, 0x40,
    4, 'N', 'E', 'G', 'B', INHER, 0x50,
    3, 'N', 'O', 'P', INHER, 0x12,
    3, 'O', 'R', 'A', ALL, 0x8A,
    3, 'O', 'R', 'B', ALL, 0xCA,
    4, 'O', 'R', 'C', 'C', IMMED, 0x1A,
    4, 'P', 'S', 'H', 'S', SSTAK, 0x34,
    4, 'P', 'S', 'H', 'U', USTAK, 0x36,
    4, 'P', 'U', 'L', 'S', SSTAK, 0x35,
    4, 'P', 'U', 'L', 'U', USTAK, 0x37,
    3, 'R', 'O', 'L', ALTER, 0x09,
    4, 'R', 'O', 'L', 'A', INHER, 0x49,
    4, 'R', 'O', 'L', 'B', INHER, 0x59,
    3, 'R', 'O', 'R', ALTER, 0x06,
    4, 'R', 'O', 'R', 'A', INHER, 0x46,
    4, 'R', 'O', 'R', 'B', INHER, 0x56,
    3, 'R', 'T', 'I', INHER, 0x3B,
    3, 'R', 'T', 'S', INHER, 0x39,
    4, 'S', 'B', 'C', 'A', ALL, 0x82,
    4, 'S', 'B', 'C', 'B', ALL, 0xC2,
    3, 'S', 'E', 'X', INHER, 0x1D,
    3, 'S', 'T', 'A', ALTER, 0x87,
    3, 'S', 'T', 'B', ALTER, 0xC7,
    3, 'S', 'T', 'D', ALTER, 0xCD,
    3, 'S', 'T', 'U', ALTER, 0xCF,
    3, 'S', 'T', 'X', ALTER, 0x8F,
    4, 'S', 'U', 'B', 'A', ALL, 0x80,
    4, 'S', 'U', 'B', 'B', ALL, 0xC0,
    4, 'S', 'U', 'B', 'D', ALL, 0x83,
    3, 'S', 'W', 'I', INHER, 0x3F,
    4, 'S', 'Y', 'N', 'C', INHER, 0x13,
    3, 'T', 'F', 'R', SWAP, 0x1F,
    3, 'T', 'S', 'T', ALTER, 0x0D,
    4, 'T', 'S', 'T', 'A', INHER, 0x4D,
    4, 'T', 'S', 'T', 'B', INHER, 0x5D,
#endif /* MC6809 */
    0				/* end of ops */
};

PUBLIC char page1ops[] =
{
#ifdef I80386
    3, 'B', 'S', 'F', GvEv, 0xBC,
    3, 'B', 'S', 'R', GvEv, 0xBD,
    5, 'B', 'S', 'W', 'A', 'P', BSWAP, 0xC8,
    2, 'B', 'T', GROUP8, 0x20,
    3, 'B', 'T', 'C', GROUP8, 0x38,
    3, 'B', 'T', 'R', GROUP8, 0x30,
    3, 'B', 'T', 'S', GROUP8, 0x28,
    4, 'C', 'L', 'T', 'S', INHER, 0x06,
    7, 'C', 'M', 'P', 'X', 'C', 'H', 'G', ExGx, 0xA6,
    4, 'I', 'N', 'V', 'D', INHER, 0x08,
    6, 'I', 'N', 'V', 'L', 'P', 'G', GROUP7, 0x38,
    3, 'L', 'A', 'R', GvEv, 0x02,
    3, 'L', 'F', 'S', GvMp, 0xB4,
    4, 'L', 'G', 'D', 'T', GROUP7, 0x10,
    3, 'L', 'G', 'S', GvMp, 0xB5,
    4, 'L', 'I', 'D', 'T', GROUP7, 0x18,
    4, 'L', 'L', 'D', 'T', GROUP6, 0x10,
    4, 'L', 'M', 'S', 'W', GROUP7, 0x30,
    3, 'L', 'S', 'L', GvEv, 0x03,
    3, 'L', 'S', 'S', GvMp, 0xB2,
    3, 'L', 'T', 'R', GROUP6, 0x18,
    5, 'M', 'O', 'V', 'S', 'X', MOVX, 0xBE,
    5, 'M', 'O', 'V', 'Z', 'X', MOVX, 0xB6,
    4, 'S', 'E', 'T', 'A', SETCC, 0x97,
    5, 'S', 'E', 'T', 'A', 'E', SETCC, 0x93,
    4, 'S', 'E', 'T', 'B', SETCC, 0x92,
    5, 'S', 'E', 'T', 'B', 'E', SETCC, 0x96,
    4, 'S', 'E', 'T', 'C', SETCC, 0x92,
    4, 'S', 'E', 'T', 'E', SETCC, 0x94,
    4, 'S', 'E', 'T', 'G', SETCC, 0x9F,
    5, 'S', 'E', 'T', 'G', 'E', SETCC, 0x9D,
    4, 'S', 'E', 'T', 'L', SETCC, 0x9C,
    5, 'S', 'E', 'T', 'L', 'E', SETCC, 0x9E,
    5, 'S', 'E', 'T', 'N', 'A', SETCC, 0x96,
    6, 'S', 'E', 'T', 'N', 'A', 'E', SETCC, 0x92,
    5, 'S', 'E', 'T', 'N', 'B', SETCC, 0x93,
    6, 'S', 'E', 'T', 'N', 'B', 'E', SETCC, 0x97,
    5, 'S', 'E', 'T', 'N', 'C', SETCC, 0x93,
    5, 'S', 'E', 'T', 'N', 'E', SETCC, 0x95,
    5, 'S', 'E', 'T', 'N', 'G', SETCC, 0x9E,
    6, 'S', 'E', 'T', 'N', 'G', 'E', SETCC, 0x9C,
    5, 'S', 'E', 'T', 'N', 'L', SETCC, 0x9D,
    6, 'S', 'E', 'T', 'N', 'L', 'E', SETCC, 0x9F,
    5, 'S', 'E', 'T', 'N', 'O', SETCC, 0x91,
    5, 'S', 'E', 'T', 'N', 'P', SETCC, 0x9B,
    5, 'S', 'E', 'T', 'N', 'S', SETCC, 0x99,
    5, 'S', 'E', 'T', 'N', 'Z', SETCC, 0x95,
    4, 'S', 'E', 'T', 'O', SETCC, 0x90,
    4, 'S', 'E', 'T', 'P', SETCC, 0x9A,
    5, 'S', 'E', 'T', 'P', 'E', SETCC, 0x9A,
    5, 'S', 'E', 'T', 'P', 'O', SETCC, 0x9B,
    4, 'S', 'E', 'T', 'S', SETCC, 0x98,
    4, 'S', 'E', 'T', 'Z', SETCC, 0x94,
    4, 'S', 'G', 'D', 'T', GROUP7, 0x00,
    4, 'S', 'I', 'D', 'T', GROUP7, 0x08,
    4, 'S', 'H', 'L', 'D', SH_DOUBLE, 0xA4,
    4, 'S', 'H', 'R', 'D', SH_DOUBLE, 0xAC,
    4, 'S', 'L', 'D', 'T', GROUP6, 0x00,
    4, 'S', 'M', 'S', 'W', GROUP7, 0x20,
    3, 'S', 'T', 'R', GROUP6, 0x08,
    4, 'V', 'E', 'R', 'R', GROUP6, 0x20,
    4, 'V', 'E', 'R', 'W', GROUP6, 0x28,
    6, 'W', 'B', 'I', 'N', 'V', 'D', INHER, 0x09,
    4, 'X', 'A', 'D', 'D', ExGx, 0xC0,
#endif /* I80386 */

#ifdef MC6809
    4, 'L', 'B', 'C', 'C', LONG, 0x24,
    4, 'L', 'B', 'C', 'S', LONG, 0x25,
    4, 'L', 'B', 'E', 'Q', LONG, 0x27,
    4, 'L', 'B', 'G', 'E', LONG, 0x2C,
    4, 'L', 'B', 'G', 'T', LONG, 0x2E,
    4, 'L', 'B', 'H', 'I', LONG, 0x22,
    4, 'L', 'B', 'H', 'S', LONG, 0x24,
    4, 'L', 'B', 'L', 'E', LONG, 0x2F,
    4, 'L', 'B', 'L', 'O', LONG, 0x25,
    4, 'L', 'B', 'L', 'S', LONG, 0x23,
    4, 'L', 'B', 'L', 'T', LONG, 0x2D,
    4, 'L', 'B', 'M', 'I', LONG, 0x2B,
    4, 'L', 'B', 'N', 'E', LONG, 0x26,
    4, 'L', 'B', 'P', 'L', LONG, 0x2A,
    4, 'L', 'B', 'R', 'N', LONG, 0x21,
    4, 'L', 'B', 'V', 'C', LONG, 0x28,
    4, 'L', 'B', 'V', 'S', LONG, 0x29,
    4, 'C', 'M', 'P', 'D', ALL, 0x83,
    4, 'C', 'M', 'P', 'Y', ALL, 0x8C,
    3, 'L', 'D', 'S', ALL, 0xCE,
    3, 'L', 'D', 'Y', ALL, 0x8E,
    3, 'S', 'T', 'S', ALTER, 0xCF,
    3, 'S', 'T', 'Y', ALTER, 0x8F,
    4, 'S', 'W', 'I', '2', INHER, 0x3F,
#endif /* MC6809 */
    0				/* end of page 1 ops */
};

PUBLIC char page2ops[] =
{
#ifdef MC6809
    4, 'C', 'M', 'P', 'S', ALL, 0x8C,
    4, 'C', 'M', 'P', 'U', ALL, 0x83,
    4, 'S', 'W', 'I', '3', INHER, 0x3F,
#endif
    0				/* end of page 2 ops */
};

#ifdef I80386
# ifdef MNSIZE
PUBLIC char bytesizeops[] =
{
    4, 'A', 'D', 'C', 'B', GROUP1, 0x10,
    4, 'A', 'D', 'D', 'B', GROUP1, 0x00,
    4, 'A', 'N', 'D', 'B', GROUP1, 0x20,
    4, 'C', 'M', 'P', 'B', GROUP1, CMP_OPCODE_BASE,
    4, 'D', 'E', 'C', 'b', INCDEC, 0x08,
    4, 'D', 'I', 'V', 'B', DIVMUL, 0x30,
    5, 'I', 'D', 'I', 'V', 'B', DIVMUL, 0x38,
    5, 'I', 'M', 'U', 'L', 'B', IMUL, 0x28,
    3, 'I', 'N', 'B', IN, 0xEC,
    4, 'I', 'N', 'C', 'B', INCDEC, 0x00,
    4, 'M', 'O', 'V', 'B', MOV, 0x88,
    4, 'M', 'U', 'L', 'B', DIVMUL, 0x20,
    4, 'N', 'E', 'G', 'B', NEGNOT, 0x18,
    4, 'N', 'O', 'T', 'B', NEGNOT, 0x10,
    3, 'O', 'R', 'B', GROUP1, 0x08,
    4, 'O', 'U', 'T', 'B', OUT, 0xEE,
    4, 'R', 'C', 'L', 'B', GROUP2, 0x10,
    4, 'R', 'C', 'R', 'B', GROUP2, 0x18,
    4, 'R', 'O', 'L', 'B', GROUP2, 0x00,
    4, 'R', 'O', 'R', 'B', GROUP2, 0x08,
    4, 'S', 'A', 'L', 'B', GROUP2, 0x20,
    4, 'S', 'A', 'R', 'B', GROUP2, 0x38,
    4, 'S', 'H', 'L', 'B', GROUP2, 0x20,
    4, 'S', 'H', 'R', 'B', GROUP2, 0x28,
    4, 'S', 'B', 'B', 'B', GROUP1, 0x18,
    4, 'S', 'U', 'B', 'B', GROUP1, 0x28,
    5, 'T', 'E', 'S', 'T', 'B', TEST, 0x84,
    5, 'X', 'C', 'H', 'G', 'B', XCHG, 0x86,
    4, 'X', 'O', 'R', 'B', GROUP1, 0x30,
    0				/* end of byte size ops */
};
# endif /* MNSIZE */
#endif /* I80386 */

/* --- end of keywords --- */

FORWARD void install P((register char *keyptr, int data));

PUBLIC void inst_keywords()
{
    install(regs, REGBIT);
#ifdef I80386
    install(typesizes, SIZEBIT);
#endif
    install(ops, 0);
    install(page1ops, PAGE1);
    install(page2ops, PAGE2);
#ifdef I80386
# ifdef MNSIZE
    install(bytesizeops, PAGE1 | PAGE2);
# endif
#endif
}

PRIVATE void install(keyptr, data)
register char *keyptr;
unsigned char data;
{
    char lowcasebuf[20];
    unsigned namelength;
    char *nameptr;
    char *namend;
    register struct sym_s *symptr;

    while (*keyptr != 0)
    {
	namelength = *keyptr++;
	lineptr = (symname = keyptr) + namelength;
	for (nameptr = lowcasebuf, namend = lowcasebuf + namelength;
	     nameptr < namend;)
	{
	    if (*keyptr < 'A' || *keyptr > 'Z')
		*nameptr++ = *keyptr++;
	    else
		*nameptr++ = *keyptr++ + ('a' - 'A');
	}
	symptr = lookup();
	symptr->type = MNREGBIT;
	symptr->data = data;
	symptr->value_reg_or_op.op.routine = *keyptr;
	symptr->value_reg_or_op.op.opcode = keyptr[1];
	lineptr = (symname = lowcasebuf) + namelength;
	symptr = lookup();
	symptr->type = MNREGBIT;
	symptr->data = data;
	symptr->value_reg_or_op.op.routine = *keyptr;
	symptr->value_reg_or_op.op.opcode = keyptr[1];
	keyptr += 2;
    }
}

/* Lookup() searches symbol table for the string from symname to lineptr - 1.
 * If string is not found and ifflag is TRUE, string is added to table, with
 *	type = 0
 *	data = inidata (RELBIT | UNDBIT, possibly with IMPBIT | SEGM)
 * Returns pointer to symbol entry (NULL if not found and not installed)
 * unless symbol table overflows, when routine aborts.
 */

PUBLIC struct sym_s *lookup()
{
    struct sym_s **hashptr;
    register char *nameptr;
    register struct sym_s *symptr;
    register unsigned hashval;
    register unsigned length;
#ifdef DEBUG
    int tries;

    ++nlookup;
    tries = 0;
#endif

    /* Hash function is a weighted xor of 1 to 4 chars in the string.
     * This works seems to work better than looking at all the chars.
     * It is important that the function be fast.
     * The string comparision function should also be fast and it helps
     * if it is optimized for mostly identical comparisions.
     * The multiplication by MULTIPLIER should compile as a shift.
     */

#define MULTIPLIER (SPTSIZ / (1 << USEFUL_BITS_IN_ASCII))
#define USEFUL_BITS_IN_ASCII 6

    nameptr = lineptr;
    length = nameptr - symname;
    if (length <= 3)
    {
	if (length <= 2)
	    hashval  = hconv(nameptr[-1]) * MULTIPLIER;
	else
	    hashval  = hconv(nameptr[-2]) * MULTIPLIER,
	    hashval ^= hconv(nameptr[-1]);
    }
    else
	hashval  = hconv(nameptr[-(length / 2)]) * MULTIPLIER,
	hashval ^= hconv(nameptr[-2]) << 2,
	hashval ^= hconv(nameptr[-1]);
    nameptr = symname;
    if ((symptr = *(hashptr = spt +
			      (hashval ^ (hconv(nameptr[0]) << 1)) % SPTSIZ))
	!= NULL)
    {
	do
	{
#ifdef DEBUG
	    if (tries != 0)
		--nx[tries];
	    ++tries;
	    if (tries < sizeof nx / sizeof nx[0])
		++nx[tries];
	    if (tries >= 5)
		printchain(hashptr - spt)
#endif
	    if ((unsigned char) length != symptr->length)
		continue;
	    if (memcmp(symptr->name, nameptr, length) == 0)
		return symptr;
	}
	while ((symptr = symptr->next) != NULL);

	/* Calculate last non-NULL hash ptr.
	 * This is faster than keeping hashptr up to date in previous loop
	 * since most lookups are successful and hash ptr is not needed.
	 */
	do
	{
	    symptr = *hashptr;
	    hashptr = &symptr->next;
	}
	while (symptr->next != NULL);
    }
    if (!ifflag)
	return NULL;
    align(heapptr);
    if (heapptr >= heapend)
	fatalerror(SYMOV);
#ifdef DEBUG
    ++nsym;
    if (hashptr >= spt && hashptr < spt + SPTSIZ)
	++nhash;
#endif
    *hashptr = symptr = (struct sym_s *) heapptr;
    symptr->type = 0;
    symptr->data = inidata;
    symptr->length = length;
    symptr->value_reg_or_op.value = (unsigned) (symptr->next = NULL);
    heapptr = symptr->name;
    do
	*heapptr++ = *nameptr++;
    while (--length != 0);
    *heapptr++ = 0;
    return symptr;
}

#ifdef DEBUG

static void printchain(hashval)
unsigned hashval;
{
    register struct sym_s *symptr;

    printf("%04x ", hashval);
    for (symptr = spt[hashval]; symptr != NULL; symptr = symptr->next)
	printf("%s ", symptr->name);
    printf("\n");
}

#endif

PUBLIC void statistics()
{
#ifdef DEBUG
    int i;
    int weight;

    for (i = 0; i < SPTSIZ; ++i)
	printchain(i);
    printf("nhash = %d, nsym = %d, nlookup = %d nx =\n", nhash, nsym, nlookup);
    weight = 0;
    for (i = 0; i < 30; ++i) 
    {
	printf("%5d", nx[i]);
	weight += nx[i] * i;
    }
    printf("\n");
    printf("weight = %d%d\n", w;
#endif
}