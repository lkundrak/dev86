/* exptree.c - expression tree routines for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"
#include "byteord.h"
#include "gencode.h"
#include "parse.h"
#include "reg.h"
#include "sc.h"
#include "scan.h"
#include "sizes.h"
#include "type.h"

#ifdef VERY_SMALL_MEMORY
#define ETREESIZE 300
#else
#define ETREESIZE 1200
#endif
#define errtype itype
#define redtype ctype
#define uredtype uctype

PRIVATE struct nodestruct etree[ETREESIZE];	/* expression tree */
PRIVATE struct nodestruct *ettop;

FORWARD void badlvalue P((struct nodestruct *nodeptr));
FORWARD void binconvert P((struct nodestruct *nodeptr));
FORWARD void castiright P((struct nodestruct *nodeptr));
FORWARD void etreefull P((void));
FORWARD void fixnode P((struct nodestruct *nodeptr));
FORWARD bool_pt isconst0 P((struct nodestruct *nodeptr));
FORWARD bool_pt isnodecharconst P((struct nodestruct *nodeptr));
FORWARD void needint P((struct nodestruct *nodeptr));
FORWARD void neednonstruct P((struct nodestruct *nodeptr));
FORWARD void needscalar P((struct nodestruct *nodeptr));
FORWARD void needspv P((struct nodestruct *nodeptr));
FORWARD struct typestruct *nodetype P((struct nodestruct *nodeptr));
FORWARD int redscalar P((struct nodestruct *nodeptr));
FORWARD struct nodestruct *unconvert P((struct nodestruct *nodeptr));

PRIVATE void badlvalue(nodeptr)
struct nodestruct *nodeptr;
{
    error("invalid lvalue");
    fixnode(nodeptr);
}

PRIVATE void binconvert(nodeptr)
register struct nodestruct *nodeptr;
{
    bool_t bothscalar;
    value_t divisor;
    scalar_t lscalar;
    register struct nodestruct *right;
    scalar_t rscalar;

    rscalar = (right = nodeptr->right)->nodetype->scalar;
    lscalar = nodeptr->left.nodeptr->nodetype->scalar;
    if ((bothscalar = lscalar | rscalar) & RSCALAR)
    {
	nodeptr->nodetype = dtype;
	if (!(lscalar & DOUBLE))
	    nodeptr->left.nodeptr = castnode(dtype, nodeptr->left.nodeptr);
	if (!(rscalar & DOUBLE))
	    nodeptr->right = castnode(dtype, right);
    }
    else if (!(bothscalar & DLONG) && ((nodeptr->tag == ANDOP &&
	     (redscalar(nodeptr->left.nodeptr) | redscalar(right)) & CHAR) ||
	     ((nodeptr->tag == EOROP || nodeptr->tag == OROP) &&
	     redscalar(nodeptr->left.nodeptr) & redscalar(right) & CHAR) ||
	     (nodeptr->tag == MODOP && right->tag == LEAF &&
	     right->left.symptr->storage == CONSTANT &&
	     (divisor = right->left.symptr->offset.offv,
	      (uvalue_t) divisor <= MAXUCHTO + 1) &&
	     bitcount((uvalue_t) divisor) <= 1)))
    {
	/* result fits in char and extends correctly */
	if (bothscalar & UNSIGNED)
	    nodeptr->nodetype = uredtype;
	else
	    nodeptr->nodetype = redtype;
    }
    else if (bothscalar & LONG)
    {
	nodeptr->nodetype = ltype;
	if (bothscalar & UNSIGNED)
	    nodeptr->nodetype = ultype;
	if (bothscalar & (DLONG | SHORT | INT))
	{
	    /* XXX - otherwise both are long, or one is long with the same
	     * size as int and the other is char, and we want the lower level
	     * routines to handle the char.
	     */
	    if (!(lscalar & LONG))
		nodeptr->left.nodeptr = castnode(nodeptr->nodetype,
						 nodeptr->left.nodeptr);
	    if (!(rscalar & LONG))
		nodeptr->right = castnode(nodeptr->nodetype, right);
	}
    }
#ifdef I80386
    else if (i386_32 && bothscalar & SHORT)
    {
	nodeptr->nodetype = itype;
	if (bothscalar & UNSIGNED)
	    nodeptr->nodetype = uitype;
	if (lscalar & SHORT)
	    nodeptr->left.nodeptr = castnode(nodeptr->nodetype,
					     nodeptr->left.nodeptr);
	if (rscalar & SHORT)
	    nodeptr->right = castnode(nodeptr->nodetype, right);
    }
#endif
    else if (bothscalar & UNSIGNED)
	nodeptr->nodetype = uitype;
    else
	nodeptr->nodetype = itype;
}

PRIVATE void castiright(nodeptr)
struct nodestruct *nodeptr;
{
    nodeptr->right = castnode(itype, nodeptr->right);
}

PUBLIC struct nodestruct *castnode(type, nodeptr)
struct typestruct *type;
struct nodestruct *nodeptr;
{
    struct symstruct *symptr;

    (symptr = constsym((value_t) 0))->type = type;
    return node(CASTOP, nodeptr, leafnode(symptr));
}

PRIVATE void etreefull()
{
#if ETREESIZE == 1200
    limiterror("expression too complex (1201 nodes)");
#else
    limiterror("expression too complex (ETREESIZE)");
#endif
}

PUBLIC void etreeinit()
{
    ettop = (etptr = etree) + ETREESIZE;
}

PRIVATE void fixnode(nodeptr)
register struct nodestruct *nodeptr;
{
    nodeptr->tag = LEAF;
    nodeptr->flags = nodeptr->weight = 0;
    nodeptr->left.symptr = constsym((value_t) 0);  /* this has type errtype */
    nodeptr->nodetype = errtype;
}

PRIVATE bool_pt isconst0(nodeptr)
register struct nodestruct *nodeptr;
{
    register struct symstruct *symptr;

    return nodeptr->tag == LEAF &&
	   (symptr = nodeptr->left.symptr)->storage == CONSTANT &&
	   symptr->offset.offv == 0 &&
	   symptr->type->scalar & ISCALAR;
}

PRIVATE bool_pt isnodecharconst(nodeptr)
register struct nodestruct *nodeptr;
{
    register struct symstruct *symptr;

    if (nodeptr->tag == LEAF &&
	(symptr = nodeptr->left.symptr)->storage == CONSTANT &&
	ischarconst(symptr->offset.offv) &&
	symptr->type->scalar & ISCALAR)
	return TRUE;
    return FALSE;
}

PUBLIC struct nodestruct *leafnode(source)
struct symstruct *source;
{
    register struct nodestruct *leafptr;

    if ((leafptr = etptr++) >= ettop)
	etreefull();
    leafptr->tag = LEAF;
    leafptr->flags = leafptr->weight = 0;
    leafptr->nodetype = source->type;
    leafptr->left.symptr = source;
    leafptr->right = NULL;
    return leafptr;
}

PRIVATE void needint(nodeptr)
struct nodestruct *nodeptr;
{
    if (!(nodeptr->nodetype->scalar & ISCALAR))
    {
	error("need integral type");
	fixnode(nodeptr);
    }
}

PRIVATE void neednonstruct(nodeptr)
struct nodestruct *nodeptr;
{
    if (nodeptr->nodetype->constructor & STRUCTU)
    {
	error("need non-structure type");
	fixnode(nodeptr);
    }
}

PRIVATE void needscalar(nodeptr)
struct nodestruct *nodeptr;
{
    if (!nodeptr->nodetype->scalar)
    {
	error("need scalar");
	fixnode(nodeptr);
    }
}

PRIVATE void needspv(nodeptr)
struct nodestruct *nodeptr;
{
    if (nodeptr->nodetype->constructor & (ARRAY | FUNCTION | STRUCTU))
    {
	error("need scalar or pointer or void");
	fixnode(nodeptr);
    }
}

PUBLIC struct nodestruct *node(t, p1, p2)
op_pt t;
struct nodestruct *p1;
struct nodestruct *p2;
{
#if MAXREGS != 1
    weight_t rightweight;
#endif
    double dsourceval = 0; /* for -Wall */
    double dtargval = 0 ; /* for -Wall */
    bool_t lflag;
    scalar_t lscalar;
    struct nodestruct *pswap;
    struct symstruct *source = NULL; /* for -Wall */
    value_t sourceval = 0 ;  /* for -Wall */
    struct symstruct *target;
    offset_T targszdelta;
    value_t targval = 0; /* for -Wall */
    scalar_t rscalar = 0; /* for -Wall */
    bool_t uflag;

    switch ((op_t) t)
    {
    case ADDABOP:
	if (p1->nodetype->constructor & (ARRAY | POINTER))
	    t = PTRADDABOP;
	break;
    case ADDOP:
	if (p2->nodetype->constructor & (ARRAY | POINTER))
	{
	    pswap = p1;
	    p1 = p2;
	    p2 = pswap;
	}
	if (p1->nodetype->constructor & (ARRAY | POINTER))
	    t = PTRADDOP;
	break;
    case CONDOP:
	/* Change structs to struct pointers so condop() doesn't have to know
	 * about structs.
	 */
	if (p2->nodetype->constructor & STRUCTU)
	{
	    p2 = node(COLONOP, node(ADDRESSOP, p2->left.nodeptr, NULLNODE),
		      node(ADDRESSOP, p2->right, NULLNODE));
	    return node(INDIRECTOP, node(CONDOP, p1, p2), NULLNODE);
	}
	break;
    case FUNCOP:
	if (p1->nodetype->constructor & (ARRAY | POINTER))
	    p1 = node(INDIRECTOP, p1, NULLNODE);
	break;
    case SUBABOP:
	if (p1->nodetype->constructor & (ARRAY | POINTER))
	{
	    t = PTRADDABOP;
	    p2 = node(NEGOP, p2, NULLNODE);
	    break;
	}
    case SUBOP:
#if 0
	/* This works but sometimes pessimizes the code by converting small
	 * constants into large negative constants.  It makes the most
	 * difference for longs by avoiding stack tangles.  longop could be
	 * more careful.  Only the case (regpair) - constant is naturally
	 * tangled.
	 */
	if (p2->tag == LEAF && p2->nodetype->scalar
	    && (source = p2->left.symptr)->storage == CONSTANT)
	{
	    if (source->type->scalar & RSCALAR)
		*source->offset.offd = -*source->offset.offd;
	    else
		source->offset.offv = -source->offset.offv;
	    return node((op_t) t == SUBOP ? ADDOP : ADDABOP, p1, p2);
	}
#endif
	if (p1->nodetype->constructor & (ARRAY | POINTER))
	{
	    if (p2->nodetype->nexttype == p1->nodetype->nexttype)
		t = PTRSUBOP;
	    else
	    {
		t = PTRADDOP;
		p2 = node(NEGOP, p2, NULLNODE);
	    }
	}
	break;
    }
    if (((op_t) t == PTRADDABOP || (op_t) t == PTRADDOP) &&
	p1->nodetype->nexttype->constructor & (FUNCTION | VOID))
	error("arithmetic on pointer to function or void");
    if (p1->tag != LEAF)
	goto node1;
    target = p1->left.symptr;
    switch ((op_t) t)
    {
    case ADDRESSOP:
	if (target->indcount == 0 && target->flags == REGVAR)
	    error("register variable addressed");
	else if (target->type->constructor & ARRAY)
	{
	    if (target->indcount != 0)
		bugerror("botched array indirection count");
	    target->type = pointype(target->type);
	}
	else if (!(p1->flags & LVALUE))
	    badlvalue(p1);
	else
	{
	    address(target);	/* change p1, no code generated */
	    if (target->indcount == 2)	/* XXX - MAXINDIRECT? */
		/* just became direct enough */
		p1->weight = 1;
	}
	p1->flags &= ~LVALUE;
	p1->nodetype = target->type;
	return p1;
    case CASTOP:
	needspv(p2);
	if (p1->nodetype == p2->nodetype)
	{
	    p1->flags &= ~LVALUE;
	    return p1;
	}
	if ((rscalar = p2->nodetype->scalar) & ISCALAR)
	    neednonstruct(p1);
	else if (rscalar & RSCALAR)
	    needscalar(p1);
	else			/* POINTER */
	    neednonstruct(p1);	/* H & S say functions & arrays must match */
	if (((lscalar = p1->nodetype->scalar) | rscalar) & RSCALAR)
	{
	    if (target->storage != CONSTANT)
		goto node1;
	    if (lscalar & RSCALAR && !(rscalar & RSCALAR))
	    {
		double val;
		/* XXX: Gcc warns about ansi vs k&r problem with this */
		static double MAXULONG = (double)0xFFFFFFFFL +1;

		val = *target->offset.offd;
		if (val > maxlongto)
		    val -= MAXULONG;
		target->offset.offv = (value_t) val;
	    }
	    if (!(lscalar & RSCALAR) && rscalar & RSCALAR)
	    {
		value_t val;

		val = target->offset.offv;
		target->offset.offd = qmalloc(sizeof *target->offset.offd);
		if (lscalar & UNSIGNED)
		    *target->offset.offd = (uvalue_t) val;
		else
		    *target->offset.offd = val;
	    }
	}
	if (target->storage == CONSTANT)
	{
	    if (rscalar & CHAR)
	    {
		target->offset.offv &= CHMASKTO;
		if (p2->nodetype == sctype &&
			target->offset.offv&((~CHMASKTO)>>1))
		    target->offset.offv |= ~CHMASKTO;
	    }
	    else if (rscalar & SHORT)
	    {
		target->offset.offv &= shortmaskto;
		if (!(rscalar & UNSIGNED) && target->offset.offv > maxshortto)
		    target->offset.offv -= (maxushortto + 1);
	    }
	    else if (rscalar & INT)
	    {
		target->offset.offv &= intmaskto;
		if (!(rscalar & UNSIGNED) && target->offset.offv > maxintto)
		    target->offset.offv -= (maxuintto + 1);
	    }
	    else if (rscalar & FLOAT)
		*target->offset.offd = (float) *target->offset.offd;
	}
	else if ((targszdelta =
		  ((p1->nodetype->constructor & (ARRAY | POINTER)) ?
		   ptypesize : p1->nodetype->typesize) -
		  p2->nodetype->typesize) == 0)
	    ;
	else if (target->indcount == 1 && targszdelta > 0 &&
		 target->flags != REGVAR)
	{
#if DYNAMIC_LONG_ORDER
	    if (long_big_endian)
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN
# if INT_BIG_ENDIAN
		target->offset.offi += targszdelta;
# else
	    {
		if (lscalar & DLONG)
		    target->offset.offi += itypesize;	/* discard msword */
	    }
# endif
#endif
#if DYNAMIC_LONG_ORDER
	    else
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN == 0
# if INT_BIG_ENDIAN
	    {
		if (rscalar & CHAR)
		    target->offset.offi += ctypesize;
	    }
# else
	    ;
# endif
#endif
	}
	else
	    goto node1;
	p1->flags &= ~LVALUE;
	p1->nodetype = target->type = p2->nodetype;
	return p1;
    case INDIRECTOP:
	if (!(target->type->constructor & (ARRAY | POINTER)))
	    error("illegal indirection");
	else
	{
	    indirec(target);
	    if (target->indcount == 3)	/* XXX - MAXINDIRECT + 1? */
		/* just became too indirect */
		p1->weight = 1;
	    p1->flags |= LVALUE;
	    p1->nodetype = target->type;
	}
	return p1;
    case STRUCELTOP:		/* p2 known leaf */
	if (target->indcount != 1)
	    break;		/* struc() would generate code */
	struc(p2->left.symptr, target);
	p1->flags |= LVALUE;
	p1->nodetype = target->type;
	return p1;
    }
    if (target->indcount != 0)
	goto node1;
    lscalar = target->type->scalar;
    if (target->storage == CONSTANT)
    {
	if (lscalar & RSCALAR)
	    dtargval = *target->offset.offd;
	else
	    targval = target->offset.offv;
	switch ((op_t) t)
	{
	case COMMAOP:
	    return p2;
	case CONDOP:
	    if ((lscalar & RSCALAR && dtargval != FALSE)
		|| (!(lscalar & RSCALAR) && targval != FALSE))
	    {
		p2->left.nodeptr->nodetype = p2->nodetype;
		return p2->left.nodeptr;
	    }
	    p2->right->nodetype = p2->nodetype;
	    return p2->right;
	case LOGANDOP:
	    if ((lscalar & RSCALAR && dtargval != FALSE)
		|| (!(lscalar & RSCALAR) && targval != FALSE))
		break;
	    p1->nodetype = target->type = itype;
	    return p1;
	case LOGOROP:
	    if ((lscalar & RSCALAR && dtargval == FALSE)
		|| (!(lscalar & RSCALAR) && targval == FALSE))
		break;
	    target->offset.offv = TRUE;
	    p1->nodetype = target->type = itype;
	    return p1;
	}
    }
    switch ((op_t) t)
    {
    case COLONOP:
    case FUNCOP:
    case LISTOP:
    case ROOTLISTOP:
	goto node1;
    case PTRADDOP:
	if (p2->tag == LEAF &&
	    (source = p2->left.symptr)->storage == CONSTANT &&
	    source->type->scalar & ISCALAR)
	{
	    indexadr(source, target);
	    p1->flags &= ~LVALUE;
	    p1->nodetype = target->type;
	    return p1;
	}
    }
    if (target->storage != CONSTANT ||
	!((lscalar & (ISCALAR | RSCALAR)) && (op_t) t != PTRSUBOP) ||
	(p2 != NULL &&
	(p2->tag != LEAF || (source = p2->left.symptr)->storage != CONSTANT ||
	 (!((rscalar = source->type->scalar) & (ISCALAR | RSCALAR))
	 && (op_t) t != PTRSUBOP))))
	goto node1;
    lflag = lscalar & LONG;
    uflag = lscalar & UNSIGNED;
    if (p2 != NULL)
    {
	if (rscalar & RSCALAR)
	    dsourceval = *source->offset.offd;
	else
	{
	    sourceval = source->offset.offv;
	    lflag |= rscalar & LONG;
	    uflag |= rscalar & UNSIGNED;
	}
    }
    if (lscalar & RSCALAR || (p2 != NULL && rscalar & RSCALAR))
    {
    if (!(lscalar & RSCALAR))
    {
	if (uflag)
	    dtargval = (uvalue_t) targval;
	else
	    dtargval = targval;
    }
    if (p2 != NULL && !(rscalar & RSCALAR))
    {
	if (rscalar & UNSIGNED)
	    dsourceval = (uvalue_t) sourceval;
	else
	    dsourceval = sourceval;
    }
    switch ((op_t) t)
    {
    case ADDOP:
	dtargval += dsourceval;
	break;
    case DIVOP:
	if (dsourceval == 0)
	{
	    error("divison by 0");
	    dsourceval = 1;
	}
	dtargval /= dsourceval;
	break;
    case EQOP:
	targval = dtargval == dsourceval;
	goto intconst;
    case GEOP:
	targval = dtargval >= dsourceval;
	goto intconst;
    case GTOP:
	targval = dtargval > dsourceval;
	goto intconst;
    case LEOP:
	targval = dtargval <= dsourceval;
	goto intconst;
    case LOGANDOP:
	targval = dtargval && dsourceval;
	goto intconst;
    case LOGNOTOP:
	targval = !dtargval;
	goto intconst;
    case LOGOROP:
	targval = dtargval || dsourceval;
	goto intconst;
    case LTOP:
	targval = dtargval < dsourceval;
	goto intconst;
    case MULOP:
	dtargval *= dsourceval;
	break;
    case NEGOP:
	dtargval = -dtargval;
	break;
    case NEOP:
	targval = dtargval != dsourceval;
	goto intconst;
    case SUBOP:
	dtargval -= dsourceval;
	break;
    }
    /* XXX - leaks memory */
    target->offset.offd = qmalloc(sizeof *target->offset.offd);
    *target->offset.offd = dtargval;
    p1->nodetype = target->type = dtype;
    return p1;
    }
    switch ((op_t) t)
    {
    case ADDOP:
	targval += sourceval;
	break;
    case ANDOP:
	targval &= sourceval;
	break;
    case DIVOP:
	if (sourceval == 0)
	{
	    error("divison by 0");
	    sourceval = 1;
	}
	if (uflag)
	    targval = (uvalue_t) targval / sourceval;
	else
	    targval /= sourceval;
	break;
    case EOROP:
	targval ^= sourceval;
	break;
    case EQOP:
	targval = targval == sourceval;
	break;
    case GEOP:
	if (uflag)
	    targval = (uvalue_t) targval >= sourceval;
	else
	    targval = targval >= sourceval;
	goto intconst;
    case GTOP:
	if (uflag)
	    targval = (uvalue_t) targval > sourceval;
	else
	    targval = targval > sourceval;
	goto intconst;
    case LEOP:
	if (uflag)
	    targval = (uvalue_t) targval <= sourceval;
	else
	    targval = targval <= sourceval;
	goto intconst;
    case LOGANDOP:
	targval = targval && sourceval;
	goto intconst;
    case LOGNOTOP:
	targval = !targval;
	goto intconst;
    case LOGOROP:
	targval = targval || sourceval;
	goto intconst;
    case LTOP:
	if (uflag)
	    targval = (uvalue_t) targval < sourceval;
	else
	    targval = targval < sourceval;
	goto intconst;
	break;
    case MODOP:
	if (sourceval == 0)
	{
	    error("modulo by 0");
	    sourceval = 1;
	}
	if (uflag)
	    targval = (uvalue_t) targval % sourceval;
	else
	    targval %= sourceval;
	break;
    case MULOP:
	targval *= sourceval;
	break;
    case NEGOP:
	targval = -targval;
	break;
    case NEOP:
	targval = targval != sourceval;
	goto intconst;
    case NOTOP:
	targval = ~targval;
	break;
    case OROP:
	targval |= sourceval;
	break;
    case PTRSUBOP:
	if (targval < sourceval)
	    targval = -((uvalue_t) (sourceval - targval) /
			target->type->nexttype->typesize);
	else
	    targval = (uvalue_t) (targval - sourceval) /
			target->type->nexttype->typesize;
	uflag = FALSE;
	break;
    case SLOP:
	targval <<= sourceval;	/* assume shiftcount treated as unsigned */
	lflag = lscalar & LONG;
	uflag = lscalar & UNSIGNED;
	break;
    case SROP:
	if (uflag)
	    targval = (uvalue_t) targval >> sourceval;
	else
	    targval >>= sourceval;
	lflag = lscalar & LONG;
	uflag = lscalar & UNSIGNED;
	break;
    case SUBOP:
	targval -= sourceval;
	break;
    }
    if (lflag)
    {
	target->type = ltype;
	if (uflag)
	    target->type = ultype;
    }
    else
    {
	targval &= intmaskto;
	if (uflag)
	    target->type = uitype;
	else
	{
	    target->type = itype;
	    if (targval > maxintto)
		targval -= (maxuintto + 1);
	}
    }
    target->offset.offv = targval;
    p1->nodetype = target->type;
    return p1;

intconst:
    target->offset.offv = targval;
    p1->nodetype = target->type = itype;
    return p1;

node1:
    {
	register struct nodestruct *nodeptr;
	register struct nodestruct *regp2;

	if ((nodeptr = etptr++) >= ettop)
	    etreefull();
	regp2 = p2;
	nodeptr->tag = t;
	nodeptr->left.nodeptr = p1;
	nodeptr->right = regp2;
	if ((op_t) t == FUNCOP)
	    nodeptr->weight = MAXREGS + 1;
	else
	{
#if MAXREGS == 1
	    if ((nodeptr->weight = p1->weight) <= MAXREGS &&
		(regp2 == NULL ||
		 (nodeptr->weight = regp2->weight) < MAXREGS))
		nodeptr->weight = MAXREGS;
#else
	    nodeptr->weight = p1->weight;
	    if (regp2 == NULL)
		rightweight = 1;	/* could do POST-ops with 0 */
	    else		/* 0 more approp for LIST-ops but 1 OK */
		rightweight = regp2->weight;
	    if (nodeptr->weight < rightweight)
		nodeptr->weight = rightweight;
	    else if (nodeptr->weight == rightweight &&
		     nodeptr->weight < MAXREGS)
		++nodeptr->weight;
#endif
	}
	nodeptr->nodetype = nodetype(nodeptr);
	return nodeptr;
    }
}

PRIVATE struct typestruct *nodetype(nodeptr)
struct nodestruct *nodeptr;
{
    scalar_t bothscalar;
    scalar_t lscalar;
    scalar_t rscalar;
    struct nodestruct *left;
    struct nodestruct *right;
    struct typestruct *targtype;

    if (nodeptr->tag == LEAF)
	return nodeptr->left.symptr->type;
    targtype = (left = nodeptr->left.nodeptr)->nodetype;
    right = nodeptr->right;
    switch (nodeptr->tag)
    {
    case FUNCOP:
	if (!(targtype->constructor & FUNCTION))
	{
	    error("call of non-function");
	    fixnode(nodeptr);
	    return errtype;
	}
	return targtype->nexttype;
    case INDIRECTOP:
	if (!(targtype->constructor & (ARRAY | POINTER)))
	{
	    error("illegal indirection");
	    fixnode(nodeptr);
	    return errtype;
	}
	nodeptr->flags |= LVALUE;
	return targtype->nexttype;
    case LISTOP:
    case ROOTLISTOP:
	nodeptr->left.nodeptr = unconvert(left);
	return promote(targtype);
    case CASTOP:
	needspv(right);
    case COMMAOP:
    case CONDOP:
	return right->nodetype;

    case STRUCELTOP:
	nodeptr->flags |= LVALUE;
	return right->nodetype;

    case ADDRESSOP:
	if (!(left->flags & LVALUE))
	{
	    badlvalue(nodeptr);
	    return errtype;
	}
	return pointype(targtype);
    case LOGNOTOP:
	neednonstruct(left);
	return redtype;
    case NEGOP:
	needscalar(left);
	return promote(left->nodetype);
    case NOTOP:
	needint(left);
	return promote(left->nodetype);
    case PREDECOP:
    case PREINCOP:
    case POSTDECOP:
    case POSTINCOP:
	neednonstruct(left);
	if (!(left->flags & LVALUE))
	{
	    badlvalue(nodeptr);
	    return errtype;
	}
	break;

    case ANDOP:
    case EOROP:
    case MODOP:
    case OROP:
	needint(left);
	needint(right);
	/* fall through to redundant check and code to cast to same types */
    case ADDOP:
    case DIVOP:
    case MULOP:
    case SUBOP:
	needscalar(left);
	needscalar(right);
	binconvert(nodeptr);
	return nodeptr->nodetype;

    case PTRADDABOP:
	if (!(left->flags & LVALUE))
	{
	    badlvalue(nodeptr);
	    return errtype;
	}
    case PTRADDOP:		/* right type conversion to same size as ptr */
	needint(right);
	if (right->nodetype->scalar & LONG)
	    castiright(nodeptr);
	break;
    case PTRSUBOP:		/* already checked */
	return itype;
    case SLOP:			/* right type conversion always to int */
    case SROP:			/* ulong strictly to unsigned but no matter */
	needint(left);
	needint(right);
	if (right->nodetype->scalar & LONG)
	    castiright(nodeptr);
	return promote(left->nodetype);

    case LTOP:
    case LEOP:
    case GTOP:
    case GEOP:
    case EQOP:
    case LOGOROP:
    case LOGANDOP:
    case NEOP:
	neednonstruct(left);
	neednonstruct(right);
	nodeptr->left.nodeptr = left = unconvert(left);
	nodeptr->right = right = unconvert(right);
#ifdef MAYBE_DO_LATER /* OK if pointers fit in uitype */
	if (left->nodetype->constructor & POINTER)
	    nodeptr->left.nodeptr = left = castnode(uitype, left);
	if (right->nodetype->constructor & POINTER)
	    nodeptr->right = right = castnode(uitype, right);
#endif
	binconvert(nodeptr);
	return redtype;

    case COLONOP:
	if (isnodecharconst(left) && isnodecharconst(right))
	{
	    targtype = (targtype->scalar | right->nodetype->scalar) & UNSIGNED
		       ? uctype : ctype;
	    left = castnode(targtype, left);
	    right = castnode(targtype, right);
	}
	nodeptr->left.nodeptr = left = unconvert(left);
	nodeptr->right = right = unconvert(right);
	targtype = left->nodetype;
	if (
#ifdef I80386
	      i386_32 ||
#endif
	      ( targtype->scalar & FLOAT ) == 0)
	    if (targtype == right->nodetype)
	        return targtype;  /* XXX - reduced from promote(targtype) */
	if ((lscalar = targtype->scalar) != 0 &&
	    (rscalar = right->nodetype->scalar) != 0)
	{
	    /* promote minimally without using binconvert for char/short */
	    if ((bothscalar = lscalar | rscalar) & (LONG | RSCALAR))
	    {
		binconvert(nodeptr);
		return nodeptr->nodetype;
	    }
	    if (bothscalar & INT || (lscalar ^ rscalar) & UNSIGNED)
		targtype = itype;
	    else if (bothscalar & SHORT)
		targtype = stype;
	    else
		targtype = ctype;
	    if (bothscalar & UNSIGNED)
		targtype = tounsigned(targtype);
	    if ((!(bothscalar & INT) || (left->tag == LEAF && lscalar & INT)) ||
		(right->tag == LEAF && rscalar & INT))
	    {
		/* this is correct even when the if fails (the casts have to */
		/* be considered at a lower level) but gives worse code */
		if (left->nodetype != targtype)
		    nodeptr->left.nodeptr = castnode(targtype, left);
		if (right->nodetype != targtype)
		    nodeptr->right = castnode(targtype, right);
	    }
	    return targtype;	/* XXX - reduced from promote(targtype) */
	}
	if (targtype->constructor & POINTER &&
	    (isconst0(right) || (right->nodetype->constructor & POINTER &&
	     right->nodetype->nexttype->constructor & VOID)))
	    return targtype;
	if (right->nodetype->constructor & POINTER &&
	    (isconst0(left) || (targtype->constructor & POINTER &&
	     targtype->nexttype->constructor & VOID)))
	    return right->nodetype;
	error("mismatched types");
	fixnode(left);		/* XXX - better to fix the CONDOP */
	fixnode(right);
	return errtype;

    case ASSIGNOP:
	if (right->nodetype->constructor & VOID)
	{
	    error("assignment of void");
	    fixnode(right);
	}
	if (targtype->constructor & (ARRAY | FUNCTION | VOID))
	{
	    error("assignment to array, function or void");
	    fixnode(nodeptr);
	    return errtype;
	}
	if ((targtype->constructor | right->nodetype->constructor)
	    & STRUCTU && targtype != right->nodetype)
	{
	    error("assignment to/from struct/union of a different type");
	    fixnode(nodeptr);
	    return errtype;
	}
	if (!(left->flags & LVALUE))
	{
	    badlvalue(nodeptr);
	    return errtype;
	}
#if 0
	/* XXX - this is done at the lowest levels, with too many chances for
	 * errors.
	 */
	if (targtype != right->nodetype)
	    nodeptr->right = castnode(left->nodetype, right);
#endif
	break;
    case ANDABOP:
    case EORABOP:
    case MODABOP:
    case ORABOP:
	needint(right);
	needint(left);
	if (!(left->flags & LVALUE))
	{
	    badlvalue(nodeptr);
	    return errtype;
	}
	/* fall through to redundant check and code to cast to same types */
    case ADDABOP:
    case SUBABOP:
    case DIVABOP:
    case MULABOP:
	needscalar(right);
	needscalar(left);
	if (!(left->flags & LVALUE))
	{
	    badlvalue(nodeptr);
	    return errtype;
	}
	rscalar = right->nodetype->scalar;
	if ((lscalar = (targtype = left->nodetype)->scalar) & RSCALAR)
	{
	    if (!(rscalar & DOUBLE))
		nodeptr->right = castnode(dtype, right);
	}
	else if (lscalar & LONG)
	{
	    if (!(rscalar & LONG))
		nodeptr->right = castnode(rscalar & UNSIGNED
					  ? ultype : targtype, right);
	}
#ifdef I80386
	else if (i386_32 && lscalar & INT)
	{
	    if (rscalar & SHORT)
		nodeptr->right = castnode(rscalar & UNSIGNED
					  ? uitype : targtype, right);
	}
#endif
	break;
    case SLABOP:
    case SRABOP:
	needint(right);
	needint(left);
	if (!(left->flags & LVALUE))
	{
	    badlvalue(nodeptr);
	    return errtype;
	}
	if (!(right->nodetype->scalar & INT))
	    castiright(nodeptr);
	break;
    }
    return left->nodetype;
}

PRIVATE int redscalar(nodeptr)
register struct nodestruct *nodeptr;
{
    if (isnodecharconst(nodeptr))
	return CHAR;
    return nodeptr->nodetype->scalar;
}

PRIVATE struct nodestruct *unconvert(nodeptr)
struct nodestruct *nodeptr;
{
    if (nodeptr->nodetype->constructor & (ARRAY | FUNCTION))
	return castnode(pointype(nodeptr->nodetype->constructor & ARRAY ?
				 nodeptr->nodetype->nexttype :
				 nodeptr->nodetype), nodeptr);
    return nodeptr;
}
