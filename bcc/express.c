/* express.c - expression parsing routines for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"
#include "gencode.h"
#include "parse.h"
#include "reg.h"
#include "sc.h"
#include "scan.h"
#include "table.h"		/* just for charptr for string constant */
#include "type.h"

PRIVATE unsigned insizeof;	/* nest level for getsizeof */
				/* used to avoid aborting undefined idents */
				/* to 0 when they appear in a cpp expression */
				/* under sizeof */

/* names of expression functions are related to the 15 precedence levels */
/* on p49 of K & R */

FORWARD struct nodestruct *cast_exp P((void));
FORWARD struct nodestruct *exp2 P((void));
FORWARD struct nodestruct *exp3to12 P((fastin_pt lprecedence));
FORWARD struct nodestruct *listargs P((void));
FORWARD struct nodestruct *postfix_exp P((bool_pt seenlp));
FORWARD struct nodestruct *primary_exp P((void));
FORWARD struct nodestruct *unary_exp P((void));

PRIVATE struct nodestruct *cast_exp()
{
    struct nodestruct *nodeptr;
    scalar_t scalar;
    struct typestruct *vartype;

    if (sym != LPAREN)
	return unary_exp();
    nextsym();
    if ((vartype = typename()) == NULL)
	return postfix_exp(TRUE);
    rparen();
    scalar = (nodeptr = cast_exp())->nodetype->scalar;
    if (vartype->scalar & INT && scalar & (CHAR | SHORT | INT)
	&& !((vartype->scalar ^ scalar) & UNSIGNED))
    {
	nodeptr->flags &= ~LVALUE;
	return nodeptr;		/* skip casts that are default promotions */
    }
    return castnode(vartype, nodeptr);
}

PUBLIC struct nodestruct *assignment_exp()
{
    struct nodestruct *lhs;
    op_pt op;

    lhs = exp2();
    if (sym >= ASSIGNOP && sym <= SUBABOP)	/* assign-op syms in order! */
    {
	op = sym;
	nextsym();
	lhs = node(op, lhs, assignment_exp());
    }
    return lhs;
}

PUBLIC struct nodestruct *expression()
{
    struct nodestruct *lhs;

    lhs = assignment_exp();
    while (sym == COMMA)
    {
	nextsym();
	lhs = node(COMMAOP, lhs, assignment_exp());
    }
    return lhs;
}

PRIVATE struct nodestruct *exp2()
{
    struct nodestruct *lhs;
    struct nodestruct *rhs;

    lhs = exp3to12(0);
    if (sym == CONDOP)
    {
	nextsym();
	rhs = expression();
	colon();
	lhs = node(CONDOP, lhs, node(COLONOP, rhs, exp2()));
    }
    return lhs;
}

PRIVATE struct nodestruct *exp3to12(lprecedence)
fastin_pt lprecedence;
{
    struct nodestruct *lhs;
    op_pt op;
    fastin_t rprecedence;

    lhs = cast_exp();
    while (TRUE)
    {
	rprecedence = 0;
	switch (sym)
	{
	case LOGOROP:
	    if ((fastin_t) lprecedence <= 1)
		rprecedence = 2;
	    break;
	case LOGANDOP:
	    if ((fastin_t) lprecedence <= 3)
		rprecedence = 4;
	    break;
	case OROP:
	    if ((fastin_t) lprecedence <= 5)
		rprecedence = 6;
	    break;
	case EOROP:
	    if ((fastin_t) lprecedence <= 7)
		rprecedence = 8;
	    break;
	case AMPERSAND:
	    if ((fastin_t) lprecedence <= 9)
	    {
		sym = ANDOP;
		rprecedence = 10;
	    }
	    break;
	case EQOP:
	case NEOP:
	    if ((fastin_t) lprecedence <= 11)
		rprecedence = 12;
	    break;
	case GEOP:
	case GTOP:
	case LEOP:
	case LTOP:
	    if ((fastin_t) lprecedence <= 13)
		rprecedence = 14;
	    break;
	case SLOP:
	case SROP:
	    if ((fastin_t) lprecedence <= 15)
		rprecedence = 16;
	    break;
	case HYPHEN:
	    if ((fastin_t) lprecedence <= 17)
	    {
		sym = SUBOP;
		rprecedence = 18;
	    }
	    break;
	case ADDOP:
	    if ((fastin_t) lprecedence <= 17)
		rprecedence = 18;
	    break;
	case STAR:
	    if ((fastin_t) lprecedence <= 19)
	    {
		sym = MULOP;
		rprecedence = 20;
	    }
	    break;
	case DIVOP:
	case MODOP:
	    if ((fastin_t) lprecedence <= 19)
		rprecedence = 20;
	    break;
	}
	if (rprecedence == 0)
	    break;
	op = sym;
	nextsym();
	lhs = node(op, lhs, exp3to12(rprecedence));
    }
    return lhs;
}

PRIVATE struct nodestruct *listargs()
{
    struct nodestruct *parent;
    struct nodestruct *nextright;

    if (sym == RPAREN)
    {
	nextsym();
	return NULLNODE;
    }
    parent = node(arg1op, assignment_exp(), NULLNODE);
    nextright = parent;
    while (sym == COMMA)
    {
	nextsym();
	nextright = nextright->right
	    = node(LISTOP, assignment_exp(), NULLNODE);
    }
    rparen();
    return parent;
}

PRIVATE struct nodestruct *postfix_exp(seenlp)
bool_pt seenlp;
{
    struct nodestruct *nodeptr;
    struct symstruct *symptr;

    if (seenlp)
    {
	nodeptr = expression();
	rparen();
    }
    else
	nodeptr = primary_exp();
    while (TRUE)
    {
	switch (sym)
	{
	case DECSYM:
	    nextsym();
	    nodeptr = node(POSTDECOP, nodeptr, NULLNODE);
	    continue;
	case INCSYM:
	    nextsym();
	    nodeptr = node(POSTINCOP, nodeptr, NULLNODE);
	    continue;
	case LBRACKET:
	    nextsym();
	    nodeptr = node(INDIRECTOP, node(ADDOP, nodeptr, expression()),
			   NULLNODE);
	    rbracket();
	    continue;
	case LPAREN:
	    nextsym();
	    nodeptr = node(FUNCOP, nodeptr, listargs());
	    {
		register struct nodestruct *np;

		for (np = nodeptr->right; np != NULL; np = np->right)
		{
		    if (np->nodetype->scalar & RSCALAR)
		    {
			np = nodeptr->left.nodeptr;
			if (np->tag != LEAF)
			    printf_fp = TRUE;
			else
			{
			    unsigned len;
			    register char *name;

			    name = np->left.symptr->name.namep;
			    if ((len = strlen(name)) >= 6
				&& strcmp(name + len - 6, "printf") == 0)
				printf_fp = TRUE;
			}
			break;
		    }
		}
		for (np = nodeptr->right; np != NULL; np = np->right)
		{
		    if (np->nodetype->constructor & POINTER
			&& np->nodetype->nexttype->scalar & RSCALAR)
		    {
			np = nodeptr->left.nodeptr;
			if (np->tag != LEAF)
			    scanf_fp = TRUE;
			else
			{
			    unsigned len;
			    register char *name;

			    name = np->left.symptr->name.namep;
			    if ((len = strlen(name)) >= 5
				&& strcmp(name + len - 5, "scanf") == 0)
				scanf_fp = TRUE;
			}
			break;
		    }
		}
	    }
	    continue;
	case STRUCPTROP:
	    nodeptr = node(INDIRECTOP, nodeptr, NULLNODE);
	case STRUCELTOP:
	    nextsym();
	    gs2name[0] = nodeptr->nodetype->structkey[0];
	    gs2name[1] = nodeptr->nodetype->structkey[1];
	    if ((gsymptr = findlorg(gs2name)) == NULL)
	    {
		error("undefined structure element");
		gsymptr = addglb(gs2name, itype);
	    }
	    symptr = exprsym(gsymptr);
	    nextsym();
	    nodeptr = node(STRUCELTOP, nodeptr, leafnode(symptr));
	    continue;
	default:
	    return nodeptr;
	}
    }
}

PRIVATE struct nodestruct *primary_exp()
{
    bool_t isdefined;
    struct nodestruct *nodeptr;
    uoffset_T stringlen;
    struct symstruct *symptr;
    struct symstruct *symptr1;
    bool_t waslparen;

    switch (sym)
    {
    case IDENT:
	if (incppexpr && !insizeof)
	{
cpp_ident:
	    nextsym();
	    return leafnode(constsym((value_t) 0));
	}
	if ((symptr = gsymptr) != NULL)
	    nextsym();
	else
	{
	    symptr = addglb(gsname, fitype);
	    nextsym();
	    if (sym != LPAREN)
	    {
		error2error(symptr->name.namea, " undeclared");
		symptr->indcount = 1;
		symptr->type = itype;
	    }
	}
	symptr1 = exprsym(symptr);
	if (symptr->flags & STATIC && symptr->level != GLBLEVEL)
	{
	    symptr1->flags |= LABELLED;
	    symptr1->offset.offi = 0;
	    symptr1->name.label = symptr->offset.offlabel;
	}
	nodeptr = leafnode(symptr1);
	if (!(nodeptr->nodetype->constructor & (ARRAY | FUNCTION | VOID)))
	    nodeptr->flags = LVALUE;
	return nodeptr;
    case TYPEDEFNAME:
	if (incppexpr && !insizeof)
	    goto cpp_ident;	/* else fall through */
    default:
	error("bad expression");
	constant.value.v = 0;
	constant.type = itype;
    case CHARCONST:
    case INTCONST:		/* this includes enumeration-constants */
	symptr = constsym(constant.value.v);
	symptr->type = constant.type;
	if (!(ltype->scalar & LONG))
	{
	    if (symptr->type == ltype)
		symptr->type = itype;
	    else if (symptr->type == ultype)
		symptr->type = uitype;
	}
	nextsym();
	return leafnode(symptr);
    case DEFINEDSYM:
	waslparen = isdefined = FALSE;
	if (!blanksident())
	{
	    nextsym();
	    if (sym != LPAREN)
		lparen();
	    else
		waslparen = TRUE;
	}
	if (waslparen && !blanksident())
	    needvarname();
	else
	{
	    if ((symptr = findlorg(gsname)) != NULL &&
		symptr->flags == DEFINITION)
		isdefined = TRUE;
	    nextsym();
	}
	if (waslparen)
	    rparen();
	return leafnode(constsym((value_t) isdefined));
    case FLOATCONST:
	symptr = constsym((value_t) 0);
	symptr->type = constant.type;
	symptr->offset.offd = qmalloc(sizeof *symptr->offset.offd);
	*symptr->offset.offd = constant.value.d;
	nextsym();
	return leafnode(symptr);
    case LPAREN:
	nextsym();
	nodeptr = expression();
	rparen();
	return nodeptr;
    case STRINGCONST:
	symptr = constsym((value_t) 0);
	symptr->storage = GLOBAL;
	symptr->flags = LABELLED | STRING;
	/* string length before defstr() or prefix() updates charptr */
	stringlen = charptr - constant.value.s + 1;
	symptr->name.label = defstr(constant.value.s, charptr, FALSE);
	symptr->type = prefix(ARRAY, stringlen, ctype);
	nextsym();
	return leafnode(symptr);
    }
}

PRIVATE struct nodestruct *unary_exp()
{
    value_t size;
    struct typestruct *vartype;

    switch (sym)
    {
    case ADDOP:
	nextsym();
	return cast_exp();
    case AMPERSAND:
	nextsym();
	return node(ADDRESSOP, cast_exp(), NULLNODE);	/* maybe unary_exp */
    case DECSYM:
	nextsym();
	return node(PREDECOP, unary_exp(), NULLNODE);
    case HYPHEN:
	nextsym();
	return node(NEGOP, cast_exp(), NULLNODE);
    case INCSYM:
	nextsym();
	return node(PREINCOP, unary_exp(), NULLNODE);
    case LOGNOTOP:
	nextsym();
	return node(LOGNOTOP, cast_exp(), NULLNODE);
    case NOTOP:
	nextsym();
	return node(NOTOP, cast_exp(), NULLNODE);
    case SIZEOFSYM:
	nextsym();
	++insizeof;
	if (sym != LPAREN)
	    size = unary_exp()->nodetype->typesize;
	else
	{
	    nextsym();
	    if ((vartype = typename()) != NULL)
	    {
		rparen();
		size = vartype->typesize;
	    }
	    else
		size = postfix_exp(TRUE)->nodetype->typesize;
	}
	--insizeof;
	return leafnode(constsym(size));
    case STAR:
	nextsym();
	return node(INDIRECTOP, cast_exp(), NULLNODE);	/* maybe unary_exp */
    }
    return postfix_exp(FALSE);
}
