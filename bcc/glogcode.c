/* glogcode.c - generate code for logical expressions for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"
#include "condcode.h"
#include "gencode.h"
#include "reg.h"
#include "scan.h"
#include "sizes.h"
#include "type.h"

#define cc_signed(cc) ((cc) >= 4 && (cc) < 8)

PRIVATE char oppcc[] =		/* opposite condition codes LT --> GE etc */
/*  EQ, NE, RA, RN, LT, GE, LE, GT, LO, HS, LS, HI,  indices */
{
    NE, EQ, RN, RA, GE, LT, GT, LE, HS, LO, HI, LS,
};

PRIVATE char reverscc[] =	/* reverse condition codes LT --> GT etc */
{
    EQ, NE, RN, RA, GT, LE, GE, LT, HI, LS, HS, LO,
};

PRIVATE char testcc[] =		/* test condition codes LS --> EQ etc */
{
    EQ, NE, RA, RN, LT, GE, LE, GT, RN, RA, EQ, NE,
};

PRIVATE char unsigncc[] =	/* unsigned condition codes LT --> LO etc */
{
    EQ, NE, RA, RN, LO, HS, LS, HI, LO, HS, LS, HI,
};

FORWARD void cmplocal P((struct symstruct *source, struct symstruct *target,
			  ccode_t *pcondtrue));
#ifdef MC6809
FORWARD void cmporsub P((struct symstruct *target));
FORWARD bool_pt cmpsmallconst P((value_t intconst, struct symstruct *target,
				  ccode_t *pcondtrue));
#endif
FORWARD void comparecond P((struct nodestruct *exp, label_no truelab,
			     label_no falselab, bool_pt nojump));
FORWARD void jumpcond P((struct nodestruct *exp, label_no truelab,
			  label_no falselab, bool_pt nojump));
FORWARD void loadlogical P((struct symstruct *source, label_no falselab));
FORWARD void logandcond P((struct nodestruct *exp, label_no truelab,
			    label_no falselab, bool_pt nojump));
FORWARD void logorcond P((struct nodestruct *exp, label_no truelab,
			   label_no falselab, bool_pt nojump));
FORWARD void reduceconst P((struct symstruct *source));
FORWARD void test P((struct symstruct *target, ccode_t *pcondtrue));
FORWARD void testcond P((struct nodestruct *exp, label_no truelab,
			  label_no falselab, bool_pt nojump));

PUBLIC void cmp(source, target, pcondtrue)
struct symstruct *source;
struct symstruct *target;
ccode_t *pcondtrue;
{
    label_no falselab;

    cmplocal(source, target, pcondtrue);
#if 0
#ifdef I80386
    if (i386_32)
    {
	if (*pcondtrue == LO)
	{
	    getlabel();
	    getlabel();
	    outnop2str("sbb\teax,eax");
	    outnop1str("inc eax");
	    target->storage = BREG;
	    target->type = ctype;
	    return;
	}
	if (*pcondtrue == HS)
	{
	    getlabel();
	    getlabel();
	    outnop2str("sbb\teax,eax");
	    outnop2str("neg eax");
	    target->storage = BREG;
	    target->type = ctype;
	    return;
	}
    }
#endif
#endif
    sbranch(oppcc[(int)*pcondtrue], falselab = getlabel());
    loadlogical(target, falselab);
}

PRIVATE void cmplocal(source, target, pcondtrue)
struct symstruct *source;
struct symstruct *target;
ccode_t *pcondtrue;
{
    scalar_t sscalar;
    scalar_t tempscalar;
    scalar_t tscalar;

    reduceconst(source);
    reduceconst(target);
    sscalar = source->type->scalar;
    tscalar = target->type->scalar;
    if ((source->storage != CONSTANT &&
	(target->storage == CONSTANT ||
	 (sscalar & CHAR && !(tscalar & CHAR)) ||
	 ((sscalar & CHAR || !(tscalar & CHAR)) &&
	 source->indcount == 0 && target->indcount != 0))) ||
	(tscalar & DLONG && target->indcount != 0))
    {
	swapsym(target, source);
	*pcondtrue = reverscc[(int)*pcondtrue];
	tempscalar = sscalar;
	sscalar = tscalar;
	tscalar = tempscalar;
    }
    if ((sscalar & CHAR && tscalar & CHAR &&
	(source->type != sctype || target->type != sctype)) ||
	(sscalar | tscalar) & UNSIGNED ||
	(source->type->constructor | target->type->constructor) &
	(ARRAY | POINTER))
	*pcondtrue = unsigncc[(int)*pcondtrue];
    if (source->type->scalar & DLONG)
    {
	longop(EQOP, source, target);
	return;
    }
    if (source->type->scalar & RSCALAR)
    {
	floatop(EQOP, source, target);
	return;
    }
    if (source->storage == CONSTANT)
    {
	if (sscalar & CHAR)
	{
	    if (tscalar & CHAR)
		*pcondtrue = unsigncc[(int)*pcondtrue];
	    else
	    {
		source->type = iscalartotype(sscalar);
		sscalar = source->type->scalar;
	    }
	}
	if (source->offset.offv == 0)
	{
	    test(target, pcondtrue);
	    return;
	}
#ifdef MC6809
	if (cmpsmallconst(source->offset.offv, target, pcondtrue))
	    return;
#endif
    }
    if (!(sscalar & CHAR) && tscalar & CHAR)
    {
	loadpres(target, source);
	extend(target);
    }
#ifndef MC6809
# define posindependent 0
#endif
    if (source->indcount == 0 && source->storage != CONSTANT &&
	(posindependent || source->storage != GLOBAL))
    {
	loadpres(source, target);
#ifdef MC6809
	push(source);
#endif
    }
    loadpres(target, source);
#ifdef MC6809
    cmporsub(target);
#else
    outcmp();
#endif
#ifdef I8088
    if (source->storage == GLOBAL && source->indcount == 0 &&
	!(target->storage & (AXREG | ALREG)))
	bumplc();
#endif
    movereg(source, target->storage);
}

#ifdef MC6809

PRIVATE void cmporsub(target)
struct symstruct *target;
{
    if (target->storage & ALLDATREGS)
	outsub();
    else
    {
	outcmp();
	if (target->storage != XREG)
	    bumplc();
    }
}

PRIVATE bool_pt cmpsmallconst(intconst, target, pcondtrue)
value_t intconst;
struct symstruct *target;
ccode_t *pcondtrue;
{
    store_pt targreg;

    if ((*pcondtrue == EQ || *pcondtrue == NE) &&
	!(target->storage & ALLDATREGS) && !(target->type->scalar & CHAR) &&
	isnegbyteoffset(intconst) &&
	(reguse & (XREG | YREG)) != (XREG | YREG))
    {
	targreg = XREG;
	if (reguse & XREG)
	    targreg = YREG;
	if (target->indcount != 0)
	    load(target, targreg);
	target->offset.offi -= (offset_T) intconst;
	loadreg(target, targreg);
	return TRUE;
    }
    return FALSE;
}

#endif

PRIVATE void comparecond(exp, truelab, falselab, nojump)
struct nodestruct *exp;
label_no truelab;
label_no falselab;
bool_pt nojump;			/* NB if nonzero, is ~0 so complement is 0 */
{
    ccode_t condtrue;
    store_t regmark;
    struct symstruct *source;
    offset_T spmark;
    struct symstruct *target;

    regmark = reguse;
    spmark = sp;
    bileaf(exp);
    target = exp->left.nodeptr->left.symptr;
    source = exp->right->left.symptr;	/* exp->right != NULL since cond */
    switch (exp->tag)
    {
    case GEOP:
	condtrue = GE;
	break;
    case GTOP:
	condtrue = GT;
	break;
    case LEOP:
	condtrue = LE;
	break;
    case EQOP:
	condtrue = EQ;
	break;
    case LTOP:
	condtrue = LT;
	break;
    case NEOP:
	condtrue = NE;
	break;
    }
    cmplocal(source, target, &condtrue);
    changesp(spmark, FALSE);
    reguse = regmark;
    if ((bool_t) nojump)
	lbranch(oppcc[(int)condtrue], falselab);
    else
	lbranch(condtrue, truelab);
}

PUBLIC void condop(exp)
struct nodestruct *exp;
{
    label_no exitlab;
    label_no falselab;
    struct nodestruct *falsenode;
    struct symstruct *falsesym;
    label_no truelab;
    struct nodestruct *truenode;
    struct symstruct *truesym;

    jumpcond(exp->left.nodeptr, truelab = getlabel(),
	     falselab = getlabel(), ~0);
    deflabel(truelab);
    makeleaf(truenode = exp->right->left.nodeptr);
    loadany(truesym = truenode->left.symptr);
    if (truesym->storage & reguse)
    {
	/* This can happen if truesym was a reg variable. */
	if (truesym->type->scalar & RSCALAR)
	    /* XXX - always happens for non-386 with 2 regs vars assigned. */
	    bugerror("loaded float or double into used reg");
	load(truesym, DREG);
    }
    falsenode = exp->right->right;
    if (/* falsenode->tag != LEAF || XXX */
	truesym->type != falsenode->left.symptr->type)
	cast(truenode->nodetype == falsenode->nodetype ?
	     truenode->nodetype : exp->nodetype, truesym);
    jump(exitlab = getlabel());
    deflabel(falselab);
    makeleaf(falsenode);
    falsesym = falsenode->left.symptr;
    if (falsesym->type != truesym->type)
	cast(truesym->type, falsesym);
    load(falsesym, truesym->storage);
    deflabel(exitlab);
    exp->tag = LEAF;
    exp->left.symptr = truesym;
}

PRIVATE void jumpcond(exp, truelab, falselab, nojump)
struct nodestruct *exp;
label_no truelab;
label_no falselab;
bool_pt nojump;			/* NB if nonzero, is ~0 so complement is 0 */
{
    switch (exp->tag)
    {
    case GEOP:
    case GTOP:
    case LEOP:
    case EQOP:
    case LTOP:
    case NEOP:
	comparecond(exp, truelab, falselab, nojump);
	break;
    case LOGANDOP:
	logandcond(exp, truelab, falselab, nojump);
	break;
    case LOGNOTOP:
	jumpcond(exp->left.nodeptr, falselab, truelab, ~nojump);
	break;
    case LOGOROP:
	logorcond(exp, truelab, falselab, nojump);
	break;
    default:
	testcond(exp, truelab, falselab, nojump);
	break;
    }
}

PUBLIC void jumpfalse(exp, label)
struct nodestruct *exp;
label_no label;
{
    label_no truelab;

    jumpcond(exp, truelab = getlabel(), label, ~0);
    deflabel(truelab);
}

PUBLIC void jumptrue(exp, label)
struct nodestruct *exp;
label_no label;
{
    label_no falselab;

    jumpcond(exp, label, falselab = getlabel(), 0);
    deflabel(falselab);
}

PRIVATE void loadlogical(source, falselab)
struct symstruct *source;
label_no falselab;
{
    label_no exitlab;
    struct symstruct *target;

    target = constsym((value_t) TRUE);
    target->type = ctype;
    loadreg(target, DREG);
    sbranch(RA, exitlab = getlabel());
    deflabel(falselab);
    target = constsym((value_t) FALSE);
    target->type = ctype;
    *source = *target;
    loadreg(source, DREG);
    outnlabel(exitlab);
}

PRIVATE void logandcond(exp, truelab, falselab, nojump)
struct nodestruct *exp;
label_no truelab;
label_no falselab;
bool_pt nojump;			/* NB if nonzero, is ~0 so complement is 0 */
{
    label_no andlab;

    andlab = getlabel();
    jumpcond(exp->left.nodeptr, andlab, falselab, ~0);
    deflabel(andlab);
    jumpcond(exp->right, truelab, falselab, nojump);
}

PUBLIC void logop(exp)
struct nodestruct *exp;
{
    label_no falselab;
    struct symstruct *target;
    label_no truelab;

    jumpcond(exp, truelab = getlabel(), falselab = getlabel(), ~0);
    deflabel(truelab);
    target = constsym((value_t) 0);	/* anything, loadlogical makes B reg */
    target->type = ctype;
    loadlogical(target, falselab);
    exp->tag = LEAF;
    exp->left.symptr = target;
}

PRIVATE void logorcond(exp, truelab, falselab, nojump)
struct nodestruct *exp;
label_no truelab;
label_no falselab;
bool_pt nojump;			/* NB if nonzero, is ~0 so complement is 0 */
{
    label_no orlab;

    orlab = getlabel();
    jumpcond(exp->left.nodeptr, truelab, orlab, 0);
    deflabel(orlab);
    jumpcond(exp->right, truelab, falselab, nojump);
}

PRIVATE void reduceconst(source)
struct symstruct *source;
{
    if (source->storage == CONSTANT && ischarconst(source->offset.offv) &&
	(source->type->scalar & (CHAR | SHORT | INT | DLONG)) != DLONG)
    {
	if (source->type->scalar & UNSIGNED)
	    source->type = uctype;
	else
	    source->type = ctype;
    }
}

PRIVATE void test(target, pcondtrue)
struct symstruct *target;
ccode_t *pcondtrue;
{
#ifdef I8088
    store_t targreg;
#endif

    *pcondtrue = testcc[(int)*pcondtrue];
    if (target->type->scalar & DLONG)
    {
	long1op(EQOP, target);
	return;
    }
    if (target->type->scalar & RSCALAR)
    {
	float1op(EQOP, target);
	return;
    }
#ifdef I8088
    if (target->indcount != 0 ||
	(target->storage == LOCAL && target->offset.offi != sp))
	load(target, DREG);
    if (target->storage == GLOBAL)
	load(target, getindexreg());
    if (target->type->scalar & CHAR)
	load(target, DREG);
    targreg = target->storage;
    if (target->offset.offi != 0 && cc_signed(*pcondtrue))
	load(target, targreg);

#ifdef I80386
    /* Extension was not done in exptree for the == 0 case, to allow
     * optimization here - which we don't do for shorts.  (foo--) is
     * newfoo == -1 here and used to be missed.
     */
    if (i386_32 && target->type->scalar & SHORT)
	extend(target);
#endif

    if (target->offset.offi == 0)
    {
	outtest();
	outregname(targreg);
	outcomma();
	outnregname(targreg);
	return;
    }
    outcmp();
    outimadj(-target->offset.offi, targreg);
#endif
#ifdef MC6809
    if (target->indcount != 0 ||
	target->storage == LOCAL && target->offset.offi != sp)
    {
	load(target, DREG);
	return;
    }
    if (cmpsmallconst(0, target, pcondtrue))
	return;
    if (target->storage == GLOBAL)
	load(target, getindexreg());
    if (target->type->scalar & CHAR)
	load(target, DREG);
    if (target->offset.offi != 0 && cc_signed(*pcondtrue))
	load(target, target->storage);
    if (target->type->scalar & CHAR)
    {
	if (target->offset.offi == 0)
	{
	    outtest();
	    outnregname(BREG);
	    return;
	}
	outcmp();
	outimadj(-target->offset.offi, BREG);
    }
    else
    {
	cmporsub(target);
	outimadj(-target->offset.offi, target->storage);
    }
#endif
}

/* test expression and jump depending on NE/EQ */

PRIVATE void testcond(exp, truelab, falselab, nojump)
struct nodestruct *exp;
label_no truelab;
label_no falselab;
bool_pt nojump;			/* NB if nonzero, is ~0 so complement is 0 */
{
    ccode_t condtrue;
    struct symstruct *source;

    makeleaf(exp);
    source = exp->left.symptr;
    reduceconst(source);
    if (source->storage != CONSTANT)
    {
	condtrue = NE;
	test(source, &condtrue);
	if ((bool_t) nojump)
	    lbranch(oppcc[(int)condtrue], falselab);
	else
	    lbranch(condtrue, truelab);
    }
    else if (source->offset.offi == 0)
    {
	if ((bool_t) nojump)
	    jump(falselab);
    }
    else if (!(bool_t) nojump)
	jump(truelab);
}
