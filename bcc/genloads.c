/* genloads.c - generate loads of registers and memory for bcc */

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

FORWARD void badaddress P((void));
FORWARD void blockpush P((struct symstruct *source));
FORWARD void loadadr P((struct symstruct *source, store_pt targreg));
FORWARD void loadlongindirect P((struct symstruct *source, store_pt targreg));
FORWARD void outnamoffset P((struct symstruct *adr));
FORWARD void outnnadr P((struct symstruct *adr));
FORWARD fastin_pt pushpull P((store_pt reglist, bool_pt pushflag));

PUBLIC void addoffset(source)
struct symstruct *source;
{
#ifdef I8088
    if (source->level == OFFKLUDGELEVEL)
    {
	outadd();
	outregname(source->storage);
	outcomma();
        outimmed();
	outnamoffset(source);
	outnl();
	if (source->storage & (AXREG | ALREG))
	    unbumplc();
	if (source->level == OFFKLUDGELEVEL)
	    source->level = EXPRLEVEL;
	source->offset.offi = 0;
    }
    else
#endif
    if (source->offset.offi != 0)
    {
	addconst(source->offset.offi, source->storage);
	source->offset.offi = 0;
    }
}

PUBLIC void address(source)
struct symstruct *source;
{
    if (source->indcount == 0)
	bugerror("taking address of non-lvalue");
    else
    {
	if (source->type->constructor & (ARRAY | FUNCTION))
	    bugerror("botched indirect array or function");
	else if (--source->indcount == 0 && source->storage == GLOBAL &&
		 !(source->flags & LABELLED) && *source->name.namep == 0)
	    source->storage = CONSTANT;
	source->type = pointype(source->type);
    }
}

PRIVATE void badaddress()
{
    bugerror("bad address");
}

PRIVATE void blockpush(source)
struct symstruct *source;
{
    struct symstruct *length;
    offset_T spmark;
    uoffset_T typesize;

    typesize = source->type->typesize;
    length = constsym((value_t) typesize);
    length->type = uitype;
    address(source);
    modstk(spmark = sp - (offset_T) typesize);
#ifdef STACKREG
    regtransfer(STACKREG, DREG);
#else
#ifdef MC6809 /* XXX ? */
    regtransfer(LOCAL, DREG);
#else
#include "need STACKREG and stackregstr"
#endif
#endif
    push(length);
    push(source);
    pushreg(DREG);
    call("_memcpy");
    outnl();
    modstk(spmark);
    indirec(source);
}

PUBLIC void exchange(source, target)
struct symstruct *source;
struct symstruct *target;
{
    store_t tempreg;

    regexchange(source->storage, target->storage);
    tempreg = target->storage;
    target->storage = source->storage;
    source->storage = tempreg;
}

/*-----------------------------------------------------------------------------
	getindexreg()
	returns the "best" available index register
-----------------------------------------------------------------------------*/

PUBLIC store_pt getindexreg()
{
    if (!(reguse & INDREG0))
	return INDREG0;
    if (!(reguse & INDREG1))
	return INDREG1;
    if (!(reguse & INDREG2))
	return INDREG2;
#if NOTFINISHED
#ifdef I80386
    if (i386_32)
    {
	if (!(reguse & DATREG1))
	    return DATREG1;
	if (!(reguse & DATREG2))
	    return DATREG2;
    }
#endif
#endif
    bugerror("out of index regs");
    return 0;
}

/*-----------------------------------------------------------------------------
	indexadr(index leaf, pointer leaf)
	is used by the index and add and subtract (pointer) routines
	it handles expressions like
		pointer + index
		&array[index]
	the target becomes register direct with offset
	(except for index = 0, when nothing is changed)
	constant indexes are optimised by leaving them as offsets
	register direct pointers are optimised by leaving the offset alone
	(except for PC register direct, since there is no LEAX D,PC)
-----------------------------------------------------------------------------*/

PUBLIC void indexadr(source, target)
struct symstruct *source;
struct symstruct *target;
{
#ifdef MC6809
    bool_t canABX;
#endif
    uoffset_T size;
    store_pt sourcereg;
    struct typestruct *targtype;
    store_pt targreg;

    if (!(target->type->constructor & (ARRAY | POINTER)))
    {
	bugerror("cannot index");
	return;
    }
    size = target->type->nexttype->typesize;
    if (source->storage == CONSTANT)
    {
	if (source->offset.offv != 0)
	{
	    if (target->indcount != 0)
		loadany(target);
	    target->offset.offi += source->offset.offv * size;
	}
	return;
    }
    if (target->storage & ALLDATREGS)
	push(target);
    if ((store_t) (sourcereg = target->storage) & ~reguse & allindregs)
	targreg = sourcereg;
    else
	targreg = getindexreg();
#ifdef I8088
    if ((store_t) sourcereg == GLOBAL && target->indcount == 0 &&
	!(source->type->scalar & CHAR) && source->storage != DREG)
	load(source, targreg);
    else
#endif
	load(source, DREG);

#ifdef I8088
    softop(MULOP, constsym((value_t) size), source);
#endif
#ifdef MC6809

/*-----------------------------------------------------------------------------
	do some calculations in advance to decide if index can be done with ABX
-----------------------------------------------------------------------------*/

    if ((store_t) targreg == XREG && source->type->scalar & CHAR &&
	size < CANABXCUTOFF)
	canABX = TRUE;
    else
    {
	canABX = FALSE;
	softop(MULOP, constsym((value_t) size), source);
    }

#endif

/*-----------------------------------------------------------------------------
	deal with constant target - constant becomes offset, result in DREG
-----------------------------------------------------------------------------*/

    if (target->storage == CONSTANT)
    {
	target->storage = DREG;
	return;
    }

/*-----------------------------------------------------------------------------
	load target if it is indirect or GLOBAL or canABX so D or B can be added
	otherwise, it is register direct (maybe S register, maybe with offset)
	and the offset can be left after adding DREG
-----------------------------------------------------------------------------*/

#ifdef I8088
    if (target->indcount != 0)
    {
	targtype = target->type;
	target->type = itype;
	add(source, target);
	target->type = targtype;
	return;
    }
    if ((store_t) sourcereg == GLOBAL)
    {
	target->storage = source->storage;
	target->level = OFFKLUDGELEVEL;
	return;
    }
#endif
#ifdef MC6809
    if (canABX || (store_t) sourcereg == GLOBAL)
    {
	load(target, targreg);
	sourcereg = targreg;
    }
    else if (target->indcount != 0)
    {
	targtype = target->type;
	target->type = itype;
	add(source, target);
	target->type = targtype;
	return;
    }
    if (canABX)
	while (size--)
	    outABX();
    else
#endif
    {
#ifdef I8088
	if ((store_t) targreg != (store_t) sourcereg)
	    regtransfer(sourcereg, targreg);
	outadd();
	outregname(targreg);
	outncregname(DREG);
#endif
#ifdef MC6809
	outlea();
	outregname(targreg);
	outtab();
	outregname(DREG);
	outncregname(sourcereg);
#endif
    }
    if ((store_t) sourcereg == LOCAL)
#ifdef FRAMEPOINTER
	target->offset.offi -= framep;
#else
	target->offset.offi -= sp;
#endif
    target->storage = targreg;
}

PUBLIC void indirec(source)
struct symstruct *source;
{
    if (!(source->type->constructor & (ARRAY | POINTER)))
	bugerror("illegal indirection");
    else if (source->indcount == (indn_t) - 1)
	limiterror("too many indirections (256)");
    else
    {
	if (source->storage & ALLDATREGS)
	    transfer(source, getindexreg());
	if (!((source->type = source->type->nexttype)->constructor &
	      (ARRAY | FUNCTION)))
	    ++source->indcount;
	if (source->storage == CONSTANT)
	    source->storage = GLOBAL;
    }
}

/*-----------------------------------------------------------------------------
	load(source leaf, target register)
	loads the specified register without changing any others (except CC)
	if the type is long or float, DREG is paired with the target register
	the result has no offset
-----------------------------------------------------------------------------*/

PUBLIC void load(source, targreg)
struct symstruct *source;
store_pt targreg;
{
    if (source->type->scalar & DLONG)
    {
	if (source->storage == CONSTANT)
	    loadreg(source, targreg);
	else if (source->indcount == 0)
	{
#if DYNAMIC_LONG_ORDER
	    if (!long_big_endian)
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN == 0
	    {
		if ((store_t) targreg == DREG)
		    source->storage = DREG;
	    }
#endif
	    if (source->storage != (store_t) targreg)
		transfer(source, targreg);
	    if (source->offset.offi != 0)
		bugerror("loading direct long with offset not implemented");
	}
	else
	    loadlongindirect(source, targreg);
    }
    else if (source->type->scalar & DOUBLE)
    {
	if (source->storage == targreg && source->indcount == 0)
	    return;
	if (source->storage == CONSTANT)
	{
#ifdef I80386
	    if (i386_32)
	    {
	       loadconst(((offset_T *) source->offset.offd)[0], DREG);
	       loadconst(((offset_T *) source->offset.offd)[1], targreg&~DREG);
	    }
	    else /* XXX - more for non-386 */
#endif
	    {
	       int regs, i, off=1;
	       loadconst(((unsigned short *) source->offset.offd)[0], DREG);
	       regs = (targreg&~DREG);
	       for(i=1; i; i<<=1)
	       {
		  if( regs&i )
	              loadconst(
		         ((unsigned short *) source->offset.offd)[off++],
		         i);
	       }
	    }
	}
	else
	{
	    push(source);
	    poplist(targreg | DREG);	/* actually it's the full reg list */
	}
	source->storage = targreg;	/* XXX - multi for non-386 */
	source->indcount = 0;
	source->flags = 0;
	if (source->level == OFFKLUDGELEVEL)
	    source->level = EXPRLEVEL;
	source->offset.offi = 0;
    }
    else if (source->type->scalar & FLOAT && source->storage == CONSTANT)
    {
	float val;

	val = *source->offset.offd;
#ifdef I80386
	if (i386_32)
	   loadconst(((offset_T *) &val)[0], targreg);	/* XXX 386 */
	else
#endif
	{
	   loadconst(((unsigned short *) &val)[0], DREG);
	   loadconst(((unsigned short *) &val)[1], targreg&~DREG);
	}
    }
#ifdef I80386
    else if (!i386_32 && source->type->scalar & FLOAT)
#else
    else if (source->type->scalar & FLOAT)
#endif
    {
	/* Treat a float just like a long ... */
	if (source->indcount == 0)
	{
	    if (source->storage != (store_t) targreg)
		transfer(source, targreg);
	    if (source->offset.offi != 0)
		bugerror("loading direct float with offset not implemented");
	}
	else
	    loadlongindirect(source, targreg);
    }
    else if (source->indcount == 0 && source->storage != CONSTANT)
	loadadr(source, targreg);
    else if (source->type->scalar ||
	     source->type->constructor & (ARRAY | POINTER))
	loadreg(source, targreg);
    else
	bugerror("attempting to load non-scalar non-pointer");
}

PRIVATE void loadadr(source, targreg)
struct symstruct *source;
store_pt targreg;
{
    if ((store_t) targreg & ALLDATREGS)
    {
	if (source->storage == GLOBAL)
	{
#ifdef MC6809
	    if (posindependent)
	    {
		pushreg(INDREG0);
		loadreg(source, INDREG0);
		transfer(source, DREG);
		recovlist(INDREG0);
	    }
	    else
#endif
		loadreg(source, targreg);
	}
	if (source->storage == LOCAL)
#ifdef FRAMEPOINTER
	    source->offset.offi -= framep;
#else
	    source->offset.offi -= sp;
#endif
	if (source->type->scalar & CHAR)
	    targreg = BREG;
#ifdef I8088
	if (source->storage == DREG)
	    addoffset(source);
#endif
	if (source->storage != (store_t) targreg)
	    transfer(source, targreg);
	addoffset(source);
    }
    else if (source->storage & ALLDATREGS)
    {
	addoffset(source);
	transfer(source, targreg);
    }
    else if (source->storage != (store_t) targreg ||
	     source->offset.offi != 0 || source->level == OFFKLUDGELEVEL)
	loadreg(source, targreg);
}

PUBLIC void loadany(source)
struct symstruct *source;
{
    if (source->indcount != 0 || source->offset.offi != 0 || /* kludge u cmp */
	source->level == OFFKLUDGELEVEL || !(source->storage & allregs))
    {
	if (source->type->scalar & RSCALAR)
	    load(source, doubleregs & ~DREG);
	else if ((source->storage == CONSTANT &&
		  !(source->type->scalar & DLONG))
		 || source->type->scalar & CHAR)
	    load(source, DREG);
	else if (source->storage & ~reguse & allregs)
	    load(source, source->storage);
	else if (((reguse & allindregs) == allindregs ||
		  ((!(source->type->constructor & (ARRAY | POINTER)) &&
		  source->indcount != 0) &&
		 !(source->type->scalar & DLONG))))
	    load(source, DREG);
	else
	    load(source, getindexreg());
    }
}

PRIVATE void loadlongindirect(source, targreg)
struct symstruct *source;
store_pt targreg;
{
    sc_t flags;
    offset_T offset;
    store_t reg;
    struct typestruct *type;

    if (source->level == OFFKLUDGELEVEL)
	addoffset(source);	/* else kludge is lost and offsets big */
    flags = source->flags;
    offset = source->offset.offi;
    reg = source->storage;
    type = source->type;
    source->type = itype;
    loadreg(source, DREG);
    source->flags = flags;
    source->storage = reg;
    source->indcount = 1;
    source->offset.offi = offset + accregsize;
    loadreg(source, targreg);
    source->type = type;
}

PUBLIC void loadreg(source, targreg)
struct symstruct *source;
store_pt targreg;
{
    offset_T longhigh;
    offset_T longlow;

    if (source->storage == CONSTANT)
    {
	if (source->type->scalar & CHAR && (store_t) targreg & ALLDATREGS)
	    targreg = BREG;
	longlow = (offset_T) source->offset.offv;
	if (source->type->scalar & DLONG)
	{
	    longlow &= (offset_T) intmaskto;
	    longhigh = (offset_T) (source->offset.offv >> INT16BITSTO)
		       & (offset_T) intmaskto;
	    if ((store_t) targreg != LONGREG2)	/* loading the whole long */
	    {
#if DYNAMIC_LONG_ORDER
		if (long_big_endian)
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN
		    loadconst(longhigh, DREG);
#endif
#if DYNAMIC_LONG_ORDER
		else
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN == 0
		{
		    loadconst(longlow, DREG);
		    longlow = longhigh;
		}
#endif
	    }
	}
	loadconst(longlow, targreg);
	source->storage = targreg;
	source->offset.offi = 0;
    }
    else
    {
#ifdef I8088
	if (source->indcount == 0 && source->storage != GLOBAL &&
	    (source->offset.offi != 0 || source->level == OFFKLUDGELEVEL))
	{
	    if ((store_t) targreg == source->storage)
	    {
		addoffset(source);
		return;
	    }
	    source->indcount = 1;	/* fake for outadr() to produce "()" */
	    outlea();
	}
	else
	{
#ifdef I8088
	    /* Added acess to CPU registers. Just declare _AX etc. as
	     * extern int, and you can use them to get/set
	     * the register values. (vak) */
	    if (source->storage == GLOBAL && !(source->flags & LABELLED) &&
		*source->name.namep != 0 && 
		strncmp(source->name.namep, "__", 2) == 0)
	    {
		if (strcmp (source->name.namep, "__AX") == 0)
		{
		    /* Load AX register - do nothing. */
done:		    
		    if (targreg != AXREG) 
		       bugerror("specified access of register clone variable not implemented");
		    source->storage = AXREG;	/* in register for further use */
		    source->flags = 0;
		    if (source->level == OFFKLUDGELEVEL)
			source->level = EXPRLEVEL;
		    source->offset.offi = 0;	/* indcount was adjusted by outadr */
		    return;
		}
		if (strcmp (source->name.namep, "__BX") == 0) { outstr ("mov\tax,bx\n"); goto done; }
		if (strcmp (source->name.namep, "__CX") == 0) { outstr ("mov\tax,cx\n"); goto done; }
		if (strcmp (source->name.namep, "__DX") == 0) { outstr ("mov\tax,dx\n"); goto done; }
		if (strcmp (source->name.namep, "__SP") == 0) { outstr ("mov\tax,sp\n"); goto done; }
		if (strcmp (source->name.namep, "__BP") == 0) { outstr ("mov\tax,bp\n"); goto done; }
		if (strcmp (source->name.namep, "__SI") == 0) { outstr ("mov\tax,si\n"); goto done; }
		if (strcmp (source->name.namep, "__DI") == 0) { outstr ("mov\tax,di\n"); goto done; }
		if (strcmp (source->name.namep, "__CS") == 0) { outstr ("mov\tax,cs\n"); goto done; }
		if (strcmp (source->name.namep, "__DS") == 0) { outstr ("mov\tax,ds\n"); goto done; }
		if (strcmp (source->name.namep, "__ES") == 0) { outstr ("mov\tax,es\n"); goto done; }
		if (strcmp (source->name.namep, "__SS") == 0) { outstr ("mov\tax,ss\n"); goto done; }
		if (strcmp (source->name.namep, "__FLAGS") == 0) { outstr ("pushf\npop\tax\n"); goto done; }
	    }
#endif
	    outload();
	    if (source->storage == GLOBAL && source->indcount != 0 &&
		(store_t) targreg & (AXREG | ALREG))
		unbumplc();
	}
#endif
#ifdef MC6809
	if (source->indcount == 0 &&
	    (source->storage != GLOBAL || posindependent))
	    outlea();
	else
	{
	    outload();
	    if ((store_t) targreg == YREG)
		bumplc();
	}
#endif
	movereg(source, targreg);
    }
}

PUBLIC void makelessindirect(source)
struct symstruct *source;
{
    store_pt lreg;

    if (!((store_t) (lreg = source->storage) & ~reguse & allindregs))
	lreg = getindexreg();
    while (source->indcount > MAXINDIRECT)
	loadreg(source, lreg);
#if MAXINDIRECT > 1
    if (source->indcount == MAXINDIRECT &&
	(source->type->typesize > maxregsize ||
	 source->type->constructor & FUNCTION))
    {
	source->indcount = 1;
	loadreg(source, lreg);
	source->indcount = 1;
    }
#endif
}

PUBLIC void movereg(source, targreg)
struct symstruct *source;
store_pt targreg;
{
    if ((store_t) targreg & ALLDATREGS && source->type->scalar & CHAR)
	targreg = BREG;
#ifdef I8086
    if (source->type->scalar & CHAR &&
	  targreg != BREG && targreg != DATREG1B ) {
       bugerror("moving char sym into int register");
    }
#endif
#ifdef I80386
    if (i386_32 && source->type->scalar & SHORT &&
	source->indcount <= 1)
    {
	outshortregname(targreg);
	bumplc();
    }
    else
#endif
	outregname(targreg);
    if (source->storage == CONSTANT)
	adjlc((offset_T) source->offset.offv, targreg);
#ifdef I8088
    outcomma();
#endif
    outadr(source);
    source->storage = targreg;	/* in register for further use */
    source->flags = 0;
    if (source->level == OFFKLUDGELEVEL)
	source->level = EXPRLEVEL;
    source->offset.offi = 0;	/* indcount was adjusted by outadr */
}

PUBLIC void onstack(target)
register struct symstruct *target;
{
    target->storage = LOCAL;
    target->flags = TEMP;
    if (target->level == OFFKLUDGELEVEL)
	target->level = EXPRLEVEL;
    target->indcount = 1;
    target->offset.offi = sp;
}

PUBLIC void outadr(adr)
struct symstruct *adr;
{
    outnnadr(adr);
    outnl();
}

PUBLIC void outcregname(reg)
store_pt reg;
{
    outcomma();
    outregname(reg);
}

PRIVATE void outnamoffset(adr)
struct symstruct *adr;
{
    if (adr->flags & LABELLED)
	outlabel(adr->name.label);
    else
	outccname(adr->name.namep);
    if (adr->offset.offi != 0)
    {
	if (adr->offset.offi > 0)
	    outplus();
	outshex(adr->offset.offi);
    }
    bumplc2();
#ifdef I80386
    if (i386_32)
	bumplc2();
#endif
}

/* print comma, then register name, then newline */

PUBLIC void outncregname(reg)
store_pt reg;
{
    outcomma();
    outnregname(reg);
}

PRIVATE void outnnadr(adr)
struct symstruct *adr;
{
    bool_t indflag;

    indflag = FALSE;
#ifdef I8088
    if (adr->indcount >= MAXINDIRECT)
	indflag = TRUE;
#endif
#ifdef MC6809
    outtab();
    if (adr->indcount >= MAXINDIRECT && (adr->indcount & 1) == 0)
    {
	indflag = TRUE;		/* indirection means double indirect */
	outindleft();
    }
#endif
    switch (adr->storage)
    {
    case CONSTANT:
	outimmadr((offset_T) adr->offset.offv);
	break;
#ifdef I8088
    case DREG:
	if (indflag || adr->offset.offi != 0 || adr->level == OFFKLUDGELEVEL)
	    badaddress();
	else
	    outregname(DREG);
	break;
#endif
#ifdef I8088
    case DATREG1:
    case DATREG2:
#ifdef I80386
	if (indflag && !i386_32)
#else
	if (indflag)
#endif
	{
	    outnl();
	    badaddress();
	    break;
	}
#endif
    case INDREG0:
    case INDREG1:
    case INDREG2:
	if (adr->level == OFFKLUDGELEVEL)
	{
#ifdef I8088
	    if (!indflag)
#endif
                outimmed();
	    outnamoffset(adr);
        }
#ifndef MC6809
	else if (adr->offset.offi != 0)
#endif
	    outoffset(adr->offset.offi);
#ifdef I8088
	if (indflag)
	    outindleft();
	outregname(adr->storage);
# ifdef XENIX_AS
	if (indflag)
	    outindright();
# endif
#endif
#ifdef MC6809
	if (indflag && adr->offset.offi != 0 && is5bitoffset(adr->offset.offi))
	    bumplc();
	outcregname(adr->storage);
#endif
	break;
    case LOCAL:
#ifdef I8088
# ifdef FRAMEPOINTER
	if (framep == 0)
	    bugerror("no frame pointer");
	if (indflag)
	{
	    if (adr->offset.offi == framep)
		bumplc();
	    else
	    {
	        int off;
		if (switchnow != NULL && adr->flags == TEMP)
		    outswoffset(off = adr->offset.offi);
		else
		    outoffset(off = adr->offset.offi - framep);
#ifndef NO_DEL_PUSH
		if (optimise && !callersaves && off < 0) 
		{
		    outstr("+");
		    outstr(funcname);
		    outstr(".off");
		}
#endif
	    }
	    outindleft();
	}
	else if (adr->offset.offi != framep)
	    badaddress();
	outregname(LOCAL);
# else
	if (indflag)
	{
	    bumplc();
	    if (adr->offset.offi != sp)
		outoffset(adr->offset.offi - sp);
	    outindleft();
	}
	else if (adr->offset.offi != sp)
	    badaddress();
	outregname(LOCAL);
# endif /* FRAMEPOINTER */
# ifdef XENIX_AS
	if (indflag)
	    outindright();
# endif
#endif /* I8088 */
#ifdef MC6809
	if (adr->flags == TEMP && adr->offset.offi == sp &&
	    adr->indcount == 1)
	{
	    outcregname(LOCAL);
	    outplus();
	    ++sp;
	    if (adr->type->typesize != 1)
	    {
		outplus();
		++sp;
	    }
	    break;
	}
	outoffset(adr->offset.offi - sp);
	if (indflag && adr->offset.offi != sp &&
	    is5bitoffset(adr->offset.offi - sp))
	    bumplc();
	outcregname(LOCAL);
#endif /* MC6809 */
	break;
    case GLOBAL:
#ifdef I8088
	bumplc();
#ifdef I80386
	if (i386_32)
	    bumplc2();
#endif
	if (!indflag)
	    outimmed();
	else
	{
# ifndef XENIX_AS
	    outindleft();
# endif
	    bumplc();
	}
#endif
#ifdef MC6809
	if (!posindependent)
	{
	    if (adr->indcount == 0)
	    {
		outimmed();
		bumplc();
	    }
	    else if (indflag)
	    {
		outextended();
		bumplc2();
	    }
	    else if (adr->flags & DIRECTPAGE)
		outdirectpage();
	    else
	    {
		outextended();
		bumplc();
	    }
	}
#endif
	if (adr->flags & LABELLED)
	    outlabel(adr->name.label);
	else if (*adr->name.namep == 0)	/* constant address */
	{
	    outhex((uoffset_T) adr->offset.offi);
	    break;
	}
	else
	    outccname(adr->name.namep);
	if (adr->offset.offi != 0)
	{
	    if (adr->offset.offi > 0)
		outplus();
	    outshex(adr->offset.offi);
	}
#ifdef MC6809
	if (posindependent)
	{
	    outcregname(GLOBAL);
	    bumplc2();
	}
#endif
	break;
    default:
	outnl();
	badaddress();
	break;
    }
#ifdef I8088
    if (indflag)
    {
	--adr->indcount;
# ifndef XENIX_AS
	outindright();
# endif
    }
#endif
#ifdef MC6809
    if (indflag)
    {
	outindright();
	adr->indcount -= MAXINDIRECT;
    }
    else if (adr->indcount != 0)
	--adr->indcount;
#endif
}

/* print register name, then newline */

PUBLIC void outnregname(reg)
store_pt reg;
{
    outregname(reg);
    outnl();
}

/* print register name */

PUBLIC void outregname(reg)
store_pt reg;
{
    switch ((store_t) reg)
    {
    case BREG:
	outstr(acclostr);
	break;
    case DREG:
	outstr(accumstr);
	break;
#ifdef MC6809
    case GLOBAL:
	outstr("PC");
	break;
#endif
    case INDREG0:
	outstr(ireg0str);
	regfuse |= INDREG0;
	break;
    case INDREG1:
	outstr(ireg1str);
	regfuse |= INDREG1;
	break;
    case INDREG2:
	outstr(ireg2str);
	regfuse |= INDREG2;
	break;
    case LOCAL:
	outstr(localregstr);
	regfuse |= LOCAL;
	break;
#ifdef STACKREG
    case STACKREG:
	outstr(stackregstr);
	break;
#endif
#ifdef DATREG1
    case DATREG1:
	outstr(dreg1str);
	break;
#endif
#ifdef DATREG1B
    case DATREG1B:
	outstr(dreg1bstr);
	break;
#endif
#ifdef DATREG2
    case DATREG2:
	outstr(dreg2str);
	break;
#endif
    default:
	{ int i;
	   if (reg)
	      for(i=1; i; i<<=1)
	      {
	         if( reg&i )
		 {
		    outregname(i);
		    outstr(" ");
		 }
	      }
	   else
              outstr(badregstr);
	}
	break;
    }
}

#if defined(I8088) && defined(I80386)
/* print register name for short type */

PUBLIC void outshortregname(reg)
store_pt reg;
{
    switch ((store_t) reg)
    {
    case DREG:
	outstr(accumstr + 1);
	break;
    case INDREG0:
	outstr(ireg0str + 1);
	break;
    case INDREG1:
	outstr(ireg1str + 1);
	break;
    case INDREG2:
	outstr(ireg2str + 1);
	break;
    case DATREG1:
	outstr(dreg1str + 1);
	break;
    case DATREG2:
	outstr(dreg2str + 1);
	break;
    default:
	outstr(badregstr);
	break;
    }
}

#endif

/*-----------------------------------------------------------------------------
	pointat(target leaf)
	point OPREG at target
	target must be singly indirect or float or double
-----------------------------------------------------------------------------*/

PUBLIC void pointat(target)
struct symstruct *target;
{
    if (target->type->scalar & RSCALAR)
	(void) f_indirect(target);
    address(target);
    load(target, OPREG);
    target->type = target->type->nexttype;
}

PUBLIC void poplist(reglist)
store_pt reglist;
{
    if (reglist)
	sp += pushpull(reglist, FALSE);
}

PUBLIC void push(source)
struct symstruct *source;
{
    store_t reg;
#ifdef I8088
    uoffset_T size;
#endif
    scalar_t sscalar;

    if (source->type->constructor & STRUCTU)
    {
	if (source->flags != TEMP)	/* TEMP must be from last function */
	    blockpush(source);
    }
    else if ((sscalar = source->type->scalar) & RSCALAR)
    {
	if (!f_indirect(source))
	{
	    saveopreg();
	    fpush(source);
	    restoreopreg();
	}
    }
#ifdef I8088
    else if ((source->indcount == 1 &&
	     (sscalar & (SHORT | INT | LONG | FLOAT) ||
	      source->type->constructor & POINTER))
#ifdef I80386
	      || (source->storage == CONSTANT && i386_32)
#endif
	      )
    {
	size = source->type->typesize;
	if (size == 1)
	    size = 2;
	if (sscalar & DLONG)
	{
	    source->offset.offi += itypesize;
	    outpshs();
	    bumplc();
	    outtab();
	    outadr(source);
	    source->indcount = 1;
	    source->offset.offi -= itypesize;
	}
	outpshs();
	bumplc();
	outtab();
#ifdef I80386
	if (i386_32)
	{
	    if (source->storage == CONSTANT)
	    {
		unbumplc();
		adjlc((offset_T) source->offset.offv, INDREG0);
	    }
	    if (size == 2)
	    {
		outword();
		bumplc();
	    }
	    else
		outdword();
	}
#endif
	outadr(source);
	sp -= size;
    }
#endif
    else
    {
	reg = source->storage;
	loadany(source);
	if (sscalar & DLONG)
	    pushlist(DREG | source->storage);
	else if (sscalar & CHAR)
	    pushchar();
	else
	    pushreg(source->storage);
	if (source->flags != REGVAR)
	    reguse &= ~(reg | source->storage);
    }
    onstack(source);
}

PUBLIC void pushlist(reglist)
store_pt reglist;
{
    if ((store_t) reglist)
	sp -= pushpull(reglist, TRUE);
}

PRIVATE fastin_pt pushpull(reglist, pushflag)
store_pt reglist;
bool_pt pushflag;
{
    store_pt lastregbit;
    void (*ppfunc) P((void));
    char *regptr;

#ifdef MC6809
    int separator;		/* promoted char for output */

#endif
    fastin_t bytespushed;
    store_pt regbit;

    if ((bool_t) pushflag)
    {
	ppfunc = outpshs;
#ifdef I8088
	regbit = 1 << 10;
#else
	regbit = 1 << 7;
#endif
	regptr = regpushlist;
	lastregbit = 1;
    }
    else
    {
	ppfunc = outpuls;
	regbit = 1;
	regptr = regpulllist;
#ifdef I8088
	lastregbit = 1 << 10;
#else
	lastregbit = 1 << 7;
#endif
    }
#ifdef MC6809 /* temp use pull strings to keep old order */
    regbit = 1;
    regptr = regpulllist;
    lastregbit = 1 << 7;
#endif
#ifdef MC6809
    separator = OPSEPARATOR;
    (*ppfunc) ();
#endif
    bytespushed = 0;
    while (TRUE)
    {
	if (regbit & reglist)
	{
#ifdef I8088
	    (*ppfunc)();
	    if (*regptr != FLAGSREGCHAR)
		outtab();
#endif
#ifdef MC6809
	    outbyte(separator);
#endif
	    do
		outbyte(*regptr++);
	    while (*regptr >= MINREGCHAR);
	    bytespushed += *regptr++ - '0';
#ifdef I8088
	    outnl();
#endif
#ifdef MC6809
	    separator = OPERANDSEPARATOR;
#endif
	}
	else
	    do
		;
	    while (*regptr++ >= MINREGCHAR);
	if (regbit == lastregbit)
	    break;
#ifdef MC6809 /* temp use pull strings to keep old order */
	regbit <<= 1;
#else /* this should normally be unconditional */
	if ((bool_t) pushflag)
	    regbit >>= 1;
	else
	    regbit <<= 1;
#endif
    }
#ifdef MC6809
    outnl();
#endif
    return bytespushed;
}

PUBLIC void pushreg(reg)
store_pt reg;
{
    outpshs();
    outtab();
    outnregname(reg);
    sp -= pshregsize;
}

PUBLIC void storereg(sourcereg, target)
store_pt sourcereg;
struct symstruct *target;
{
    store_pt targreg;

    if (target->indcount == 0)
    {
	if (target->offset.offi != 0 || target->level == OFFKLUDGELEVEL ||
	    !(target->storage & allregs) || target->storage & CHARREGS)
	    bugerror("bad register store");
	else if ((store_t) (targreg = target->storage) != (store_t) sourcereg)
	{
	    target->storage = sourcereg;
	    loadadr(target, targreg);	/* do LEA or TFR */
	}
    }
    else
    {
#ifdef I8088
	/* Added acess to CPU registers. Just declare _AX etc. as
	 * extern int, and you can use them to get/set
	 * the register values. (vak) */
	if (target->storage == GLOBAL && !(target->flags & LABELLED) &&
	    *target->name.namep != 0 && 
	     strncmp(target->name.namep, "__", 2) == 0)
	{
	    if (strcmp (target->name.namep, "__AX") == 0) { return; }
	    if (strcmp (target->name.namep, "__BX") == 0) { outstr ("mov\tbx,"); outregname(sourcereg); outnl(); return; }
	    if (strcmp (target->name.namep, "__CX") == 0) { outstr ("mov\tcx,"); outregname(sourcereg); outnl(); return; }
	    if (strcmp (target->name.namep, "__DX") == 0) { outstr ("mov\tdx,"); outregname(sourcereg); outnl(); return; }
	    if (strcmp (target->name.namep, "__SP") == 0) { outstr ("mov\tsp,"); outregname(sourcereg); outnl(); return; }
	    if (strcmp (target->name.namep, "__BP") == 0) { outstr ("mov\tbp,"); outregname(sourcereg); outnl(); return; }
	    if (strcmp (target->name.namep, "__SI") == 0) { outstr ("mov\tsi,"); outregname(sourcereg); outnl(); return; }
	    if (strcmp (target->name.namep, "__DI") == 0) { outstr ("mov\tdi,"); outregname(sourcereg); outnl(); return; }
	    if (strcmp (target->name.namep, "__CS") == 0) { outstr ("mov\tcs,"); outregname(sourcereg); outnl(); return; }
	    if (strcmp (target->name.namep, "__DS") == 0) { outstr ("mov\tds,"); outregname(sourcereg); outnl(); return; }
	    if (strcmp (target->name.namep, "__ES") == 0) { outstr ("mov\tes,"); outregname(sourcereg); outnl(); return; }
	    if (strcmp (target->name.namep, "__SS") == 0) { outstr ("mov\tss,"); outregname(sourcereg); outnl(); return; }
	    if (strcmp (target->name.namep, "__FLAGS") == 0) { outstr ("push\tax"); outregname(sourcereg); outstr ("\npopf\n"); return; }
	}
#endif
	outstore();
#ifdef I8088
	if (target->storage == GLOBAL && (store_t) sourcereg & (AXREG | ALREG))
	    unbumplc();
	outnnadr(target);
	outcomma();
#ifdef I80386
	if (i386_32 && target->type->scalar & SHORT)
	{
	    outshortregname(sourcereg);
	    bumplc();
	    outnl();
	}
	else
#endif
	    outnregname(sourcereg);
#endif
#ifdef MC6809
	if ((store_t) sourcereg == YREG)
	    bumplc();
	outregname(sourcereg);
	outadr(target);
#endif
    }
}

/*-----------------------------------------------------------------------------
	struc(element leaf, structure leaf)
	evaluates the expression
		structure.element
-----------------------------------------------------------------------------*/

PUBLIC void struc(source, target)
struct symstruct *source;
struct symstruct *target;
{
    address(target);
    if (source->offset.offi != 0 || source->level == OFFKLUDGELEVEL)
    {
	if (target->indcount != 0 || target->level == OFFKLUDGELEVEL)
	    load(target, getindexreg());
	target->offset.offi += source->offset.offi;
    }
    if (source->indcount == 0)
	target->type = source->type;
    else
    {
	target->type = pointype(source->type);	/* lost by = */
	indirec(target);
    }
}

PUBLIC void transfer(source, targreg)
struct symstruct *source;
store_pt targreg;
{
    regtransfer(source->storage, targreg);
    source->storage = targreg;
}
