/* hardop.c - hardware operations for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"
#include "byteord.h"
#include "condcode.h"
#include "gencode.h"
#include "reg.h"
#include "sc.h"
#include "scan.h"
#include "sizes.h"
#include "type.h"

FORWARD void sub1 P((struct symstruct *source, struct symstruct *target));

PUBLIC void add(source, target)
struct symstruct *source;
struct symstruct *target;
{
    scalar_t sscalar;

    if (source->indcount == 0 && source->storage != CONSTANT &&
	(target->indcount != 0 || target->storage & ALLDATREGS))
	swapsym(target, source);
    if ((sscalar = source->type->scalar) & DLONG)
    {
	longop(ADDOP, source, target);
	return;
    }
    if (sscalar & RSCALAR)
    {
	floatop(ADDOP, source, target);
	return;
    }
    if (source->storage == CONSTANT)
    {
	extend(target);
	if (target->indcount != 0 || target->storage & reguse)
	    loadany(target);
	target->offset.offi += (offset_T) source->offset.offv;
    }
    else if (source->indcount == 0)
    {
	/* target is also direct */
	sscalar |= target->type->scalar;	/* remember if unsigned/long */
	target->type = pctype;	/* fake to use indexadr() */
	indexadr(source, target);
    }
    else
    {
	/* source is indirect and not DREG */
	extend(target);
	load(target, DREG);
	outadd();
	movereg(source, DREG);
	if (source->type->scalar & CHAR)
	    adc0();
    }
    target->type = iscalartotype(target->type->scalar | sscalar);
}

PUBLIC void incdec(op, source)
op_pt op;
struct symstruct *source;
{
    offset_T bump;
    bool_t postflag;
    store_t regmark;
    struct symstruct *target;
    struct symstruct targ;
#ifdef MC6809
    store_pt targreg;
#endif

    *(target = &targ) = *source;
    bump = 1;
    if (targ.type->constructor & POINTER)
	bump = targ.type->nexttype->typesize;
    if (op == PREDECOP || op == POSTDECOP)
	bump = -bump;
    postflag = FALSE;
    if (op == POSTDECOP || op == POSTINCOP)
	postflag = TRUE;
    reguse |= targ.storage;
    if (targ.type->scalar & DLONG)	/* cannot be direct */
    {
	if (postflag)
	{
	    regmark = reguse;
	    if (((reguse |= OPREG) & allindregs) == allindregs)
	    {
		saveopreg();
		load(source, OPREG);
		pushreg(source->storage);
		restoreopreg();
	    }
	    else
	    {
		loadany(source);
		reguse = regmark | source->storage;
		saveopreg();
	    }
	}
	else
	    saveopreg();
	pointat(target);
	switch (op)
	{
	case PREDECOP:
	case POSTDECOP:
	    call("ldec");
	    break;
	case PREINCOP:
	case POSTINCOP:
	    call("linc");
	    break;
	}
	outlongendian();
	restoreopreg();
	if (postflag && source->storage == OPREG)
	    poplist(OPREG);
	return;
    }
    if (targ.type->scalar & RSCALAR)
    {
	saveopreg();
	pointat(target);
	switch (op)
	{
	case PREDECOP:
	    call("Fpredec");
	    break;
	case PREINCOP:
	    call("Fpreinc");
	    break;
	case POSTDECOP:
	    call("Fpostdec");
	    break;
	case POSTINCOP:
	    call("Fpostinc");
	    break;
	}
	outntypechar(targ.type);
	restoreopreg();
	if (postflag)
	{
	    justpushed(source);
	    source->type = targ.type;
	}
	return;
    }
    loadany(source);
#ifdef MC6809
    if (postflag && targ.flags != REGVAR &&
	!(source->storage & ALLDATREGS) &&
	((reguse |= source->storage) & allindregs) != allindregs)
    {
	targreg = getindexreg();
	outlea();
	outregname(targreg);
	outtab();
	outshex(bump);
	outncregname(source->storage);
    }
    else
    {
	addconst(bump, targreg = source->storage);
	if (postflag)
	    source->offset.offi = -bump;
    }
    storereg(targreg, target);
    target->storage = targreg;
    target->offset.offi = 0;
#else
    addconst(bump, source->storage);
    if (postflag)
	source->offset.offi = -bump;
    storereg(source->storage, target);
    target->storage = source->storage;
    target->offset.offi = 0;
#endif
}

PUBLIC void neg(target)
struct symstruct *target;
{
    scalar_t scalar;
    struct symstruct *source;

    if ((scalar = target->type->scalar) & DLONG)
	long1op(NEGOP, target);
    else if (scalar & RSCALAR)
	float1op(NEGOP, target);
    else
    {
	if (scalar & SHORT)
	    extend(target);
	if (!(target->storage & ALLDATREGS))
	{
	    /* load 0, subtract is shorter than load, negate */
	    /* if no hardware integer negate; about the same if there is */
	    sub1(target, source = constsym((value_t) 0));
	    *target = *source;
	}
	else
	{
	    load(target, DREG);
	    negreg(target->storage);
	    target->storage = DREG;
	}
	target->type = iscalartotype(scalar);
    }
}

PUBLIC void not(target)
struct symstruct *target;
{
    if (target->type->scalar & DLONG)
	long1op(NOTOP, target);
    else
    {
	extend(target);
	load(target, DREG);
	comDreg();
    }
}

/* 1-byte ops like AND acting on integers (not both constant) */

PUBLIC void op1(op, source, target)
op_pt op;
struct symstruct *source;
struct symstruct *target;
{
    char *opstr;
#ifdef OP1
# if MAXINDIRECT > 1
    indn_t indcount;
# endif
#endif
    bool_t resultchar;
    scalar_t resultscalar;
    scalar_t sscalar;
    scalar_t tscalar;

    if ((sscalar = source->type->scalar) & DLONG)
    {
	longop(op, source, target);
	return;
    }
    /* Emergency fix. The types of constants should be reduced here and in
     * other low-level routines anyway, and not in exptree.c and table.c,
     * and for the 80386 and maybe the 8086 they would be better not
     * reduced.
     */
    if (source->storage == CONSTANT && ischarconst(source->offset.offv))
    {
	if (sscalar & UNSIGNED)
	    source->type = uctype;
	else
	    source->type = ctype;
	sscalar = source->type->scalar;
    }
    tscalar = target->type->scalar;
    if (target->storage == CONSTANT && ischarconst(target->offset.offv))
    {
	if (sscalar & UNSIGNED)
	    target->type = uctype;
	else
	    target->type = ctype;
	tscalar = target->type->scalar;
    }
    resultscalar = sscalar | tscalar;
    if (op != ANDOP)
	resultchar = sscalar & tscalar & CHAR;
    else if ((resultchar = (sscalar | tscalar) & CHAR) != 0 &&
	     source->storage == CONSTANT)
    {
	source->offset.offv &= CHMASKTO;
	if (sscalar & UNSIGNED)
	    source->type = uctype;
	else
	    source->type = ctype;
	sscalar = source->type->scalar;
    }
    if (target->indcount != 0 &&
	((tscalar & CHAR && !(sscalar & CHAR) &&
       op != ANDOP) || (source->indcount == 0 && source->storage != CONSTANT)))
    {
	swapsym(target, source);
	sscalar = tscalar;
	tscalar = target->type->scalar;
    }
    if (source->indcount == 0 && source->storage != CONSTANT)
    {
	loadpres(source, target);
	push(source);
    }
#if MAXINDIRECT > 1
    else if (source->indcount >= MAXINDIRECT && !(sscalar & CHAR))
    {
	address(source);
	if (!(target->storage & ALLDATREGS))
	    preserve(target);
	load(source, getindexreg());
	indirec(source);
    }
#endif
    if (!(tscalar & CHAR) &&
	op == ANDOP && sscalar & CHAR && target->indcount == 1)
	cast(ctype, target);
    if (!(target->storage & ALLDATREGS) || target->indcount != 0)
	pres2(target, source);
    load(target, DREG);
    opstr = opstring(op);
    if (source->storage == CONSTANT && op == ANDOP)
	andconst((offset_T) source->offset.offv);
#ifdef OP1
    else if (tscalar & CHAR && !(sscalar & CHAR) && op != ANDOP)
	outload();
    else
	outop2str(opstr);
#else /* OP1 */
    else
    {
	if (tscalar & CHAR && !(sscalar & CHAR) && op != ANDOP)
	    extend(target);
	outop2str(opstr);
    }
#endif /* OP1 */
    if (source->storage == CONSTANT)
    {
	if (op != ANDOP)
	{
#ifdef OP1
	    if (!(sscalar & CHAR))
	    {
		outhiaccum();
		outncimmadr((offset_T) ((uoffset_T) source->offset.offv
					>> (INT16BITSTO - CHBITSTO)));
		outop2str(opstr);
	    }
	    outregname(BREG);
	    outncimmadr((offset_T) source->offset.offv & CHMASKTO);
#else /* OP1 */
	    if (!(sscalar & CHAR))
	    {
		outregname(DREG);
		bumplc();
	    }
	    else
		outregname(BREG);
# ifdef I80386
	    if (i386_32 && !(sscalar & CHAR))
		bumplc2();
# endif
	    outncimmadr((offset_T) source->offset.offv);

#endif /* OP1 */
	}
    }
    else if (sscalar & CHAR)
    {
	outregname(BREG);
	outopsep();
	outadr(source);
    }
    else
    {
#ifdef MC6809
	source->type = ctype;	/* fool outadr to avoid ,S++ */
#endif
#ifdef OP1
	if (!(tscalar & CHAR) || op != ANDOP)
	{
	    outhiaccum();
# if MAXINDIRECT > 1
	    indcount = source->indcount;
# endif
	    outopsep();
# if INT_BIG_ENDIAN == 0
	    ++source->offset.offi;
# endif
	    outadr(source);
# if INT_BIG_ENDIAN == 0
	    --source->offset.offi;
# endif
# if MAXINDIRECT > 1
	    source->indcount = indcount;
# else
	    source->indcount = 1;
# endif
	    outop2str(opstr);
	}
	outregname(BREG);
	outopsep();
# if INT_BIG_ENDIAN
	++source->offset.offi;
# endif
	outadr(source);
    }
#else /* OP1 */
	if (!(tscalar & CHAR) || op != ANDOP)
	    outregname(DREG);
	else
	    outregname(BREG);
	outopsep();
	outadr(source);
    }
#endif /* OP1 */
    if (resultchar)
    {
	target->storage = BREG;
	if (resultscalar & UNSIGNED)
	    target->type = uctype;
	else
	    target->type = ctype;
    }
    else
    {
	target->storage = DREG;
	target->type = iscalartotype(resultscalar);
    }
}

PUBLIC void ptrsub(source, target)
struct symstruct *source;
struct symstruct *target;
{
    label_no exitlab;
    uoffset_T factor;
    label_no usignlab;

    if (source->indcount == 0 && source->storage != CONSTANT)
    {
	loadpres(source, target);
	push(source);
    }
    if (target->indcount == 0)
    {
	pres2(target, source);
	load(target, DREG);
    }
    factor = target->type->nexttype->typesize;
    source->type = target->type = itype;
    sub1(source, target);
    if (factor != 1)
    {
	pushlist(CCREG);
	sbranch(HS, usignlab = getlabel());	/* HS == no carry */
	negDreg();
	outnlabel(usignlab);
	target->type = uitype;
	softop(DIVOP, constsym((value_t) factor), target);
	target->type = itype;
	poplist(CCREG);
	sbranch(HS, exitlab = getlabel());
	negDreg();
	outnlabel(exitlab);
    }
}

PUBLIC void sub(source, target)
struct symstruct *source;
struct symstruct *target;
{
    scalar_t sscalar;

    if ((sscalar = source->type->scalar) & DLONG)
    {
	longop(SUBOP, source, target);
	return;
    }
    if (sscalar & RSCALAR)
    {
	floatop(SUBOP, source, target);
	return;
    }
    if (source->storage == CONSTANT)
    {
	extend(target);
	if (target->indcount != 0 || target->storage & reguse)
	    loadany(target);
	target->offset.offi -= (offset_T) source->offset.offv;
    }
    else
	sub1(source, target);
    target->type = iscalartotype(target->type->scalar | sscalar);
}

PRIVATE void sub1(source, target)
struct symstruct *source;
struct symstruct *target;
{
    if (source->storage == CONSTANT)
	source->type = itype;
    else if (source->indcount == 0)
    {
	loadpres(source, target);
	push(source);
    }
    extend(target);
    load(target, DREG);
    outsub();
    movereg(source, DREG);
    if (source->type->scalar & CHAR)
	sbc0();
}
