/* scan.h - scanner for bcc */

/* Copyright (C) 1992 Bruce Evans */

#define NAMESIZE	64	/* limit on identifier lengths */
#define SYMOFCHAR(ch)	(symofchar[(unsigned char) (ch)])

/* scanner codes */

enum scan_states
{
/* The first group of entries consists of all the values that occur in the
   switch for cppscan().
*/
    IDENT,
    INTCONST,
#define MAXIDSYM INTCONST	/* IDENT and INTCONST must be the only
				 * symofchar[] entries below this */
    FLOATCONST,
#define MAXPPNUMSYM FLOATCONST	/* IDENT, INTCONST and FLOATCONST must be the
				 * only symofchar[] entries below this */
    CHARCONST,
    CONTROL,
    SLASH,
    SPECIALCHAR,
    STRINGCONST,

/* The next group of entries are all the rest of the values that occur in
   symofchar[] and so in the switch for nextsym().
*/
    AMPERSAND,			/* ADDRESSOP or ANDOP */
    BADCHAR,
    COLON,			/* also COLONOP */
    COMMA,			/* also COMMAOP */
    DECSYM,			/* PREDECOP or POSTDECOP */
    EOFSYM,
    HYPHEN,			/* NEGOP or SUBOP */
    INCSYM,			/* PREINCOP or POSTINCOP */
    LBRACE,
    LBRACKET,
    LPAREN,
    RBRACE,
    RBRACKET,
    RPAREN,
    SEMICOLON,
    STAR,			/* INDIRECTOP or MULOP */
    WHITESPACE,

/* The next group of entries consists of all operator codes.  These codes must
   be contiguous so they can be used as (offsetted) array indexes.  The group
   is ordered by operator-precedence (this is not necessary).  The first part
   of it overlaps the previous group.
*/

/* Assign-abops (level 1) belong here but are at end to improve switch. */

#define FIRSTOP CONDOP
    CONDOP,			/* level 2 */

    OROP,			/* level 5 */

    EOROP,			/* level 6 */

    ANDOP,			/* level 7 */

    GTOP,			/* level 9 */
    LTOP,

    ADDOP,			/* level 11 */

    DIVOP,			/* level 12 */
    MODOP,

    LOGNOTOP,			/* level 13 */
    NOTOP,

    STRUCELTOP,			/* level 14 */
    STRUCPTROP,

/* End of symbols that appear in symofchar[]. */

    ASSIGNOP,			/* level 1 - assign ops must be contiguous */
    ADDABOP,
    ANDABOP,
    DIVABOP,
    EORABOP,
    MODABOP,
    MULABOP,
    ORABOP,
    SLABOP,
    SRABOP,
    SUBABOP,

    COMMAOP,			/* level 0 */

    COLONOP,			/* level 2 */

    LOGOROP,			/* level 3 */

    LOGANDOP,			/* level 4 */

    EQOP,			/* level 8 */
    NEOP,

    GEOP,			/* level 9 */
    LEOP,

    SLOP,			/* level 10 */
    SROP,

    SUBOP,			/* level 11 */

    MULOP,			/* level 12 */

    ADDRESSOP,			/* level 13 */
    CASTOP,
    INDIRECTOP,
    NEGOP,
    PREDECOP,
    PREINCOP,
    POSTDECOP,
    POSTINCOP,

    FUNCOP,			/* level 14 */
    LISTOP,
    ROOTLISTOP,

    LEAF,			/* special */
    PTRADDABOP,
    PTRADDOP,
    PTRSUBOP,

/* end of operator codes (they must stay contiguous) */

#define LASTOP PTRSUBOP

    ENUMDECL,
    NULLDECL,
    SIGNDECL,
    STRUCTDECL,
    TYPEDECL,
    TYPEDEFNAME,
    UNIONDECL,
    UNSIGNDECL,

    AUTODECL,
    EXTERNDECL,
    REGDECL,
    STATICDECL,
    TYPEDEFDECL,

    ASMSYM,
    BREAKSYM,
    CASESYM,
    CONTSYM,
    DEFAULTSYM,
    DEFINEDSYM,
    DOSYM,
    ELSESYM,
    FORSYM,
    GOTOSYM,
    IFSYM,
    RETURNSYM,
    SIZEOFSYM,
    SWITCHSYM,
    WHILESYM,

    ASMCNTL,
    DEFINECNTL,
    ENDASMCNTL,
    INCLUDECNTL,
    LINECNTL,
    UNDEFCNTL,
    WARNINGCNTL,
    ERRORCNTL,

    ELIFCNTL,			/* "IF" controls must be contiguous */
    ELSECNTL,
    ENDIFCNTL,
    IFCNTL,
    IFDEFCNTL,
    IFNDEFCNTL
};

EXTERN op_pt arg1op;		/* LISTOP, or ROOTLISTOP if arg1inreg */
EXTERN struct
{
    union
    {
	char *s;
	value_t v;
	double d;
    }
      value;
    struct typestruct *type;
}
 constant;			/* value of last constant scanned */
				/* sym tells type */
EXTERN char funcname[NAMESIZE];	/* name of current function for unique labels */
EXTERN char gs2name[2 + NAMESIZE];	/* 2 reserved for namespace keys */
#define gsname (gs2name + 2)	/* before last identifier */
EXTERN struct symstruct *gsymptr;	/* symbol ptr for last identifier */
EXTERN bool_t incppexpr;	/* nonzero while scanning cpp expression */
EXTERN sym_t sym;		/* current symbol */
extern sym_t symofchar[];	/* table to convert chars to their symbols */
EXTERN bool_t expect_statement; /* If set #asm needs to clear the recursive
				 * pending operations. ie: if stmts. */

