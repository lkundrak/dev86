/* declare.c - declaration routines for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"
#include "align.h"
#include "byteord.h"
#include "gencode.h"
#include "input.h"		/* just for orig_cppmode */
#include "reg.h"
#include "sc.h"
#include "scan.h"
#include "sizes.h"
#include "table.h"
#include "type.h"

#undef EXTERN
#define EXTERN
#include "parse.h"

PRIVATE bool_t argsallowed;	/* nonzero to allow args in declarator */
PRIVATE sym_t gvarsc;		/* sc flags of last identifier declared */
PRIVATE char gvar2name[2 + NAMESIZE];	/* space for structure keys and .. */

#define gvarname (gvar2name + 2)	/* last identifier declared */
PRIVATE struct typestruct *gvartype;	/* type of last identifier declared */
PRIVATE bool_t initlistflag;	/* remembers whether initializer is a list */

FORWARD struct typestruct *chainprefix P((struct typestruct *pretype,
					   struct typestruct *sufftype));
FORWARD void declaf P((void));
FORWARD void declarator P((void));
FORWARD void declarg P((void));
FORWARD struct typestruct *declenum P((void));
FORWARD void declfunc P((void));
FORWARD void declselt P((struct typestruct *structype, offset_T *psoffset,
			 struct typelist ** ptypelist));
FORWARD bool_pt declspec P((void));
FORWARD struct typestruct *declsu P((void));
FORWARD void idecllist P((void));
FORWARD void initarray P((struct typestruct *type));
FORWARD void inititem P((struct typestruct *type));
FORWARD void initstruct P((struct typestruct *type));
FORWARD void lbrace P((void));
FORWARD void multidecl P((char *sname));
FORWARD void need P((int charneeded));
FORWARD void rdeclarator P((void));
FORWARD bool_pt regdecl P((void));

PRIVATE struct typestruct *chainprefix(pretype, sufftype)
struct typestruct *pretype;
struct typestruct *sufftype;
{
    if (pretype->nexttype != NULL)
    {
	sufftype = chainprefix(pretype->nexttype, sufftype);
	if (pretype->constructor != ARRAY)
	    return prefix(pretype->constructor, pretype->typesize, sufftype);
	else
	    return prefix(ARRAY,
			  pretype->typesize / pretype->nexttype->typesize
					    * sufftype->typesize, sufftype);
    }
    return sufftype;
}

PUBLIC void colon()
{
    if (sym != COLON)
	need(':');
    else
	nextsym();
}

/* decllist() - list of local declarations */
/* this does the non-terminal symbols declaration-list, type-declaration */
/* list and declaration */
/* the different cases are and ";" at the end are decided by idecllist() */

PUBLIC void decllist()
{
    while (declspec())
    {
	if (gvarsc == NULLDECL)
	    gvarsc = AUTODECL;
	idecllist();
    }
}

PRIVATE void declaf()
{
    uoffset_T asize;
    bool_t levelnew;
#ifdef TESTING_PROTOTYPES
    char ogvarname[NAMESIZE];
    sym_t ogvarsc;
    struct symstruct *ogvarsymptr;
    struct typestruct *ogvartype;
#endif

    if (sym == LPAREN)		/* function */
    {
	nextsym();
	if (argsallowed && level == GLBLEVEL)
	{
	    newlevel();
	    levelnew = TRUE;
	}
	else
	    levelnew = FALSE;
#ifdef TESTING_PROTOTYPES
	strcpy(ogvarname, gvarname);
	ogvarsc = gvarsc;
	ogvarsymptr = gvarsymptr;
	ogvartype = gvartype;
	if (declspec())
	{
	    if (level == ARGLEVEL)
		outstr(gvarname);
	    outbyte('(');
	    do
	    {
		declarator();
		dbtype(gvartype);
		outbyte(' ');
#if 0
	    if (gvarname[0] != 0 && gsymptr != NULL
		    && gsymptr->level == level)
		    error("repeated argument");
#endif
		outstr(gvarname);
		if (levelnew)
		    addloc(gsname, gvartype);  /* adjust offsets later */
		if (sym != COMMA)
		    break;
		outcomma();
		nextsym();
	    }
	    while (declspec());
	    if (sym == STRUCELTOP)
	    {
		/* ... should be lexed. */
		nextsym();
		if (sym == STRUCELTOP)
		{
		    nextsym();
		    if (sym == STRUCELTOP)
		    {
			nextsym();
			outstr(" ...");
		    }
		}
	    }
	    outbyte(')');
	    if (level == ARGLEVEL)
		outnl();
	}
	else
	{
#endif /* TESTING_PROTOTYPES */
	    while (sym == IDENT)
	    {
		if (gsymptr != NULL && gsymptr->level == level)
		    error("repeated argument");
		if (levelnew)
		    addloc(gsname, itype);  /* adjust types and offsets later */
		nextsym();
		if (sym != COMMA)
		    break;
		nextsym();
	    }
#ifdef TESTING_PROTOTYPES
	}
	strcpy(gvarname, ogvarname);
	gvarsc = ogvarsc;
	gvarsymptr = ogvarsymptr;
	gvartype = ogvartype;
#endif /* TESTING_PROTOTYPES */
	rparen();
	declaf();
	switch (gvartype->constructor)
	{
	case ARRAY:
	    error("function returning array is illegal");
	    break;
	case FUNCTION:
	    error("function returning function is illegal");
	    break;
	}
	gvartype = prefix(FUNCTION, ftypesize, gvartype);
    }
    else if (sym == LBRACKET)
    {
	nextsym();
	if (sym == RBRACKET)
	    asize = 0;
	else
	    asize = constexpression() & intmaskto;  /* FIXME: warn overflow */
	rbracket();
	declaf();
	if (gvartype->typesize == 0)
	    switch (gvartype->constructor)
	    {
	    case FUNCTION:
		error("array of functions is illegal");
		break;
	    case STRUCTU:
		error("undefined structure");
		break;
	    default:		/* array */
		error("null dimension");
		gvartype->typesize = 1;
	    }
	gvartype = prefix(ARRAY, asize * gvartype->typesize, gvartype);
    }
}

PRIVATE void declarator()
{
    rdeclarator();
    if (gvartype->constructor == STRUCTU && gvartype->typesize == 0 &&
	gvarsc != TYPEDEFDECL && gvarsc != EXTERNDECL)
	error("undefined structure");
}

PRIVATE void declarg()
{
    if (gvarsymptr->level != ARGLEVEL)
	error2error(gvarname, " not in argument list");
    else if (gvarsymptr->flags & INITIALIZED && gvarsymptr->type != gvartype)
	multidecl(gvarname);
    else
    {
	gvarsymptr->flags = INITIALIZED; /* flag otherwise unused for locals */
	if (gvartype->constructor & (ARRAY | POINTER))
	    gvartype = pointype(gvartype->nexttype);
	gvarsymptr->type = gvartype;
	if (gvarsc == REGDECL && regdecl())
	    regarg = TRUE;
    }
}

/*
enum-specifier =
	  "enum" [identifier] "{" enumerator {, enumerator} "}"
	| "enum" identifier.
enumerator =
	  identifier
	| identifier = constant-expression.
*/

PRIVATE struct typestruct *declenum()
{
    register struct symstruct *esymptr;
    register struct typestruct *enumtype;
    offset_T ordinal;

    nextsym();
    if (sym != IDENT && sym != TYPEDEFNAME)
	enumtype = addstruct("");
    else if ((esymptr = findstruct(gsname)) == NULL)
    {
	enumtype = addstruct(gsname);
	nextsym();
    }
    else
    {
	nextsym();
	enumtype = esymptr->type;
	if (sym != LBRACE)
	    return enumtype;
	if (esymptr->level != level)
	    enumtype = addstruct(esymptr->name.namea);
    }
    lbrace();
    if (enumtype->typesize != 0)
	multidecl(enumtype->tname);
    else
    {
	enumtype->typesize = itypesize;
	enumtype->alignmask = itype->alignmask;
    }
    if (sym != IDENT)
	error("empty enumerator list");
    ordinal = 0;
    while (sym == IDENT)
    {
	if ((esymptr = gsymptr) != NULL && esymptr->level == level)
	    multidecl(gsname);
	else
	{
	    (esymptr = addlorg(gsname, itype))->storage = CONSTANT;
	    esymptr->indcount = 0;
	}
	nextsym();
	if (sym == ASSIGNOP)
	{
	    nextsym();
	    ordinal = constexpression() & intmaskto;	/* FIXME: warn ovflo */
	    if (ordinal > maxintto)
		ordinal -= (maxuintto + 1);
	}
	if (esymptr->storage == CONSTANT)
	    esymptr->offset.offv = ordinal++;
	if (sym != COMMA)
	    break;
	nextsym();
    }
    rbrace();
    return enumtype;
}

/* declselt - get list of declarations for a structure/union member */

PRIVATE void declselt(structype, psoffset, ptypelist)
struct typestruct *structype;
offset_T *psoffset;
struct typelist **ptypelist;
{
    struct typestruct *basetype;
    value_t fieldwidth;
    value_t fwidth;
    offset_T offset;
    scalar_t scalar=0;

    offset = *psoffset;
    declspec();
    if (gvarsc != NULLDECL)
	error("illegal type name");
    basetype = gvartype;
    while (sym != SEMICOLON && sym != EOFSYM)
    {
	gvartype = basetype;
	declarator();
	fieldwidth = -1;
	if (sym == COLON)
	{
	    scalar = gvartype->scalar;
	    if (!(scalar & ISCALAR))
		error("bitfield has non-integral type");
	    nextsym();
	    fwidth = constexpression();
	    if (fwidth < 0)
		error("bitfield has negative width");
	    else if (fwidth > INT32BITSTO)	/* XXX */
		error("bitfield is too wide");
	    else
		fieldwidth = fwidth;
	}
	if (gvarname[0] == 0)
	{
	    if (fieldwidth == 0)
		offset = (offset + ~gvartype->alignmask) & gvartype->alignmask;
	    else
		/* XXX - should really align to fieldwidth'th bit. */
		offset = (offset + ~gvartype->alignmask) & gvartype->alignmask;
	}
	else
	{
	    if (fieldwidth == 0)
		error("bitfield has zero width");
	    else if (fieldwidth > 0 && scalar & ISCALAR)
	    {
		if (fieldwidth <= CHBITSTO)	/* XXX! */
		    gvartype = ctype;
		else if (fieldwidth <= INT32BITSTO)	/* XXX */
		    gvartype = itype;
		else
		    gvartype = ltype;
		if (scalar & UNSIGNED)
		    gvartype = tounsigned(gvartype);
	    }
	    gvar2name[0] = structype->structkey[0];
	    gvar2name[1] = structype->structkey[1];
	    /* key provides unique name space */
	    if (findlorg(gvar2name) != NULL)
		multidecl(gvarname);
	    else
	    {
		(gvarsymptr = addlorg(gvar2name, gvartype))->storage
		    = NOSTORAGE;
		structype->alignmask &= gvartype->alignmask;
		offset = (gvarsymptr->offset.offi =
			  (offset+ ~gvartype->alignmask) & gvartype->alignmask)
			  + gvartype->typesize;
		{
		    register struct typelist *newtypelist;

		    newtypelist = qmalloc(sizeof *newtypelist);
#ifdef TS
++ts_n_newtypelist;
ts_s_newtypelist += sizeof *newtypelist;
#endif
		    newtypelist->tlnext = NULL;
		    newtypelist->tltype = gvartype;
		    if (*ptypelist == NULL)
			structype->listtype = newtypelist;
		    else
			(*ptypelist)->tlnext = newtypelist;
		    *ptypelist = newtypelist;
		}
	    }
	}
	if (sym != COMMA)
	    break;
	nextsym();
    }
    semicolon();
    *psoffset = offset;
}

PRIVATE bool_pt declspec()
{
    unsigned nsc;
    unsigned nsigned;
    unsigned ntype;
    unsigned nunsigned;

    gvarsc = NULLDECL;
    gvartype = itype;
    nsigned = nunsigned = ntype = nsc = 0;
    while (TRUE)
    {
	switch (sym)
	{
	case AUTODECL:
	    ++nsc;
	    gvarsc = sym;
	    nextsym();
	    break;
	case ENUMDECL:
	    ++ntype;
	    declenum();
	    gvartype = itype;	/* kludge - ignore type of enum */
	    break;
	case EXTERNDECL:
	    ++nsc;
	    gvarsc = sym;
	    nextsym();
	    break;
	case REGDECL:
	    ++nsc;
	    gvarsc = sym;
	    nextsym();
	    break;
	case STATICDECL:
	    ++nsc;
	    gvarsc = sym;
	    nextsym();
	    break;
	case STRUCTDECL:
	case UNIONDECL:
	    ++ntype;
	    gvartype = declsu();
	    break;
	case TYPEDECL:
	    if (ntype == 1)
	    {
		/* allow short int and long int */
		if (gvartype == itype
		    && (gsymptr->type == stype || gsymptr->type == ltype))
		    ntype = 0;

		/* Allow long double and long float */
		if (gvartype == ltype 
		    && (gsymptr->type == fltype || gsymptr->type == dtype))
		{
		    gvartype = dtype;
		    nextsym();
		    break;
		}

		/* allow int short and int long, blech */
		if (gsymptr->type == itype
		    && (gvartype == stype || gvartype == ltype))
		{
		    nextsym();
		    break;
		}
	    }
	    ++ntype;
	    gvartype = gsymptr->type;
	    nextsym();
	    break;
	case TYPEDEFDECL:
	    ++nsc;
	    gvarsc = sym;
	    nextsym();
	    break;
	case TYPEDEFNAME:
	    if (ntype != 0)
		goto break2;
	    ++ntype;
	    gvartype = gsymptr->type;
	    nextsym();
	    break;
        case SIGNDECL:
            ++nsigned;
            nextsym();
            break;
	case UNSIGNDECL:
	    ++nunsigned;
	    nextsym();
	    break;
	case ASMSYM:
	    nextsym();
	    doasm();
	    break;
	default:
	    goto break2;
	}
    }
break2:
    if (nsigned > 0)
    {
	if (ntype == 0)
	{
	    gvartype = itype;
	    ntype = 1;
	}
	gvartype = tosigned(gvartype);
	if (nsigned > 1 || nunsigned > 0)
	    ntype = 2;
    }
    if (nunsigned > 0)
    {
	if (ntype == 0)
	{
	    gvartype = uitype;
	    ntype = 1;
	}
	gvartype = tounsigned(gvartype);
	if (nunsigned > 1 || nsigned > 0)
	    ntype = 2;
    }
    if (nsc > 0)
    {
	if (ntype == 0)
	    ntype = 1;
	if (nsc > 1)
	    error("multiple storage classes in declaration");
    }
    if (ntype > 1)
	error("multiple types in declaration");
    return nsc > 0 || ntype > 0;
}

/* declsu - get structure or union name and/or declaration, return type */

PRIVATE struct typestruct *declsu()
{
    sym_t ogvarsc;
    offset_T soffset;
    offset_T struclength;
    sym_t susym;
    struct typelist *typelist;
    struct typestruct *structype;

    susym = sym;
    nextsym();
    if (sym != IDENT && sym != TYPEDEFNAME)
	structype = addstruct("");
    else if ((gvarsymptr = findstruct(gsname)) == NULL)
    {
	structype = addstruct(gsname);
	nextsym();
    }
    else
    {
	nextsym();
	if (sym == LBRACE && gvarsymptr->level != level)
	    structype = addstruct(gvarsymptr->name.namea);
	else
	    structype = gvarsymptr->type;
    }
    if (sym != LBRACE)
	return structype;
    if (structype->typesize != 0)
	multidecl(structype->tname);
    nextsym();
    ogvarsc = gvarsc;
    struclength = soffset = 0;
    typelist = NULL;
    while (sym != RBRACE && sym != EOFSYM)
    {
	declselt(structype, &soffset, &typelist);
	if (susym == STRUCTDECL)
	    struclength = soffset;
	else
	{
	    if (struclength < soffset)
		struclength = soffset;
	    soffset = 0;
	}
    }
    structype->typesize = (struclength + ~structype->alignmask) &
	structype->alignmask;
    rbrace();
    gvarsc = ogvarsc;
    return structype;
}

/* declfunc() - function name and parameter list */

PRIVATE void declfunc()
{
    offset_T argsp;
    uoffset_T argsize;
    struct symstruct *symptr;
    int main_flag = 0;

    strcpy(funcname, gvarname);
    if( strcmp(funcname, "main") != 0 ) main_flag = -1;
    if (gvarsymptr == NULL)
	gvarsymptr = addglb(gvarname, gvartype);
    else if (gvarsymptr->type != gvartype ||
	     (gvarsymptr->flags & INITIALIZED))
	multidecl(gvarname);	/* different type or full declare */
    gvarsymptr->flags = INITIALIZED;	/* show full declare */
    cseg();
    if (gvarsc == STATICDECL)
	private(gvarname);	/* don't need to put STATIC in flags */
    else
	public(gvarname);
    callee1mask = calleemask;
#ifdef FRAMEPOINTER
    frame1list = framelist;
#endif
    func1saveregsize = funcsaveregsize;
#if 0
    if ((returntype = gvartype->nexttype)->scalar & RSCALAR)
#else
    if ((returntype = gvartype->nexttype)->scalar & DOUBLE)
#endif
    {
	callee1mask &= ~doublreturnregs;
#ifdef FRAMEPOINTER
	frame1list &= ~doublreturnregs;
#endif
	func1saveregsize = funcdsaveregsize;
    }

    arg1size =		/* would already be 0 unless last function empty */
#ifdef FRAMEPOINTER
	framep =
	stackarg =
#endif
	reguse = 0;	/* would already be 0 unless last f had reg vars */
    decllist();
    argsp = returnadrsize;
    if (returntype->constructor & STRUCTU)
	argsp += ptypesize;
    softsp = -func1saveregsize;
    for (symptr = &locsyms[0];
	 symptr < locptr && symptr->flags != STRUCTNAME &&
	 *symptr->name.namea >= 'A';
	 symptr = (struct symstruct *)
		  align(&symptr->name.namea[strlen(symptr->name.namea) + 1]))
    {
        if( main_flag >= 0 ) main_flag++;
	/* convert arg sizes to offsets */
	if (symptr->type == fltype)
	    symptr->type = dtype;
	argsize = symptr->type->typesize;
#ifdef FRAMEPOINTER
	if (!arg1inreg || symptr != &locsyms[0] || symptr->flags != REGVAR)
	    stackarg = TRUE;
#endif
	if (arg1inreg && symptr == &locsyms[0] && symptr->flags != REGVAR)
	{
	    if ((arg1size = argsize) < itypesize)
		arg1size = itypesize;
	    argsp = softsp -= arg1size;
	}
#if INT_BIG_ENDIAN
	if (argsize < itypesize)
	    argsp += itypesize - argsize;
	symptr->offset.offi = argsp;
	argsp += argsize;
#else
	symptr->offset.offi = argsp;
	if (argsize > itypesize)
	    argsp += argsize;
	else
	    argsp += itypesize;
#endif
	if (arg1inreg && symptr == &locsyms[0])
	    argsp = returnadrsize;	/* skip return adr after 1st arg */
    }
    if( main_flag > 0 )
    {
       globl("__mkargv");
       if( main_flag > 2 )
          globl("environ");
    }
#ifdef I8088
    regfuse = 0;
#endif
    lbrace();
    compound();
#ifdef I8088
    if (regfuse & (callee1mask | INDREG0)) {
        outstr("! Register");
        if (regfuse & INDREG0              ) outstr(" BX");
        if (regfuse & INDREG1 & callee1mask) outstr(" SI");
	if (regfuse & INDREG2 & callee1mask) outstr(" DI");
	if (regfuse & LOCAL & callee1mask) outstr(" BP");
	outstr(" used in function ");
	outnstr(funcname);
	if (optimise && !callersaves) {
	    outstr(funcname);
	    outnstr(".off = 0");
	}
    } else
	if (optimise && !callersaves) {
	    outstr(funcname);
	    outstr(".off = ");
#ifndef I80386
	    outnhex(4);
#else
	    outnhex(i386_32?12:4);
#endif
	}
#endif
    clearfunclabels();
}

/* idecllist() - list of init-declarations with a given base type */
/* this does the non-terminal symbols list( init-declarator ) ";" */
/* and list( type-declaration ) ";" */
/* and complete declarations of functions */

PRIVATE void idecllist()
{
    struct typestruct *basetype;
    struct typestruct *inittype;
    scopelev_t levelmark;
    uoffset_T newoffset;

    argsallowed = level == GLBLEVEL;
    levelmark = level;
    basetype = gvartype;
    while (sym != SEMICOLON)
    {
	gvartype = basetype;
	declarator();
	if (gvarname[0] == 0)
	    needvarname();
	if (gvartype->constructor == FUNCTION)
	{
	    if (sym != COMMA && sym != SEMICOLON)
	    {
		if (level == ARGLEVEL && gvarsc != TYPEDEFDECL)
		    declfunc();
		else
		    error("illegal non-external function");
		if (level != levelmark)
		{
		    oldlevel();
		    locptr = &locsyms[0];
		}
		argsallowed = FALSE;
		return;
	    }
	}
	if (level != levelmark)
	{
	    oldlevel();
	    locptr = &locsyms[0];
	}
	argsallowed = FALSE;
	if (level == ARGLEVEL)
	{
	    if (gvarsymptr == NULL)
		gvarsymptr = addglb(gvarname, gvartype);	/* error soon */
	    declarg();
	}
	else if (gvarsymptr != NULL && (gvarsymptr->level == level ||
					gvartype->constructor == FUNCTION ||
					(gvarsc == EXTERNDECL &&
					gvarsymptr->level == GLBLEVEL)))
	{
	    if (gvarsymptr->level != GLBLEVEL || gvarsymptr->flags == KEYWORD)
		multidecl(gvarname);
	    else if (gvarsymptr->type != gvartype)
	    {
		if (gvartype->constructor != ARRAY ||
		    gvarsymptr->type->constructor != ARRAY ||
		    gvartype->nexttype != gvarsymptr->type->nexttype ||
		    (gvartype->typesize != 0 &&
		    gvarsymptr->type->typesize != 0))
		    multidecl(gvarname);
		else if (gvartype->typesize != 0)
		    gvarsymptr->type = gvartype;
	    }
	    if (gvarsc == NULLDECL)
		gvarsymptr->flags &= ~EXTERNAL;
	}
	else if (level == GLBLEVEL || gvartype->constructor == FUNCTION ||
		 gvarsc == EXTERNDECL)
	{
	    gvarsymptr = addglb(gvarname, gvartype);
#ifdef DIRECTPAGE /* make all global scalar and pointer variables DIRECTPAGE */
	    if (!(gvartype->constructor & ~POINTER))
		gvarsymptr->flags = DIRECTPAGE;
#endif
	    if (gvarsc == EXTERNDECL)
		gvarsymptr->flags |= EXTERNAL;
	}
	else
	{
	    gvarsymptr = addloc(gvarname, gvartype);
	    if ((gvarsc != REGDECL || !regdecl()) &&
		gvartype->constructor != FUNCTION && gvarsc != STATICDECL &&
		gvarsc != TYPEDEFDECL)
		gvarsymptr->offset.offi = softsp =
			(softsp & gvartype->alignmask) - gvartype->typesize;
	}
	if (gvarsc == STATICDECL)
	{
	    gvarsymptr->flags &= ~EXTERNAL;
	    gvarsymptr->flags |= STATIC;
	    if (gvarsymptr->level != GLBLEVEL)
	    {
		gvarsymptr->storage = GLOBAL;
		gvarsymptr->offset.offlabel = getlabel();
		/* must leave gvarsymptr->name.label alone */
		/* since name is required for findlorg() */
		/* label is activated by expression() */
	    }
	}
	if (gvarsc == TYPEDEFDECL)
	{
	    gvarsymptr->flags = KEYWORD;
	    gvarsymptr->offset.offsym = TYPEDEFNAME;
	}
	if (sym == ASSIGNOP)
	{
	    if (gvarsymptr->flags & INITIALIZED)
		multidecl(gvarname);
	    nextsym();
	    if (level == GLBLEVEL || gvarsc == STATICDECL)
	    {
#ifndef DIRECTPAGE
		dseg();
#else
		if (gvarsymptr->flags & DIRECTPAGE)
		    dpseg();
		else
		    dseg();
#endif
		if ((inittype = gvartype)->constructor == ARRAY)
		    inittype = inittype->nexttype;
		newoffset = (dataoffset + ~inittype->alignmask)
			  & inittype->alignmask;
		defnulls(newoffset - dataoffset);
		dataoffset = newoffset;
		if (gvarsc == STATICDECL)
		{
		    if (level == GLBLEVEL)
			private(gvarname);
		    else
			outnlabel(gvarsymptr->offset.offlabel);
		}
		else
		    public(gvarname);
		initlistflag = TRUE;
		if (sym != LBRACE)
		    initlistflag = FALSE;
		inititem(gvartype);
		dataoffset += (gvarsymptr->type = gvartype)->typesize;
		if (gvarsymptr->level == GLBLEVEL)
		    gvarsymptr->flags |= INITIALIZED;
		if (level != GLBLEVEL)
		    cseg();
	    }
	    else
	    {
		if (gvarsc == EXTERNDECL)
		    error("initialization of externs is illegal");
		switch (gvartype->constructor)
		{
		case ARRAY:
		    error("initialization of auto arrays is illegal");
		    break;
		case FUNCTION:
		    error("initialization of functions is illegal");
		    break;
		default:
		    initexpression(gvartype);
		    break;
		}
	    }
	}
	else if (level != GLBLEVEL && gvarsc == STATICDECL &&
		 gvartype->constructor != FUNCTION)
	{
#ifndef DIRECTPAGE
	    bssseg();
#else
	    if (gvarsymptr->flags & DIRECTPAGE)
		dpseg();
	    else
		dseg();
#endif
	    lcommlab(gvarsymptr->offset.offlabel);
	    outnhex(gvartype->typesize);
	    cseg();
	}
	if (sym != COMMA)
	    break;
	nextsym();
    }
    semicolon();
    argsallowed = FALSE;
}

PRIVATE void initarray(type)
struct typestruct *type;
{
    uoffset_T basesize;
    struct typestruct *basetype;
    uoffset_T dimension;
    char *stringend;
    uoffset_T stringlength;
    uoffset_T remaining;

    if ((basesize = (basetype = type->nexttype)->typesize) == 0)
	dimension = remaining = 0;
    else
	dimension = remaining = type->typesize / basesize;
    if (sym == STRINGCONST && (basetype->scalar & CHAR))
    {
	stringlength = (stringend = charptr) - constant.value.s;
	if (remaining != 0 && stringlength >= remaining)
	    /* same dim should be allowed but defstr always nullterms */
	{
	    error("string longer than dimension");
	    stringend = constant.value.s + (/* size_t */ unsigned)
					   (stringlength = remaining - 1);
	}
	defstr(constant.value.s, stringend, TRUE);
	remaining -= (stringlength + 1);
	nextsym();
    }
    else
	do
	    inititem(basetype);
	while (--remaining != 0 && sym == COMMA && initlistflag &&
	       (nextsym(), sym != RBRACE));
    if (dimension == 0)
    {
	if (type == gvartype)
	    gvartype = prefix(ARRAY, -remaining * basesize, basetype);
    }
    else
	defnulls(remaining * basesize);
}

PRIVATE void inititem(type)
struct typestruct *type;
{
    sym_t startsym;

    if ((startsym = sym) == LBRACE)
	nextsym();
    switch (type->constructor)
    {
    case ARRAY:
	initarray(type);
	break;
    case STRUCTU:
	initstruct(type);
	break;
    case FUNCTION:
	error("initialization of functions is illegal");
	break;
    default:
	initexpression(type);
    }
    if (startsym == LBRACE)
    {
	if (sym == COMMA)
	{
	    nextsym();
	    if (sym != RBRACE)
		error("too many initializers");
	}
	rbrace();
    }
}

PRIVATE void initstruct(type)
struct typestruct *type;
{
    struct typestruct *memtype;
    uoffset_T newoffset;
    uoffset_T offset;
    struct typelist *typelist;

    offset = 0;
    if ((typelist = type->listtype) != NULL)
	do
	{
	    memtype = typelist->tltype;
	    newoffset = (offset + ~memtype->alignmask) & memtype->alignmask;
	    defnulls(newoffset - offset);
	    offset = newoffset + memtype->typesize;
	    inititem(memtype);
	}
	while ((typelist = typelist->tlnext) != NULL && sym == COMMA &&
	       initlistflag && (nextsym(), sym != RBRACE));
    /* eof here will break next time */
    defnulls(type->typesize - offset);
}

PRIVATE void lbrace()
{
    if (sym != LBRACE)
	need('{');
    else
	nextsym();
}

PUBLIC void lparen()
{
    if (sym != LPAREN)
	need('(');
    else
	nextsym();
}

PRIVATE void multidecl(sname)
char *sname;
{
    error2error(sname, " already declared");
}

PRIVATE void need(charneeded)
int charneeded;
{
    static char message[] = "need 'x'";

    message[6] = charneeded;
    error(message);
}

PUBLIC void needvarname()
{
    error("need variable name");
    nextsym();
}

PUBLIC void program()
{
#ifdef BUILTIN_CPP
    if (orig_cppmode)
	cppscan(0);
    else
#endif
    {
	nextsym();
	while (sym != EOFSYM)
	{
	    declspec();
	    idecllist();
	}
    }
}

PUBLIC void rbrace()
{
    if (sym != RBRACE)
	need('}');
    else
	nextsym();
}

PUBLIC void rbracket()
{
    if (sym != RBRACKET)
	need(']');
    else
	nextsym();
}

PRIVATE void rdeclarator()
{
    while (sym == STAR)
    {
	nextsym();
	gvartype = pointype(gvartype);
    }
    if (sym == LPAREN)
    {
	struct typestruct *pretype;
	struct typestruct *sufftype;

	nextsym();
	sufftype = gvartype;
	gvartype = ctype;
	rdeclarator();
	rparen();
	pretype = gvartype;
	gvartype = sufftype;
	declaf();
	gvartype = chainprefix(pretype, gvartype);
    }
    else if (sym == IDENT || sym == TYPEDEFNAME)
    {
	strcpy(gvarname, gsname);
	gvarsymptr = gsymptr;
	nextsym();
    }
    else
    {
	gvarname[0] = 0;
	gvarsymptr = NULL;
    }
    declaf();
}

PRIVATE bool_pt regdecl()
{
    store_t regavail;

#if !NOTFINISHED
    if (gvarsymptr->type->constructor != POINTER)
	return FALSE;
#endif
#ifdef MC6809
    if (gvarsymptr->type->constructor != POINTER)
	return FALSE;
#endif
    if (!(regavail = regregs & ~reguse))
	return FALSE;
    gvarsymptr->flags = REGVAR;
    gvarsymptr->indcount = 0;
#define LOWBITMASK(bits) (bits - (bits & (bits - 1)))	/* & chops low bit */
    reguse |= gvarsymptr->storage = LOWBITMASK(regavail);
    return TRUE;
}

PUBLIC void rparen()
{
    if (sym != RPAREN)
	need(')');
    else
	nextsym();
}

PUBLIC void semicolon()
{
    outnstr("!BCC_EOS");
    if (sym != SEMICOLON)
	need(';');
    else
	nextsym();
}

PUBLIC struct typestruct *typename()
{
    char ogvarname[NAMESIZE];
    sym_t ogvarsc;
    struct symstruct *ogvarsymptr;
    struct typestruct *ogvartype;
    struct typestruct *type;

    /* the global variable data must be preserved */
    /* although this is only called for casts and sizeof, */
    /* the casts can be in initialisers */
    /* and the sizeof in an array size expression */
    strcpy(ogvarname, gvarname);
    ogvarsc = gvarsc;
    ogvarsymptr = gvarsymptr;
    ogvartype = gvartype;
    if (declspec())
    {
	declarator();
	if (gvarsc != NULLDECL || gvarname[0] != 0)
	    error("illegal type name");
	type = gvartype;
    }
    else
	type = NULL;
    strcpy(gvarname, ogvarname);
    gvarsc = ogvarsc;
    gvarsymptr = ogvarsymptr;
    gvartype = ogvartype;
    return type;
}
