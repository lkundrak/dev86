/* function.c - function call protocol for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"
#include "align.h"
#include "byteord.h"
#include "gencode.h"
#include "parse.h"
#include "reg.h"
#include "sc.h"
#include "table.h"
#include "type.h"
#include "scan.h"

#ifdef I8088
# define ADJUSTLONGRETURN
# define CANHANDLENOFRAME
# undef CANHANDLENOFRAME
# define STUPIDFRAME
#endif

FORWARD void out_callstring P((void));

/* call a named (assembly interface) procedure, don't print newline after */

PUBLIC void call(name)
char *name;
{
    out_callstring();
    outstr(name);
}

PUBLIC void function(source)
struct symstruct *source;
{
    if (source->indcount == 0 && source->storage == GLOBAL &&
	!(source->flags & LABELLED) && *source->name.namep != 0)
    {
	out_callstring();
	outnccname(source->name.namep);
    }
    else
    {
#ifdef XENIX_AS
	if (source->indcount == 0)	/* fix call fixed address */
	    out_callstring();
	else
#endif
	    outcalladr();
#ifdef MC6809
	if (source->indcount == 1)
	    ++source->indcount;	/* fake for outadr */
#endif
	outadr(source);
    }
    source->type = source->type->nexttype;
#ifdef LONGRETSPECIAL /* LONGRETURNREGS!=RETURNREG && RETURNREG==LONGREG2 */
    if (source->type->scalar & DLONG)
    {
# ifdef ADJUSTLONGRETURN
#  if DYNAMIC_LONG_ORDER
	if (long_big_endian)
#  endif
#  if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN
	{
	    regexchange(LONGREG2, LONGRETURNREGS & ~LONGREG2);
	    regexchange(LONGREG2, DXREG);
	}
#   endif
#  if DYNAMIC_LONG_ORDER
	else
#  endif
#  if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN == 0
	    regtransfer(DXREG, LONGRETURNREGS & ~LONGREG2);
#  endif
# endif
	source->storage = LONGRETURNREGS & ~LONGREG2;
    }
    else
#endif
    if (source->type->scalar & CHAR)
    {
#if RETURNREG != DREG
	transfer(source, DREG);
#endif
	source->storage = BREG;
    }
#ifdef I80386
    else if (i386_32)
    {
        if (source->type->scalar & DOUBLE)
	    source->storage = doublreturnregs & ~DREG;
        else
	    source->storage = RETURNREG;
    }
    else
#endif
    {
        if (source->type->scalar & DOUBLE)
	    source->storage = doublreturnregs;
#ifdef I8088
        else if (source->type->scalar & FLOAT)
	    source->storage = RETURNREG|DATREG2;
#endif
        else
	    source->storage = RETURNREG;
    }
    source->offset.offi = source->indcount = 0;
    if (source->level == OFFKLUDGELEVEL)
	source->level = EXPRLEVEL;
    if (source->type->constructor & STRUCTU)
    {
	transfer(source, getindexreg());	/* so it can be indirected
					 * and/or preserved in blockmove() */
	source->indcount = 1;
	source->flags = TEMP;	/* kludge so blockpush can be avoided */
    }
}

PUBLIC void ldregargs()
{
    register struct symstruct *symptr;
    store_pt targreg;
    struct symstruct temptarg;

    for (symptr = &locsyms[0]; symptr < locptr && symptr->level == ARGLEVEL;
	 symptr = (struct symstruct *)
		  align(&symptr->name.namea[strlen(symptr->name.namea) + 1]))
    {
	if ((store_t) (targreg = symptr->storage) & allregs)
	{

	    /* load() is designed to work on expression symbols, so don't
	     * trust it on reg variables although it almost works.
	     */
	    temptarg = *symptr;
	    if (arg1inreg && symptr == &locsyms[0])
	    {
		temptarg.storage = ARGREG;
		temptarg.offset.offi = 0;
	    }
	    else
	    {
		temptarg.storage = LOCAL;
		temptarg.indcount = 1;
	    }
	    load(&temptarg, targreg);
	    symptr->offset.offi = 0;
	}
    }
    regarg = FALSE;
}

PUBLIC void loadretexpression()
{
    if (returntype->constructor & STRUCTU)
    {
	struct nodestruct *etmark;
	struct nodestruct *exp;
	struct symstruct *exprmark;
	struct symstruct *structarg;

	etmark = etptr;
	exprmark = exprptr;
	exp = expression();
	makeleaf(exp);
	structarg = constsym((value_t) 0);
	structarg->type = pointype(returntype);
	onstack(structarg);
	indirec(structarg);
	structarg->flags = 0;	/* assign() doesn't like TEMP even for indir */
	structarg->offset.offi = returnadrsize;
	assign(exp->left.symptr, structarg);
	etptr = etmark;
	exprptr = exprmark;
    }
#ifdef LONGRETSPECIAL /* LONGRETURNREGS!=RETURNREG && RETURNREG==LONGREG2 */
    else if (returntype->scalar & DLONG)
    {
	loadexpression(LONGRETURNREGS & ~LONGREG2, returntype);
# ifdef ADJUSTLONGRETURN
#  if DYNAMIC_LONG_ORDER
	if (long_big_endian)
#  endif
#  if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN
	{
	    regexchange(LONGREG2, DXREG);
	    regexchange(LONGREG2, LONGRETURNREGS & ~LONGREG2);
	}
#   endif
#  if DYNAMIC_LONG_ORDER
	else
#  endif
#  if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN == 0
	    regtransfer(LONGRETURNREGS & ~LONGREG2, DXREG);
#  endif
# endif
    }
    else
#endif
    {
#ifdef I80386
        if (i386_32)
        {
            if (returntype->scalar & DOUBLE)
	        loadexpression(doublreturnregs & ~DREG, returntype);
            else
	        loadexpression(RETURNREG, returntype);
        }
        else
#endif
	{
            if (returntype->scalar & DOUBLE)
	        loadexpression(doublreturnregs, returntype);
#ifdef I8088
            else if (returntype->scalar & FLOAT)
	        loadexpression(/* REURNREG|*/ DATREG2, returntype);
#endif
            else
	        loadexpression(RETURNREG, returntype);
        }
    }
}

PUBLIC void listo(target, lastargsp)
struct symstruct *target;
offset_T lastargsp;
{
    extend(target);
    push(target);
    if (lastargsp != 0 && sp != lastargsp - target->type->typesize)
    {
	loadany(target);
	modstk(lastargsp);
	push(target);
	if (sp != lastargsp - target->type->typesize)
	{
	    bugerror("botched push of arg");
#ifdef DBNODE
	    outstr("arg type is ");
	    dbtype(target->type);
	    outnl();
#endif
	}
    }
}

PUBLIC void listroot(target)
struct symstruct *target;
{
    extend(target);
    /* necessary regs are free since they were saved for function */
    if (target->type->scalar & DLONG)
	load(target, LONGARGREGS & ~LONGREG2);
    else
	load(target, ARGREG);
}

PRIVATE void out_callstring()
{
    outop3str(callstring);
#ifdef I80386
    if (i386_32)
	bumplc2();
#endif
}

#ifdef FRAMEPOINTER

PUBLIC void popframe()
{
#ifdef STUPIDFRAME
#ifndef NO_DEL_PUSH
    if (optimise && !callersaves) {
        outstr("if ");
        outstr(funcname);
        outnstr(".off=0");
    }
    poplist(callee1mask);
    if (optimise && !callersaves)
        outnstr("endif");
#else
    poplist(callee1mask);
#endif
    poplist(FRAMEREG);
#else
    poplist(frame1list);
#endif
}

#endif

/* reserve storage for locals if necessary */
/* also push 1st function arg and load register args if necessary */

PUBLIC void reslocals()
{
#ifdef FRAMEPOINTER
# ifndef STUPIDFRAME
    bool_t loadframe = FALSE;

# endif
#endif

    if (switchnow != NULL)
    {
#ifdef FRAMEPOINTER
	if (framep == 0 && softsp != sp)
	    bugerror("local variables in switch statement messed up, sorry");
#else
	if (sp != softsp)
	    bugerror("local variables in switch statement don't work, sorry");
#endif
	if (lowsp > softsp)
	    lowsp = softsp;
	sp = softsp;
	return;
    }
#ifdef FRAMEPOINTER
    if (framep == 0)
    {
# ifdef STUPIDFRAME
	pushreg(FRAMEREG);
	regtransfer(STACKREG, FRAMEREG);
	framep = sp;
#ifndef NO_DEL_PUSH
	if (optimise && !callersaves) {
            outstr("if ");
            outstr(funcname);
            outnstr(".off=0");
	}
        pushlist(callee1mask);
	if (optimise && !callersaves) 
            outnstr("endif");
#else
	pushlist(callee1mask);
#endif
# else /* not STUPIDFRAME */
#  ifdef CANHANDLENOFRAME
	if (stackarg || softsp != -frameregsize)	/* args or locals */
#  endif
	{
	    pushlist(frame1list);
	    loadframe = TRUE;
	}
# endif /* not STUPIDFRAME */
    }
#else
    if (sp == 0)
	pushlist(callee1mask);
#endif /* FRAMEPOINTER */
    if (arg1size)
    {
	switch ((fastin_t) arg1size)
	{
	case 8:
	    pushlist(doubleargregs);
	    break;
	case 4:
# ifdef I80386
	    if (!i386_32)
# endif
	    {
		pushlist(LONGARGREGS);
		break;
	    }
	case 2:
# ifdef I8088
	    pushlist(ARGREG);
# endif
# ifdef MC6809
	    switch (sp - softsp)
	    {
	    case 3:
		pushlist(LOC1REGS | ARGREG);
		break;
	    case 4:
		pushlist(LOC2REGS | ARGREG);
		break;
	    case 5:
		pushlist(LOC3REGS | ARGREG);
		break;
	    case 6:
		pushlist(LOC4REGS | ARGREG);
		break;
	    default:
		pushlist(ARGREG);
		break;
	    }
# endif /* MC6809 */
	}
	arg1size = 0;		/* show 1st arg allocated */
    }
#ifdef FRAMEPOINTER
# ifndef STUPIDFRAME /* else this moved above for compat with Xenix cc frame */
    if (loadframe || softsp != -frameregsize)
	modstk(softsp);
    /* else avoid modstk() because softsp holds space for frame pointer only) */
    /* but pointer has not been pushed (must keep softsp for later levels) */
    if (loadframe)
    {
	regtransfer(STACKREG, FRAMEREG);
	framep = sp;
    }
# else /* STUPIDFRAME */
    modstk(softsp);
# endif /* STUPIDFRAME */
#else /* no FRAMEPOINTER */
    modstk(softsp);
#endif /* FRAMEPOINTER */
    if (regarg)
	ldregargs();
}

/* clean up stack and return from a function */

PUBLIC void ret()
{
#ifdef FRAMEPOINTER
    offset_T newsp;

    if (framep != 0)
    {
	newsp = -(offset_T) func1saveregsize;
	if (switchnow != NULL || newsp - sp >= 0x80)
	    changesp(newsp, TRUE);
	else
	    modstk(newsp);
	popframe();
    }
    outreturn();
#else /* no FRAMEPOINTER */
# ifdef MC6809
    store_pt reglist;

    switch (sp)
    {
    case -1:
	reglist = JUNK1REGS | PCREG;
	break;
    case -2:
	reglist = JUNK2REGS | PCREG;
	break;
    case -3:
	reglist = JUNK3REGS | PCREG;
	break;
    case -4:
	reglist = JUNK4REGS | PCREG;
	break;
    default:
	modstk(0);
	outreturn();
	return;
    }
    poplist(reglist);
#else
    if (sp != 0)
    {
	modstk(-(offset_T) func1saveregsize);
	poplist(callee1mask);
    }
    outreturn();
# endif /* no MC6809 */
#endif /* no FRAMEPOINTER */
}
