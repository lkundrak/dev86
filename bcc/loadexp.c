/* loadexp.c - load expressions for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"
#include "byteord.h"
#include "gencode.h"
#include "parse.h"
#include "reg.h"
#include "sc.h"
#include "scan.h"
#include "sizes.h"
#include "table.h"
#include "type.h"

PUBLIC value_t constexpression()
{
    struct nodestruct *etmark;
    struct nodestruct *exp;
    struct symstruct *exprmark;
    struct symstruct *symptr;

    etmark = etptr;
    exprmark = exprptr;
    exp = assignment_exp();
    etptr = etmark;
    exprptr = exprmark;
    if (exp->tag == LEAF && (symptr = exp->left.symptr)->storage == CONSTANT
	&& symptr->type->scalar & ISCALAR)
	return symptr->offset.offv;
    error("constant expression required");
    return 1;
}

PUBLIC void initexpression(type)
struct typestruct *type;
{
    struct nodestruct *etmark;
    struct nodestruct *exp;
    struct symstruct *exprmark;
    struct nodestruct *lhs;
    struct symstruct *symptr;
    uoffset_T value;

    if (gvarsymptr->storage != GLOBAL)
	reslocals();
    exprmark = exprptr;
    etmark = etptr;
    exp = assignment_exp();
    if (gvarsymptr->storage != GLOBAL)
    {
	lhs = leafnode(exprsym(gvarsymptr));
	if (!(lhs->nodetype->constructor & (ARRAY | FUNCTION | VOID)))
	    lhs->flags = LVALUE;
	makeleaf(node(ASSIGNOP, lhs, exp));
    }
    else if (exp->tag != LEAF ||
	     ((symptr = exp->left.symptr)->storage != CONSTANT &&
	     (symptr->storage != GLOBAL || symptr->indcount != 0 ||
	      type->scalar & DLONG)) ||
	     (type->constructor | (symptr->type->constructor & ~FUNCTION)) &
	     ~(ARRAY | POINTER))
	error("initializer too complicated");
    else
    {
	if ((symptr->type->scalar | type->scalar) & RSCALAR)
	{
	    /* Can only afford castnode if result known constant. */
	    exp = castnode(type, exp);
	    symptr = exp->left.symptr;
	}
	if (type->scalar & RSCALAR)
	{
	    if (type->scalar & FLOAT)
	    {
		float val;

		val = *symptr->offset.offd;
		deflong(((uoffset_T *) &val)[0]);
	    }
	    else
	    {
		deflong(((uoffset_T *) symptr->offset.offd)[0]);
		deflong(((uoffset_T *) symptr->offset.offd)[1]);
	    }
	    etptr = etmark;	/* XXX - stuff from end of function */
	    exprptr = exprmark;
	    return;
	}
	if (type->typesize == 1)
	    defbyte();
	else if (type->typesize == 2)
	    defword();
#ifdef I8088
	else if (!(type->scalar & DLONG))
	    defdword();
#endif
	switch (symptr->storage)
	{
	case CONSTANT:
	    value = (uoffset_T) symptr->offset.offv;
	    if (type->scalar & DLONG)
	    {
		deflong(value);
		break;
	    }
	    /* XXX - put sizes in type struct to avoid tests */
	    if (type->scalar & CHAR)
		value &= CHMASKTO;
	    else if (type->scalar & SHORT)
		value &= shortmaskto;
	    else if (type->scalar & INT)
		value &= intmaskto;
	    /* XXX - no longmaskto */
	    outnhex(value);
	    break;
	case GLOBAL:
	    if (symptr->flags & LABELLED)
	    {
		outlabel(symptr->name.label);
		outplus();
		outnhex((uoffset_T) symptr->offset.offi);
		break;
	    }
	    if (*symptr->name.namep == 0)	/* constant address */
	    {
		outnhex((uoffset_T) symptr->offset.offi);
		break;
	    }
	    outccname(symptr->name.namep);
	    if (symptr->offset.offi != 0)
	    {
		if (symptr->offset.offi > 0)
		    outplus();
		outshex(symptr->offset.offi);
	    }
	    outnl();
	    break;
	}
    }
    etptr = etmark;
    exprptr = exprmark;
}

PUBLIC struct typestruct *loadexpression(targreg, targtype)
store_pt targreg;
struct typestruct *targtype;
{
    struct nodestruct *etmark;
    struct nodestruct *exp;
    struct symstruct *exprmark;

    etmark = etptr;
    exprmark = exprptr;
    exp = expression();
    if (targtype != NULL)
	exp = castnode(targtype, exp);
    makeleaf(exp);
    if (targtype == NULL)	/* this is for a switch */
    {
	targtype = exp->left.symptr->type;
	if (!(targtype->scalar & (CHAR | INT))
	    && (targtype->scalar & (LONG | DLONG)) != LONG)
	{
	    if (targtype->scalar & SHORT)
		targtype = promote(targtype);
	    else
	    {
		error("non-integral selector in switch");
		targtype = itype;
	    }
	    makeleaf(exp = castnode(targtype, exp));
	}
    }
    load(exp->left.symptr, targreg);	/* resets stack if value was there */
    etptr = etmark;
    exprptr = exprmark;
    return exp->left.symptr->type;
}
