/* preserve.c - preserve opererands or registers in use for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"
#include "gencode.h"
#include "reg.h"
#include "type.h"
#include "scan.h"

/* change stack ptr without changing condition codes */

PUBLIC void changesp(newsp, absflag)
offset_T newsp;
bool_pt absflag;
{
    if (newsp != sp || ((bool_t) absflag && switchnow != NULL))
    {
#ifdef FRAMEPOINTER
	if (newsp != framep || (!(bool_t) absflag && switchnow != NULL))
	{
            int off;
	    outleasp();
	    if (!(bool_t) absflag && switchnow != NULL)
		outswoffset(off = newsp);
	    else
		outoffset(off = newsp - framep);
#ifndef NO_DEL_PUSH
	    if (optimise && !callersaves && off < 0)
	    {
	        outstr("+");
	        outstr(funcname);
	        outstr(".off");
	    }
#endif
	    outindframereg();
	    outnl();
	}
	else
	    regtransfer(FRAMEREG, STACKREG);
	sp = newsp;
	if (framep == 0)
	    bugerror("no frame pointer");
#else
# ifdef I8088
	outleasp();
	outoffset(newsp - sp);
	outindstackreg();
	outnl();
# else
	modstk(newsp);		/* this should preserve CC */
# endif
#endif /* FRAMEPOINTER */
    }
}

/* load source to any while preserving target */

PUBLIC void loadpres(source, target)
struct symstruct *source;
struct symstruct *target;
{
    store_t regmark;

    if (target->storage & ALLDATREGS)
    {
	if (source->type->scalar & CHAR)
	{
	    push(target);
	    load(source, DREG);
	}
	else
	    load(source, getindexreg());
    }
    else
    {
	regmark = reguse;
	reguse |= target->storage;
	loadany(source);
	reguse = regmark;
    }
}

/* change stack ptr */

PUBLIC void modstk(newsp)
offset_T newsp;
{
    if (newsp != sp)
    {
#ifdef FRAMEPOINTER
	if (newsp != framep || framep == 0 || switchnow != NULL)
	    addconst(newsp - sp, STACKREG);
	else
	    regtransfer(FRAMEREG, STACKREG);
#else
# ifdef I8088
	addconst(newsp - sp, STACKREG);
# else
	outleasp();
	outoffset(newsp - sp);
	outncspregname();
# endif
#endif
	sp = newsp;
    }
}

/* preserve target without changing source */

PUBLIC void pres2(source, target)
struct symstruct *source;
struct symstruct *target;
{
    if (target->storage & allregs)
    {
	if (target->storage & (allregs - allindregs) /* XXX */ ||
	    (target->indcount == 0 &&
	     target->type->scalar & (DLONG | RSCALAR)))
	    push(target);	/* XXX - perhaps not float */
	else if (((target->storage | reguse) & allindregs) == allindregs)
	{
	    loadpres(target, source);
	    push(target);
	}
	else
	    reguse |= target->storage;
    }
}

/* preserve source */

PUBLIC void preserve(source)
struct symstruct *source;
{
    if (source->storage & allregs)
    {
	if (source->storage & (allregs - allindregs) /* XXX */ ||
	    ((source->storage | reguse) & allindregs) == allindregs)
	    push(source);
	else
	    reguse |= source->storage;
    }
}

/* preserve lvalue target without changing source or target */

PUBLIC store_pt preslval(source, target)
struct symstruct *source;
struct symstruct *target;
{
    store_pt regpushed;

    if (target->indcount == 0)
	reguse &= ~target->storage;
    else
	reguse = (target->storage | reguse) & allindregs;
    if (!((source->type->scalar | target->type->scalar) & (DLONG | RSCALAR))
	|| reguse != allindregs)
	return 0;		/* XXX - perhaps not float */
    reguse = source->storage | target->storage;	/* free one other than s/t */
    pushreg(regpushed = getindexreg());
    reguse = ~(store_t) regpushed & allindregs;
    return regpushed;
}

PUBLIC void recovlist(reglist)
store_pt reglist;
{
    poplist(reglist);
    reguse |= (store_t) reglist;
}

#ifdef I8088
PRIVATE smalin_t regoffset[] = {0, 0, 0, 1, 2, 3, 0, 0, 0, 4, 5};
 /* CONSTANT, BREG, ax = DREG, bx = INDREG0, si = INDREG1, di = INDREG2 */
 /* LOCAL, GLOBAL, STACKREG, cx = DATREG1, dx = DATREG2 */
#endif
#ifdef MC6809
PRIVATE smalin_t regoffset[] = {0, 0, 0, 1, 3, 2};
 /* CONSTANT, BREG, DREG, XREG = INDREG0, UREG = INDREG1, YREG = INDREG2 */
#endif

PUBLIC void savereturn(savelist, saveoffset)
store_pt savelist;
offset_T saveoffset;
{
    store_t reg;
    smalin_t *regoffptr;
    offset_T spoffset;

    if (savelist == 0)
	return;
#ifdef MC6809 /* must check this */
    if (savelist == XREG || savelist == INDREG1)
	saveoffset -= accregsize;	/* patch for DREG/YREG not saved */
#endif
    for (reg = 1, regoffptr = regoffset; reg != 0; ++regoffptr, reg <<= 1)
	if (reg & savelist)
	{
	    outstore();
	    spoffset = saveoffset + *regoffptr * maxregsize;
#ifdef I8088
# ifdef FRAMEPOINTER
	    if (switchnow != NULL)
		outswoffset(spoffset);
	    else
		outoffset(spoffset - framep);
	    outindframereg();
# else
	    outoffset(spoffset - sp);
	    outindstackreg();
# endif
	    outncregname(reg);
#endif
#ifdef MC6809
	    if (reg == YREG)
		bumplc();
	    outregname(reg);
	    outtab();
	    outoffset(spoffset - sp);
	    outncspregname();
#endif
	}
}
