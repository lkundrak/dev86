/* longop.c - software operations on longs for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"
#include "byteord.h"
#include "gencode.h"
#include "reg.h"
#include "scan.h"
#include "type.h"

/*-----------------------------------------------------------------------------
	longop(operation code, source leaf, target leaf)
	handles all binary operations on longs
	source and target must already have been converted to long,
	(except source is int for shifts) and not more than singly indirect
	hence they must be direct (in an index reg paired with DREG),
	or singly indirect (local, global, or from an index reg)
-----------------------------------------------------------------------------*/

PUBLIC void longop(op, source, target)
op_pt op;
struct symstruct *source;
struct symstruct *target;
{
    store_pt reglist;
    store_t regmark;
    bool_t shiftflag;
    scalar_t scalar;
    offset_T spmark;

    pushlist(reglist = (regmark = reguse) & (OPREG | OPWORKREG));
    reguse &= ~reglist;
    spmark = sp;
    shiftflag = FALSE;
    scalar = target->type->scalar;
    if ((op_t) op == SLOP || (op_t) op == SROP)
	shiftflag = TRUE;
    else
	scalar |= source->type->scalar;
    if ((source->indcount == 0 && !shiftflag) ||
	source->type->scalar & CHAR ||
	source->storage & (BREG | DREG | OPREG | OPWORKREG))
    {
	pres2(source, target);
	push(source);
    }
    if (!shiftflag)
	address(source);
    load(target, OPREG);
    if (source->storage == CONSTANT && shiftflag)
    {
	if (scalar & UNSIGNED)
	    target->type = ultype;
	if ((op_t) op == SLOP)
	    source->offset.offv = lslconst(source->offset.offv,
					   target->storage);
	else
	    source->offset.offv = lsrconst(source->offset.offv,
					   target->storage, scalar & UNSIGNED);
	if (source->offset.offv == 0)
	    goto shiftdone;
    }
#ifdef I8088
    /* This is ugly! But it works. I should be able to stop it being used
     * by removing the char demotion. */
    if (source->type->scalar & CHAR && 
	  !(source->storage & (BREG|DREG|DATREG1B))) {
	load(source, DATREG1B);
	outop2str("xor\tch,ch"); outnl();
	source->storage = DATREG1;
    }
#endif
    load(source, OPWORKREG);
    switch ((op_t) op)
    {
    case ADDOP:
	call("ladd");
	break;
    case ANDOP:
	call("land");
	break;
    case DIVOP:
	call("ldiv");
	break;
    case EOROP:
	call("leor");
	break;
    case EQOP:
	call("lcmp");
	break;
    case MODOP:
	call("lmod");
	break;
    case MULOP:
	call("lmul");
	break;
    case OROP:
	call("lor");
	break;
    case SLOP:
	call("lsl");
	break;
    case SROP:
	call("lsr");
	break;
    case SUBOP:
	call("lsub");
	break;
    }
    if (scalar & UNSIGNED)
    {
	outbyte('u');
	target->type = ultype;
    }
    outlongendian();

shiftdone:
    if ((reguse = regmark) & OPREG && op != EQOP)
	load(target, getindexreg());
    if (reglist)
    {
#ifdef I8088
	if (op == EQOP)
	    changesp(spmark, FALSE);
	else
#endif
	    modstk(spmark);
	poplist(reglist);
    }
}

/*-----------------------------------------------------------------------------
	long1op(operation code, target leaf)
	handles all unary operations on longs except inc/dec
	target must be not more than singly indirect
	hence it must be direct (in an index reg paired with DREG),
	or singly indirect (local, global, or from an index reg)
-----------------------------------------------------------------------------*/

PUBLIC void long1op(op, target)
op_pt op;
struct symstruct *target;
{
    pushlist(reguse & OPREG);
    load(target, OPREG);
    if (op == NOTOP)
	call("lcom");
    else if (op == NEGOP)
	call("lneg");
    else
	call("ltst");
    outlongendian();
    if (reguse & OPREG)
    {
	if (op != EQOP)
	    load(target, getindexreg());
	poplist(reguse & OPREG);
    }
}

PUBLIC void outlongendian()
{
#ifdef MC6809
    outbyte('_');
#endif
#if DYNAMIC_LONG_ORDER
    if (long_big_endian)
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN
	outnbyte('b');
#endif
#if DYNAMIC_LONG_ORDER
    else
#endif
#if DYNAMIC_LONG_ORDER || LONG_BIG_ENDIAN == 0
	outnbyte('l');
#endif
}
