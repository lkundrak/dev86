/* dbnode.c - print debug messages for operators for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"

#ifdef DBNODE
#include "gencode.h"
#include "reg.h"
#include "sc.h"
#include "scan.h"
#include "type.h"

PRIVATE char *opname[LASTOP - FIRSTOP + 1] =	/* operator names */
{				/* order must agree with op.h */
    "cond?",
    "or",
    "eor",
    "and",
    "gt", "lt",
    "add",
    "div", "mod",
    "lognot", "not",
    "strucelt", "strucptr",
    "eq",
    "addab", "andab", "divab", "eorab", "modab", "mulab", "orab",
    "slab", "srab", "subab",
    "comma",
    "cond:",
    "logor",
    "logand",
    "logeq",
    "ne",
    "ge", "le",
    "sl", "sr",
    "sub",
    "mul",
    "address", "cast", "indirect", "neg",
    "predec", "preinc", "postdec", "postinc",
    "func", "list", "rootlist",
    "leaf",
    "ptraddab", "ptradd", "ptrsub",
};

FORWARD void outindchars P((int byte, indn_pt count));

PUBLIC void dbitem(item)
struct symstruct *item;
{
    dbtype(item->type);
    if (item->storage == NOSTORAGE)
    {
	outbyte(' ');
	outstr(item->name.namep + 2);
	outstr(" (offset ");
	outshex(item->offset.offi);
	outbyte(')');
	return;
    }
    if (item->storage == LOCAL)
    {
	outbyte(' ');
	if (item->flags == TEMP)
	    outstr("(temp)");
	else
	    outstr(item->name.namep);
    }
    outstr(" = ");
    outindchars('[', item->indcount);
    switch (item->storage)
    {
    case CONSTANT:
	outstr("const ");
	if (item->type->scalar & RSCALAR)
	    outstr("(whatever)");
	else if (item->type->scalar & UNSIGNED)
	    outuvalue((uvalue_t) item->offset.offv);
	else
	    outvalue(item->offset.offv);
	break;
    case BREG:
    case DREG:
    case INDREG0:
    case INDREG1:
    case INDREG2:
#ifdef DATREG1
    case DATREG1:
#endif
#ifdef DATREG2
    case DATREG2:
#endif
	outregname(item->storage);
	if (item->level == OFFKLUDGELEVEL)
	{
	    outplus();
	    if (item->flags & LABELLED)
		outlabel(item->name.label);
	    else
		outccname(item->name.namep);
	}
	break;
    case LOCAL:
	outbyte('S');
	if (sp <= 0)
	    outplus();
	outshex(-sp);
	break;
    case GLOBAL:
	if (item->flags & LABELLED)
	    outlabel(item->name.label);
	else
	    outstr(item->name.namep);
	break;
    default:
	outstr("bad storage (");
	outhex((uoffset_T) item->storage);
	outbyte(')');
	outstr(" offset ");
    }
    if (item->storage != CONSTANT)
    {
	if (item->offset.offi >= 0)
	    outplus();
	outshex(item->offset.offi);
    }
    outindchars(']', item->indcount);
}

PUBLIC void dbtype(type)
struct typestruct *type;
{
    for ( ; type != NULL; type = type->nexttype)
    {
	outbyte(' ');
	switch (type->constructor)
	{
	case ARRAY:
	    outbyte('[');
	    outhex(type->typesize / type->nexttype->typesize);
	    outbyte(']');
	    break;
	case FUNCTION:
	    outstr("()");
	    break;
	case POINTER:
	    outbyte('*');
	    break;
	case STRUCTU:
	    outstr("struct ");
	default:
	    if (type->scalar & UNSIGNED)
		outstr("unsigned ");
	    outstr(type->tname);
	    break;
	}
    }
}

PUBLIC void dbnode(exp)		/* sub-nodes must be leaves */
struct nodestruct *exp;
{
    if (!dbnodeon)
	return;
    outstr("! Debug: ");
    if (exp->tag < FIRSTOP && exp->tag > LASTOP)
	outstr("unknown op");
    else
	outstr(opname[exp->tag - FIRSTOP]);
    if (exp->right != NULL && exp->tag != FUNCOP &&
	exp->tag != LISTOP && exp->tag != ROOTLISTOP)
    {
	dbitem(exp->right->left.symptr);
	outstr(" to");
    }
    dbitem(exp->left.nodeptr->left.symptr);
    outstr(" (used reg = ");
    if (reguse & INDREG0)
	outregname(INDREG0);
    if (reguse & INDREG1)
	outregname(INDREG1);
    if (reguse & INDREG2)
	outregname(INDREG2);
    outnstr(")");
}

PUBLIC void dbnodeswap()
{
    if (dbnodeon)
	outnstr("! Debug: expression subtree swapping");
}

PRIVATE void outindchars(byte, count)
int byte;
indn_pt count;
{
    while (count--)
	outbyte(byte);
}

#endif /* DBNODE */
