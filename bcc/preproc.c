/* preprocessor routines for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"
#include "input.h"
#include "os.h"
#include "output.h"
#include "parse.h"
#include "sc.h"
#include "scan.h"
#include "table.h"
#include "type.h"

/* blanksident() - return nonzero if at blanks followed by an identifier */

PUBLIC bool_pt blanksident()
{
    blanks();
    return isident();
}

#ifdef BUILTIN_CPP

#define MAX_IF		32
#define MAX__LINE__	10	/* enough for 32-bit source unsigneds */
#define MAX_MACRO	32
#define MAX_PARAM	127	/* max char with no sign on all machines */

/* Definition types.  These are kept in the 'storage' member of struct
 * symstruct and must be distinct from 'LOCAL' because dumplocs() doesn't
 * check.
 */
enum
{
    DEF_LINE,			/* __LINE__ keyword */
    DEF_NONE			/* nothing special */
};

struct ifstruct
{
    bool_t elseflag;
    bool_t ifflag;
};

struct macroposition
{
    char *maclineptr;
    char **paramlist;
    char *paramspot;
    bool_t inparam;
    indn_t nparam;
    struct symstruct *symptr;
};

PRIVATE char dummyparam[] = { EOL, 0 };
PRIVATE fastin_t iflevel;	/* depends on zero init */
PRIVATE struct ifstruct ifstate;
				/* elseflag depends on zero init */
PRIVATE struct ifstruct ifstack[MAX_IF];

PRIVATE struct macroposition macrostack[MAX_MACRO];

FORWARD void asmcontrol P((void));
FORWARD void warningcntl P((void));
FORWARD void errorcntl P((void));
FORWARD void control P((void));
FORWARD void defineorundefinestring P((char *str, bool_pt defineflag));
FORWARD void elifcontrol P((void));
FORWARD void elsecontrol P((void));
FORWARD void endif P((void));
FORWARD fastin_pt getparnames P((void));
FORWARD void ifcontrol P((sym_pt ifcase));
FORWARD void undef P((void));

/* asmcontrol() - process #asm */

PRIVATE void asmcontrol()
{
#ifdef ASM_BARE
    char treasure;		/* to save at least one leading blank */
#endif

    asmmode = TRUE;
    if (expect_statement)
       return;

    if (orig_cppmode)
	outnstr("#asm");
    else
    {
	outnstr("!BCC_ASM");
	dumplocs();
    }
#ifndef ASM_BARE
    cppscan(1);
#else
    while (TRUE)
    {
	skipline();
	skipeol();
	if (eofile)
	{
	    eofin("#asm");
	    break;
	}
	if (SYMOFCHAR(ch) == SPECIALCHAR)
	    specialchar();
	treasure = 0;
	if (SYMOFCHAR(ch) == WHITESPACE)
	    treasure = ch;
	blanks();
	if (ch == '#')
	{
	    if (ctext)
	    {
		register char *lptr;

		comment();
		if (treasure != 0)
		    outbyte(treasure);
		lptr = lineptr;
		while (*lptr++ != EOL)	/* XXX - handle COEOL too */
		    outbyte(ch);
		outnl();
	    }
	    gch1();
	    docontrol();
	    if (!asmmode)
		break;
	}
	else
	{
	    if (treasure != 0)
		outbyte(treasure);
	    while (ch != EOL)	/* XXX - handle COEOL too */
	    {
		outbyte(ch);
		gch1();
	    }
	    outnl();
	}
    }
#endif
    if (orig_cppmode)
	outnstr("#endasm");
    else
	outnstr("!BCC_ENDASM");
}

PUBLIC void checknotinif()
{
    while (iflevel != 0)
    {
	if (ifstate.ifflag)
	    eofin("true #conditional");
	else
	    eofin("false #conditional");
	endif();
    }
}

/* warningcntl() - process #warning */

PRIVATE void warningcntl()
{
    char estr[256], *ep = estr;

    if (!orig_cppmode) {
       *ep++ = '%'; *ep++ = 'w'; 
    }
    while( ch != EOL ) {
       if (ep < estr+sizeof(estr)-2 )
	  *ep++ = ch;
       gch1();
    }
    *ep = 0;

    if (!orig_cppmode)
       error(estr);
    else {
       outstr("#warning");
       outnstr(estr);
    }
}

/* errorcntl() - process #error */

PRIVATE void errorcntl()
{
    char estr[256], *ep = estr;

    while( ch != EOL ) {
       if (ep < estr+sizeof(estr)-2 )
	  *ep++ = ch;
       gch1();
    }
    *ep = 0;

    if (!orig_cppmode)
       error(estr);
    else {
       outstr("#error");
       outnstr(estr);
    }
}

/* control() - select and switch to control statement */

PRIVATE void control()
{
    char sname[NAMESIZE + 1];
    sym_t ctlcase;
    struct symstruct *symptr;
    if (ctext && asmmode)
    {
        comment();
        outudec(input.linenumber);
        outbyte(' ');
        outline(lineptr);
    }

    sname[0] = '#';		/* prepare for bad control */
    sname[1] = 0;
    while (blanksident())
    {
	if ((gsymptr = findlorg(gsname)) == NULL ||
	    gsymptr->flags != DEFINITION)
	{
	    strcat(sname, gsname);
	    break;
	}
	entermac();
    }
    if (sname[1] == 0 && ch == EOL)
	return;
    if (SYMOFCHAR(ch) == INTCONST)
	{ linecontol(); return; }
    if ((symptr = findlorg(sname)) == NULL)
    {
	if (ifstate.ifflag)
	    error(" bad control");
	return;
    }
    ctlcase = symptr->offset.offsym;
    if (ifstate.ifflag == FALSE &&
	(ctlcase < ELIFCNTL || ctlcase > IFNDEFCNTL))
	return;
    switch (ctlcase)
    {
    case ASMCNTL:
        if (asmmode) 
	{
	   if (ifstate.ifflag)
	      error(" bad control");
	}
	else
	   asmcontrol();
	break;
    case DEFINECNTL:
	define();
	break;
    case ELIFCNTL:
	elifcontrol();
	break;
    case ELSECNTL:
	elsecontrol();
	break;
    case ENDASMCNTL:
        if (!asmmode) 
	{
	   if (ifstate.ifflag)
	      error(" bad control");
	}
	asmmode = FALSE;
	break;
    case ENDIFCNTL:
	endif();
	break;
    case IFCNTL:
    case IFDEFCNTL:
    case IFNDEFCNTL:
	ifcontrol(symptr->offset.offsym);
	break;
    case INCLUDECNTL:
	include();
	break;
    case LINECNTL:
	{ linecontol(); break; }
    case UNDEFCNTL:
	undef();
	break;
    case WARNINGCNTL:
	warningcntl();
	break;
    case ERRORCNTL:
	errorcntl();
	break;
    }
}

/* define() - process #define */

/*
  MACRO storage.
  A symbol recording the macro name is added to the symbol table.
  This overrides all current scopes of the name (put at head of hash chain).
  The flags are set to DEFINITION.
  The indcount is 0 if no parameters, else 1 + number of parameters.
  The offset field points to the macro string.
  The macro string is null-terminated but EOL-sentineled.
  It consists of EOL-terminated substrings followed by parameter numbers,
  e.g., junk(x,y)=-x+y is stored as '-', EOL, 1, '+', EOL, 2, EOL, 0.
  Here  1  is for the 1st parameter (start at 1 so 0 can terminate).
  EOL acts as a sentinel for the scanner.
  This choice works well because EOL cannot occur in a macro string.
*/

PUBLIC void define()
{
    char sname[NAMESIZE];
    char quote;
    struct symstruct **hashptr;
    struct symstruct *locmark = NULL; /* for -Wall */
    char *macstring;
    fastin_t nparnames;
    char *oldstring;
    struct symstruct *symptr;

    if (!blanksident())
    {
	error("illegal macro name");
	return;
    }
    strcpy(sname, gsname);	/* will overwrite gsname if parameters */
    if (ch == '(')
    {
	locmark = locptr;
	newlevel();		/* temp storage for parameter names */
	nparnames = getparnames() + 1;
    }
    else
	nparnames = 0;
    blanks();
    macstring = charptr;
    quote = 0;
    while (ch != EOL)
    {
	if (charptr >= char1top)  /* check room for char and end of string */
	    macstring = growobject(macstring, 2);
	if (nparnames && isident())
	{
	    if ((symptr = findlorg(gsname)) != NULL &&
		symptr->level == level)
	    {
#ifdef TS
++ts_n_macstring_param;
ts_s_macstring += 2;
#endif
		*charptr++ = EOL;	/* end current string */
		*charptr++ = symptr->indcount;	/* param to insert */
	    }
	    else
	    {
		if (charptr + strlen(gsname) >= chartop)	/* null too */
		    macstring = growobject(macstring, strlen(gsname) + 1);
#ifdef TS
++ts_n_macstring_ident;
ts_s_macstring += strlen(gsname);;
#endif
		strcpy(charptr, gsname);
		charptr += strlen(gsname);	/* discard null */
	    }
	    continue;
	}
	if (ch == '\\')
	{
	    gch1();
	    *charptr = '\\';
	    *(charptr + 1) = ch;
#ifdef TS
++ts_n_macstring_quoted;
ts_s_macstring += 2;
#endif
	    charptr += 2;
	    gch1();
	    continue;
	}
	if (quote)
	{
	    if (ch == quote)
		quote = 0;
	}
	else if (ch == '"' || ch == '\'')
	    quote = ch;
	else if (ch == '/')
	{
	    if (SYMOFCHAR(*(lineptr + 1)) == SPECIALCHAR)
	    {
		gch1();
		ch = *--lineptr = '/';	/* pushback */
	    }
	    if (*(lineptr + 1) == '*')
	    {
		gch1();
		skipcomment();
#if 0
		/* comment is space in modern cpp's but they have '##' too */
		ch = *--lineptr = ' ';
#else
		continue;
#endif
	    }
	}
#ifdef TS
++ts_n_macstring_ordinary;
++ts_s_macstring;
#endif
	*charptr++ = ch;
	gch1();
    }
    {
	register char *rcp;

	/* strip trailing blanks, but watch out for parameters */
	for (rcp = charptr;
	     rcp > macstring && SYMOFCHAR(*(rcp - 1)) == WHITESPACE
	     && (--rcp == macstring || *(rcp - 1) != EOL); )
	    charptr = rcp;
    }
    if (charptr+1 >= char1top)
	macstring = growobject(macstring, 3);
#ifdef TS
++ts_n_macstring_term;
ts_s_macstring += 3;
#endif
    *charptr++ = ' ';	/* Added to prevent tail recursion on expansion */
    *charptr++ = EOL;
    *charptr++ = 0;
    if (nparnames)
    {
	oldlevel();
	locptr = locmark;
    }
/*  if (asmmode) equ(sname, macstring); */

    if ((symptr = findlorg(sname)) != NULL && symptr->flags == DEFINITION)
    {
	if (strcmp(macstring, oldstring = symptr->offset.offp) != 0)
	    error("%wredefined macro");
	if (strlen(macstring) > strlen(oldstring = symptr->offset.offp))
	    symptr->offset.offp = macstring;
	else
	{
	    strcpy(oldstring, macstring);	/* copy if == to avoid test */
	    charptr = macstring;
	}
	return;
    }
    symptr = qmalloc(sizeof (struct symstruct) + strlen(sname));
#ifdef TS
++ts_n_defines;
ts_s_defines += sizeof (struct symstruct) + strlen(sname);
#endif
    addsym(sname, vtype, symptr);
    symptr->storage = DEF_NONE;
    symptr->indcount = nparnames;
    symptr->flags = DEFINITION;
    symptr->level = GLBLEVEL;
    symptr->offset.offp = macstring;
    if (*(hashptr = gethashptr(sname)) != NULL)
    {
	symptr->next = *hashptr;
	symptr->next->prev = &symptr->next;
    }
    *hashptr = symptr;
    symptr->prev = hashptr;
}

PRIVATE void defineorundefinestring(str, defineflag)
char *str;			/* "name[=def]" or "name def" */
bool_pt defineflag;
{
    char *fakeline;
    unsigned len;
    bool_t old_eof;

    len = strlen(str);
    strcpy(fakeline = (char *) ourmalloc(3 + len + 2 + 2) + 3, str);
    /* 3 pushback, 2 + 2 guards */
#ifdef TS
ts_s_fakeline += 3 + len + 2 + 2;
ts_s_fakeline_tot += 3 + len + 2 + 2;
#endif
    {
	register char *endfakeline;

	endfakeline = fakeline + len;
	endfakeline[0] = EOL;	/* guards any trailing backslash */
	endfakeline[1] = EOL;	/* line ends here or before */
    }
    old_eof = eofile;
    eofile = TRUE;			/* valid after first EOL */
    ch = *(lineptr = fakeline);
    if (defineflag)
    {
	if (blanksident())	/* if not, let define() produce error */
	{
	    blanks();
	    if (ch == '=')
		*lineptr = ' ';
	    else if (ch == EOL)
	    {
		register char *lptr;

		lptr = lineptr;
		lptr[0] = ' ';
		lptr[1] = '1';	/* 2 extra were allocated for this & EOL */
		lptr[2] = EOL;
	    }
	}
	ch = *(lineptr = fakeline);
	define();
    }
    else
	undef();
    eofile = old_eof;
#ifdef TS
ts_s_fakeline_tot -= len + 2 + 2;
#endif
    ourfree(fakeline - 3);
}

PUBLIC void definestring(str)
char *str;			/* "name[=def]" or "name def" */
{
    defineorundefinestring(str, TRUE);
}

/* docontrol() - process control statement, loop till "#if" is true */

PUBLIC void docontrol()
{
    while (TRUE)
    {
	control();
	skipline();
	if (ifstate.ifflag)
	    return;
	while (TRUE)
	{
	    skipeol();
	    if (eofile)
		return;
	    blanks();
	    if (ch == '#')
	    {
		gch1();
		break;
	    }
	    skipline();
	}
    }
}

/* elifcontrol() - process #elif */

PRIVATE void elifcontrol()
{
    if (iflevel == 0)
    {
	error("elif without if");
	return;
    }
    if (ifstate.elseflag) {
	register struct ifstruct *ifptr;

	ifptr = &ifstack[(int)--iflevel];
	ifstate.elseflag = ifptr->elseflag;
	ifstate.ifflag = ifptr->ifflag;

	ifcontrol(IFCNTL);
    } else {
	ifstate.ifflag = ifstate.elseflag;
	ifstate.elseflag = FALSE;
    }
}

/* elsecontrol() - process #else */

PRIVATE void elsecontrol()
{
    if (iflevel == 0)
    {
	error("else without if");
	return;
    }
    ifstate.ifflag = ifstate.elseflag;
    ifstate.elseflag = FALSE;
}

/* endif() - process #endif */

PRIVATE void endif()
{
    if (iflevel == 0)
    {
	error("endif without if");
	return;
    }
    {
	register struct ifstruct *ifptr;

	ifptr = &ifstack[(int)--iflevel];
	ifstate.elseflag = ifptr->elseflag;
	ifstate.ifflag = ifptr->ifflag;
    }
}

/* entermac() - switch line ptr to macro string */

PUBLIC void entermac()
{
    char quote;
    struct symstruct *symptr;
    char **paramhead;
    char **paramlist;
    int ngoodparams;
    int nparleft;
    int lpcount;

    if (maclevel >= MAX_MACRO)
    {
	limiterror("macros nested too deeply (33 levels)");
	return;
    }
    symptr = gsymptr;
    symptr->name.namea[0] |= 0x80; /* SMUDGE macro definition */
    ngoodparams = 0;
    paramhead = NULL;
    if (symptr->indcount != 0)
    {
	nparleft = symptr->indcount - 1;
	if (nparleft == 0)
	    paramhead = NULL;
	else
	    paramhead = ourmalloc(sizeof *paramlist * nparleft);
	paramlist = paramhead;
#ifdef TS
++ts_n_macparam;
ts_s_macparam += sizeof *paramlist * nparleft;
ts_s_macparam_tot += sizeof *paramlist * nparleft;
#endif
	blanks();
	while (ch == EOL && !eofile)
	{
	    skipeol();
	    blanks();
	}
	if (ch != '(')
	{
	    if (nparleft > 0)	/* macro has params, doesn't match bare word */
	    {
		symptr->name.namea[0] &= 0x7f; /* UnSMUDGE macro definition */
		outstr(symptr->name.namea);
		return;
	    }
	    error("missing '('");
	}
	else
	{
	    gch1();
	    while (nparleft)
	    {
		--nparleft;
		++ngoodparams;
		*(paramlist++) = charptr;
		quote = 0;
		lpcount = 1;
		while (TRUE)
		{
		    if (ch == '\\')
		    {
			gch1();
			if (charptr >= char1top)
			    *(paramlist - 1) = growobject(*(paramlist - 1), 2);
#ifdef TS
++ts_n_macparam_string_quoted;
++ts_s_macparam_string;
++ts_s_macparam_string_tot;
#endif
			*charptr++ = '\\';
		    }
		    else if (quote)
		    {
			if (ch == quote)
			    quote = 0;
		    }
		    else if (ch == '"' || ch == '\'')
			quote = ch;
		    else if (ch == '/')
		    {
			if (SYMOFCHAR(*(lineptr + 1)) == SPECIALCHAR)
			{
			    gch1();
			    ch = *--lineptr = '/';	/* pushback */
			}
			if (*(lineptr + 1) == '*')
			{
			    gch1();
			    skipcomment();
			    ch = *--lineptr = ' ';	/* pushback */
			}
		    }
		    else if (ch == '(')
			++lpcount;
		    else if ((ch == ')' && --lpcount == 0) ||
			     (ch == ',' && lpcount == 1))
			break;
		    if (ch == EOL)
			ch = ' ';
		    if (charptr >= char1top)
			*(paramlist - 1) = growobject(*(paramlist - 1), 2);
#ifdef TS
++ts_n_macparam_string_ordinary;
++ts_s_macparam_string;
++ts_s_macparam_string_tot;
#endif
		    *charptr++ = ch;
		    if (*lineptr == EOL)
		    {
			skipeol();	/* macro case disposed of already */
			if (SYMOFCHAR(ch) == SPECIALCHAR)
			    specialchar();
			if (eofile)
			    break;
		    }
		    else
			gch1();
		}
#ifdef TS
++ts_n_macparam_string_term;
++ts_s_macparam_string;
++ts_s_macparam_string_tot;
#endif
		*charptr++ = EOL;
		{
		    register char *newparam;
		    register char *oldparam;
		    unsigned size;

		    oldparam = *(paramlist - 1);
		    size = (/* size_t */ unsigned) (charptr - oldparam);
		    newparam = ourmalloc(size);
#ifdef TS
ts_s_macparam_string_alloced += size;
ts_s_macparam_string_alloced_tot += size;
#endif
		    memcpy(newparam, oldparam, size);
		    *(paramlist - 1) = newparam;
#ifdef TS
ts_s_macparam_string_tot -= charptr - oldparam;
#endif
		    charptr = oldparam;
		}
		if (ch == ',')
		    gch1();
		else
		    break;
	    }
	}
	blanks();
	while (ch == EOL && !eofile)
	{
	    skipeol();
	    blanks();
	}
	if (eofile)
	    eofin("macro parameter expansion");
	if (nparleft)
	{
	    error("too few macro parameters");
	    do
		*(paramlist++) = dummyparam;
	    while (--nparleft);
	}
	if (ch == ')')
	    gch1();
	else if (ch == ',')
	{
	    error("too many macro parameters");

	    /* XXX - should read and discard extra parameters.  Also check
	     * for eof at end.
	     */
	    while (ch != ')')
	    {
		if (ch == EOL)
		{
		    skipeol();
		    if (eofile)
			break;
		    continue;
		}
		gch1();
	    }
	}
	else
	    error("missing ')'");
    }

    if (symptr->storage == DEF_LINE)
    {
	char *str;

	str = pushudec(symptr->offset.offp + MAX__LINE__, input.linenumber);
	memcpy(symptr->offset.offp, str, /* size_t */
	       (unsigned) (symptr->offset.offp + MAX__LINE__ + 1 + 1 - str));
    }

    {
	register struct macroposition *mpptr;

	mpptr = &macrostack[maclevel];
	mpptr->paramlist = paramhead;
	mpptr->maclineptr = lineptr;
	ch = *(lineptr = symptr->offset.offp);
	mpptr->inparam = FALSE;
	mpptr->nparam = ngoodparams;
	mpptr->symptr = symptr;
	mpptr->symptr->name.namea[0] |= 0x80; /* SMUDGE macro definition */
	++maclevel;
    }
/*
    comment();
    outstr("MACRO (level ");
    outudec((unsigned) maclevel);
    outstr(") ");
    outline(lineptr);
*/
}

/* getparnames() - get parameter names during macro definition, return count */

PRIVATE fastin_pt getparnames()
{
    fastin_t nparnames;
    struct symstruct *symptr;

    nparnames = 0;
    gch1();
    while (blanksident())
    {
	if ((symptr = findlorg(gsname)) != NULL &&
	    symptr->level == level)
	    error("repeated parameter");
	symptr = addloc(gsname, itype);
	if (nparnames >= MAX_PARAM)
	    limiterror("too many macro parameters (128)");
	else
	    ++nparnames;	/* number params from 1 */
	symptr->indcount = nparnames;	/* param number */
	blanks();
	if (ch == ',')
	    gch1();
    }
    if (ch != ')')
	error("missing ')'");
    else
	gch1();
    return nparnames;
}

/* ifcontrol - process #if, #ifdef, #ifndef */

PRIVATE void ifcontrol(ifcase)
sym_pt ifcase;
{
    bool_t iftrue;
    struct symstruct *symptr;

    if (iflevel >= MAX_IF)
    {
	limiterror("#if's nested too deeply (33 levels)");
	return;
    }
    {
	register struct ifstruct *ifptr;

	ifptr = &ifstack[(int)iflevel++];
	ifptr->elseflag = ifstate.elseflag;
	ifptr->ifflag = ifstate.ifflag;
	ifstate.elseflag = FALSE;	/* prepare for !(if now)||(if future)*/
    }
    if (ifstate.ifflag)
    {
	if ((sym_t) ifcase != IFCNTL)
	{
	    iftrue = FALSE;
	    if (blanksident() && (symptr = findlorg(gsname)) != NULL &&
		symptr->flags == DEFINITION)
		iftrue = TRUE;
	}
	else
	{
	    incppexpr = TRUE;
	    nextsym();
	    iftrue = constexpression() != 0;
	    incppexpr = FALSE;
	}
	if ((!iftrue && (sym_t) ifcase != IFNDEFCNTL) ||
	    (iftrue && (sym_t) ifcase == IFNDEFCNTL))
	{
	    ifstate.elseflag = TRUE;
	    ifstate.ifflag = FALSE;
	}
    }
}

/* ifinit() - initialise if state */

PUBLIC void ifinit()
{
    ifstate.ifflag = TRUE;
}

PUBLIC int ifcheck()
{
    return (ifstate.ifflag == TRUE);
}

/* leavemac() - leave current and further macro substrings till not at end */

PUBLIC void leavemac()
{
    register struct macroposition *mpptr;

    do
    {
	mpptr = &macrostack[maclevel - 1];
	if (mpptr->inparam)
	{
	    lineptr = ++mpptr->paramspot;
	    mpptr->inparam = FALSE;
	}
	else
	{
	    mpptr->symptr->name.namea[0] &= 0x7F;/* UnSMUDGE macro definition */
	    ch = *++lineptr;	/* gch1() would mess up next param == EOL-1 */
	    if (ch != 0)
	    {
		mpptr->paramspot = lineptr;
		lineptr = mpptr->paramlist[ch - 1];
		mpptr->inparam = TRUE;
	    }
	    else
	    {
		lineptr = mpptr->maclineptr;
		if (mpptr->nparam != 0)
		{
		    register char **paramlist;

#ifdef TS
ts_s_macparam_tot -= sizeof *paramlist * mpptr->nparam;
#endif
		    paramlist = mpptr->paramlist;
		    do
{
#ifdef TS
ts_s_macparam_string_alloced_tot -= strchr(*paramlist, EOL) - *paramlist + 1;
#endif
			ourfree(*paramlist++);
}
		    while (--mpptr->nparam != 0);
		    ourfree(mpptr->paramlist);
		}
		--maclevel;
	    }
	}
    }
    while ((ch = *lineptr) == EOL && maclevel != 0);
}

PUBLIC void predefine()
{
    definestring("__BCC__ 1");
    definestring("__LINE__ 0123456789");	/* MAX__LINE__ digits */
    findlorg("__LINE__")->storage = DEF_LINE;
}

PUBLIC char *savedlineptr()
{
    return macrostack[0].maclineptr;
}

PUBLIC void skipcomment()
{
/* Skip current char, then everything up to '*' '/' or eof. */

    gch1();
    do
    {
	while (TRUE)
	{
	    {
		register char *reglineptr;

		reglineptr = lineptr;
		symofchar['*'] = SPECIALCHAR;
		while (SYMOFCHAR(*reglineptr) != SPECIALCHAR)
		    ++reglineptr;
		symofchar['*'] = STAR;
		lineptr = reglineptr;
		if (*reglineptr == '*')
		    break;
		ch = *reglineptr;
	    }
	    specialchar();
	    if (ch == EOL)
	    {
		skipeol();
		if (eofile)
		    break;
	    }
	    else if (ch != '*')
		gch1();
	}
	gch1();
	if (eofile)
	{
	    eofin("comment");
	    return;
	}
    }
    while (ch != '/');
    gch1();
}

/* skipline() - skip rest of line */

PUBLIC void skipline()
{
    while (TRUE)
    {
	blanks();
	if (ch == EOL)
	    return;
	if (ch == '\\')
	{
	    gch1();
	    if (ch == EOL)	/* XXX - I think blanks() eats \EOL */
		return;
	    gch1();		/* XXX - escape() better? */
	}
	else if (ch == '"' || ch == '\'')
	{
	    stringorcharconst();
	    charptr = constant.value.s;
	}
	else
	    gch1();
    }
}

/* undef() - process #undef */

PRIVATE void undef()
{
    struct symstruct *symptr;

    if (blanksident() && (symptr = findlorg(gsname)) != NULL &&
	symptr->flags == DEFINITION)
	delsym(symptr);
}

PUBLIC void undefinestring(str)
char *str;
{
    defineorundefinestring(str, FALSE);
}

#endif
