
#include "syshead.h"
#include "const.h"
#include "errors.h"

/* Error codes. */

/* Syntax errors. */
PUBLIC char COMEXP[] =          "comma expected";
PUBLIC char DELEXP[] =          "delimiter expected";
PUBLIC char FACEXP[] =          "factor expected";
PUBLIC char IREGEXP[] =         "index register expected";
PUBLIC char LABEXP[] =          "label expected";
PUBLIC char LPEXP[] =           "left parentheses expected";
PUBLIC char OPEXP[] =           "opcode expected";
PUBLIC char RBEXP[] =           "right bracket expected";
PUBLIC char REGEXP[] =          "register expected";
PUBLIC char RPEXP[] =           "right parentheses expected";
PUBLIC char SPEXP[] =           "space expected";

/* Expression errors. */
PUBLIC char ABSREQ[] =          "absolute expression required";
PUBLIC char NONIMPREQ[] =       "non-imported expression required";
PUBLIC char RELBAD[] =          "relocation impossible";

/* Label errors. */
PUBLIC char ILLAB[] =           "illegal label";
PUBLIC char MACUID[] =          "MACRO used as identifier";
PUBLIC char MISLAB[] =          "missing label";
PUBLIC char MNUID[] =           "opcode used as identifier";
PUBLIC char REGUID[] =          "register used as identifier";
PUBLIC char RELAB[] =           "redefined label";
PUBLIC char UNBLAB[] =          "unbound label";
PUBLIC char UNLAB[] =           "undefined label";
PUBLIC char VARLAB[] =          "variable used as label";

/* Addressing errors. */
PUBLIC char ABOUNDS[] =         "address out of bounds";
PUBLIC char DBOUNDS[] =         "data out of bounds";
PUBLIC char ILLMOD[] =          "illegal address mode";
PUBLIC char ILLREG[] =          "illegal register";

/* Control structure errors. */
PUBLIC char ELSEBAD[] =         "no matching IF";
PUBLIC char ENDBBAD[] =         "no matching BLOCK";
PUBLIC char EOFBLOCK[] =        "end of file in BLOCK";
PUBLIC char EOFIF[] =           "end of file in IF";
PUBLIC char EOFLC[] =           "location counter was undefined at end";
PUBLIC char EOFMAC[] =          "end of file in MACRO";
PUBLIC char FAILERR[] =         "user-generated error";

/* Overflow errors. */
PUBLIC char BLOCKOV[] =         "BLOCK stack overflow";
PUBLIC char BWRAP[] =           "binary file wrap-around";
PUBLIC char COUNTOV[] =         "counter overflow";
PUBLIC char COUNTUN[] =         "counter underflow";
PUBLIC char GETOV[] =           "GET stack overflow";
PUBLIC char IFOV[] =            "IF stack overflow";

PUBLIC char LINLONG[] =         "line too long";
PUBLIC char MACOV[] =           "MACRO stack overflow";
PUBLIC char OBJSYMOV[] =        "object symbol table overflow";
PUBLIC char OWRITE[] =          "program overwrite";
PUBLIC char PAROV[] =           "parameter table overflow";
PUBLIC char SYMOV[] =           "symbol table overflow";
PUBLIC char SYMOUTOV[] =        "output symbol table overflow";

/* I/O errors. */
PUBLIC char OBJOUT[] =          "error writing object file";

/* Miscellaneous errors. */
PUBLIC char AL_AX_EAX_EXP[] =   "al ax or eax expected";
PUBLIC char CTLINS[] =          "control character in string";
PUBLIC char FURTHER[] =         "futher errors suppressed";
PUBLIC char ILL_IMM_MODE[] =    "illegal immediate mode";
PUBLIC char ILL_IND_TO_IND[] =  "illegal indirect to indirect";
PUBLIC char ILL_IND[] =         "illegal indirection";
PUBLIC char ILL_IND_PTR[] =     "illegal indirection from previous 'ptr'";
PUBLIC char ILL_SCALE[] =       "illegal scale";
PUBLIC char ILL_SECTION[] =     "illegal section";
PUBLIC char ILL_SEG_REG[] =     "illegal segment register";
PUBLIC char ILL_SOURCE_EA[] =   "illegal source effective address";
PUBLIC char ILL_SIZE[] =        "illegal size";
PUBLIC char IMM_REQ[] =         "immediate expression expected";
PUBLIC char INDEX_REG_EXP[] =   "index register expected";
PUBLIC char IND_REQ[] =         "indirect expression required";
PUBLIC char MISMATCHED_SIZE[] =  "mismatched size";
PUBLIC char NOIMPORT[] =        "no imports with binary file output";
PUBLIC char REENTER[] =         "multiple ENTER pseudo-ops";
PUBLIC char REL_REQ[] =         "relative expression required";
PUBLIC char REPEATED_DISPL[] =  "repeated displacement";
PUBLIC char SEGREL[] =          "segment or relocatability redefined";
PUBLIC char SEG_REG_REQ[] =     "segment register required";
PUBLIC char SIZE_UNK[] =        "size unknown";
PUBLIC char UNKNOWN_ESCAPE_SEQUENCE[] =  "unknown escape sequence";

PUBLIC char FP_REG_REQ[] =      "FP register required";
PUBLIC char FP_REG_NOT_ALLOWED[] =  "FP register not allowed";
PUBLIC char ILL_FP_REG[] =      "illegal FP register";
PUBLIC char ILL_FP_REG_PAIR[] =  "illegal FP register pair";
PUBLIC char JUNK_AFTER_OPERANDS[] =  "junk after operands";

PUBLIC char ALREADY[] =         "already defined";
PUBLIC char UNSTABLE_LABEL[] =  "label moved in last pass add -O?";

/* Warnings. */
PUBLIC char CPUCLASH[] =        "instruction illegal for current cpu";
PUBLIC char SHORTB[] =          "short branch would do";
