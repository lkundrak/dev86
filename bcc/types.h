/* types.h - type definitions for bcc */

/* Copyright (C) 1992 Bruce Evans */

/*
  source types big enough to handle target quantities
  these have to be match the source compiler and target machine
  this is impossible if the source long type is too small
*/

typedef unsigned char char_t;	/* target char */
typedef long value_t;		/* target ints, longs and offsets */
typedef unsigned long uvalue_t;	/* target unsigned ints, longs and offsets */

#ifdef I8088
typedef long offset_T;		/* target machine offset */
typedef unsigned long uoffset_T;	/* target unsigned machine offset */
#define outuvalue outhex
#define outvalue outshex
#endif
#ifdef MC6809
typedef int offset_T;
typedef unsigned uoffset_T;
#endif


/*
  storage class type must hold all the flags defined elsewhere
  it must have a few more bits than the target has registers
*/

#ifdef I8088
typedef unsigned store_pt;	/* promoted store_t */
typedef unsigned store_t;	/* storage class flags */
#endif
#ifdef MC6809
#ifdef __STDC__
typedef int store_pt;
# else
typedef unsigned store_pt;
# endif
typedef unsigned char store_t;
#endif


/*
  types for library routines
*/

typedef int fd_t;		/* file descriptor */


/*
  basic scalar types - may be tuned to suit machine
*/

typedef int fastin_pt;		/* promoted fastin_t */
				/* always an int - use to show that the */
				/* value may be accessed as a fastin_t */
typedef char fastin_t;		/* fast int - a small integer value */
				/* on some machines, chars can be accessed */
				/* more efficiently than ints for arithmetic */
				/* such as comparing and masking which */
				/* does not requiring promotion */
				/* change to int if chars are inefficient */
				/* or if char has < 8 bits */
typedef int smalin_pt;		/* promoted smalin_t */
typedef char smalin_t;		/* small int - a small integer value */
				/* on most machines, chars are stored in */
				/* less space than any other type */
				/* change to fastin_t if this is not true */
				/* or if char has < 8 bits */
				/* or if space is not a limiting factor */
#ifdef __STDC__
typedef int smalu_pt;
#else
typedef unsigned smalu_pt;
#endif
typedef unsigned char smalu_t;


/*
  derived scalar types
  the types containing bit flags fit in an 8 bit smalin_t
*/

typedef fastin_pt bool_pt;	/* promoted bool_t */
typedef fastin_t bool_t;	/* boolean: TRUE if nonzero */
typedef fastin_pt ccode_pt;	/* promoted ccode_t */
typedef fastin_t ccode_t;	/* condition code code */
typedef smalu_pt constr_pt;	/* promoted constr_t */
typedef smalu_t constr_t;	/* type constructor flags */
typedef smalu_pt indn_pt;	/* promoted indn_t */
typedef smalu_t indn_t;		/* storage indirection count */
typedef unsigned label_no;	/* label number */
typedef smalu_t maclev_t;	/* macro expansion level */
typedef smalin_pt op_pt;	/* promoted op_t */
typedef smalin_t op_t;		/* operator code */
typedef smalu_t sc_t;		/* storage class flags */
typedef smalu_pt scalar_pt;	/* promoted scalar_t */
typedef smalu_t scalar_t;	/* type scalar flags */
typedef smalu_t scopelev_t;	/* scope level */
typedef fastin_pt sym_pt;	/* promoted sym_t */
typedef fastin_t sym_t;		/* symbol code from scanner */
typedef smalu_t weight_t;	/* expression tree node weight */


/*
  derived structure types
  the fields are ordered in a way that is most space-efficient
  when smalin_t is char and smalu_t is unsigned char
  the first element of the structures is the one most frequently accessed
*/

/*
  expression table entry format
*/

struct nodestruct
{
    op_t tag;
    weight_t weight;
    smalu_t flags;
    struct typestruct *nodetype;
    union nodeunion
    {
	struct nodestruct *nodeptr;
	struct symstruct *symptr;
    } left;
    struct nodestruct *right;
};

/*
  symbol table entry format
*/

struct symstruct
{
    store_t storage;
    indn_t indcount;
    sc_t flags;
    scopelev_t level;
    struct symstruct *next;
    struct symstruct **prev;
    struct typestruct *type;
    union
    {
	double *offd;		/* value for double constants */
	offset_T offi;		/* offset for register or global storage */
	label_no offlabel;	/* label number for strings */
	char *offp;		/* to string for macro definitions */
	sym_pt offsym;		/* symbol code for keywords */
	value_t offv;		/* value for integral constants */
    }
     offset;
    union
    {
	label_no label;		/* label number for strings */
	char namea[1];		/* variable length array for declarations */
	char *namep;		/* to constant storage for exprs */
    }
     name;
};

/*
  type table entry format
*/

struct typestruct
{
    scalar_t scalar;		/* scalar type flags u d f l i s c */
    constr_t constructor;	/* constructed type flags a f p s/u */
    char structkey[2];		/* unique prefix for member names */
				/* ranges from "\001\001" to "@\377" */
				/* avoiding nulls */
    uoffset_T alignmask;	/* alignment mask, typesize - 1 for scalars */
    uoffset_T typesize;		/* size of this type */
    char *tname;		/* name of scalar type or constructor */
    struct typelist *listtype;	/* list of member types */
    struct typestruct *nexttype;
				/* next in list */
    struct typestruct *prevtype;
				/* previous in list */
    struct typestruct *sidetype;
				/* next in sideways list */
};

/*
  list of type structures
*/

struct typelist
{
    struct typelist *tlnext;
    struct typestruct *tltype;
};


/*
  definitions to avoid passing raw NULLs to functions
*/

#define NULLNODE ((struct nodestruct *) NULL)
#define NULLTYPE ((struct typestruct *) NULL)
