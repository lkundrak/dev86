/* table.c - table handler for bcc */

/* Copyright (C) 1992 Bruce Evans */

/* Can I now get rid of EXPRLEVEL?   Yes, except expression symbols must
 * usually be set to some level different from OFFKLUDGELEVEL.
 */

#include "bcc.h"
#include "align.h"
#include "gencode.h"
#include "os.h"
#include "parse.h"
#include "reg.h"
#include "sc.h"
#include "scan.h"
#include "sizes.h"
#include "type.h"

#undef EXTERN
#define EXTERN
#include "table.h"

#define GOLDEN 157		/* GOLDEN/HASHTABSIZE approx golden ratio */
#define HASHTABSIZE 256
#define MARKER ((unsigned) 0x18C396A5L)	/* lint everywhere it is used */
#ifdef VERY_SMALL_MEMORY
#define MAXEXPR 125
#else
#define MAXEXPR 500
#endif
#define MAXLOCAL 100
#define NKEYWORDS 39
#ifdef NOFLOAT
#define NSCALTYPES 10
#else
#define NSCALTYPES 12
#endif
#define STACKSPACE 256		/* punt for normal recursions - switch extra */

struct keywordstruct
{
    char *kwname;
    sym_t kwcode;
};

#ifdef HOLDSTRINGS
struct string
{
    struct string *snext;
    char *sptr;
    char *stop;
    label_no slabel;
};
#endif

struct typedatastruct
{
    char *tdname;
    bool_t tdkeyscalar;
    scalar_t tdscalar;
    smalin_t tdsize;
    struct typestruct **tdtypeptr;
};

PRIVATE struct symstruct exprsyms[MAXEXPR];
PRIVATE struct symstruct *hashtab[HASHTABSIZE];
				/* depends on zero init */
#ifdef HOLDSTRINGS
PRIVATE struct string *laststring;	/* last in list of strings */
					/* depends on NULL init */
#endif
PUBLIC struct symstruct locsyms[MAXLOCAL];

PRIVATE struct symstruct constemplate =
{
    CONSTANT,			/* storage */
    0,				/* indcount */
    0,				/* flags */
    EXPRLEVEL,			/* level */
    NULL,			/* next */
    NULL,			/* prev */
    NULL,			/* type, init later to itype */
    /* offset is a union, set every time constemplate is used */
    /* name is a union, namea member is constant "\0" */
};

PRIVATE struct keywordstruct keywords[NKEYWORDS] =
{
    { "enum", ENUMDECL, },
    { "struct", STRUCTDECL, },
    { "union", UNIONDECL, },
    { "unsigned", UNSIGNDECL, },
    { "signed", SIGNDECL, },

    { "auto", AUTODECL, },
    { "extern", EXTERNDECL, },
    { "register", REGDECL, },
    { "static", STATICDECL, },

    { "typedef", TYPEDEFDECL, },

    { "asm", ASMSYM, },
    { "break", BREAKSYM, },
    { "case", CASESYM, },
    { "continue", CONTSYM, },
    { "default", DEFAULTSYM, },
    { "do", DOSYM, },
    { "else", ELSESYM, },
    { "for", FORSYM, },
    { "goto", GOTOSYM, },
    { "if", IFSYM, },
    { "return", RETURNSYM, },
    { "sizeof", SIZEOFSYM, },
    { "switch", SWITCHSYM, },
    { "while", WHILESYM, },

    { "#asm", ASMCNTL, },
    { "#define", DEFINECNTL, },
    { "#elif", ELIFCNTL, },
    { "#else", ELSECNTL, },
    { "#endasm", ENDASMCNTL, },
    { "#endif", ENDIFCNTL, },
    { "#if", IFCNTL, },
    { "#ifdef", IFDEFCNTL, },
    { "#include", INCLUDECNTL, },
    { "#ifndef", IFNDEFCNTL, },
    { "#line", LINECNTL, },
    { "#undef", UNDEFCNTL, },
    { "#warning", WARNINGCNTL, },
    { "#error", ERRORCNTL, },

    { "defined", DEFINEDSYM, },	/* should be deactivated except in #if's */
};

PRIVATE struct typedatastruct scaltypes[NSCALTYPES] =
{
    { "void", TRUE, 0, 0, &vtype, },	/* first - addkeyword uses vtype */
    { "char", TRUE, CHAR, 1, &ctype, },
    { "char", FALSE, UNSIGNED | CHAR, 1, &uctype, },
    { "char", FALSE, CHAR, 1, &sctype, },
    { "short", TRUE, SHORT, 2, &stype, },
    { "short", FALSE, UNSIGNED | SHORT, 2, &ustype, },
    { "int", TRUE, INT, 2, &itype, },
    { "int", FALSE, UNSIGNED | INT, 2, &uitype, },
    { "long", TRUE, LONG | DLONG, 4, &ltype, },
    { "long", FALSE, UNSIGNED | LONG | DLONG, 4, &ultype, },
#ifndef NOFLOAT
    { "float", TRUE, FLOAT, 4, &fltype, },
    { "double", TRUE, DOUBLE, 8, &dtype, },
#endif
};

FORWARD struct symstruct *addkeyword P((char *name, sym_pt code));
FORWARD void heapcorrupterror P((void));

PUBLIC struct symstruct *addglb(name, type)
char *name;
struct typestruct *type;
{
    struct symstruct **hashptr;
    struct symstruct *oldsymptr = NULL; /* for -Wall */
    register struct symstruct *symptr;

    hashptr = gethashptr(name);
    symptr = *hashptr;
    while (symptr != NULL)
    {
	oldsymptr = symptr;
	symptr = symptr->next;
    }
    symptr = qmalloc(sizeof (struct symstruct) + strlen(name));
#ifdef TS
++ts_n_global;
ts_size_global += sizeof (struct symstruct) + strlen(name);
#endif
    addsym(name, type, symptr);
    symptr->storage = GLOBAL;
    symptr->level = GLBLEVEL;
    if (*hashptr == NULL)
    {
	*hashptr = symptr;
	symptr->prev = hashptr;
    }
    else
    {
	oldsymptr->next = symptr;
	symptr->prev = &oldsymptr->next;
    }
    return symptr;
}

PRIVATE struct symstruct *addkeyword(name, code)
char *name;
sym_pt code;
{
    struct symstruct *symptr;

    (symptr = addglb(name, vtype))->flags = KEYWORD;
    symptr->offset.offsym = code;
    return symptr;
}

PUBLIC struct symstruct *addloc(name, type)
char *name;
struct typestruct *type;
{
    struct symstruct **hashptr;
    register struct symstruct *symptr;

    hashptr = gethashptr(name);
    symptr = *hashptr;
    while (symptr != NULL)
	symptr = symptr->next;
    symptr = locptr;
    locptr = (struct symstruct *) align(&symptr->name.namea[strlen(name) + 1]);
    if (locptr >= &locsyms[MAXLOCAL])
	limiterror("too many local symbols (101)");
    addsym(name, type, symptr);
    if (type->constructor == FUNCTION)
	symptr->storage = GLOBAL;
    else
	symptr->storage = LOCAL;
    symptr->level = level;
    if (*hashptr != NULL)
    {
	symptr->next = *hashptr;
	symptr->next->prev = &symptr->next;
    }
    *hashptr = symptr;
    symptr->prev = hashptr;
    return symptr;
}

PUBLIC struct symstruct *addlorg(name, type)
char *name;
struct typestruct *type;
{
    if (level != GLBLEVEL)
	return addloc(name, type);
    return addglb(name, type);
}

PUBLIC void addsym(name, type, symptr)
char *name;
struct typestruct *type;
register struct symstruct *symptr;
{
    if (type->constructor & (ARRAY | FUNCTION))
	symptr->indcount = 0;
    else
	symptr->indcount = 1;
    symptr->flags = 0;
    symptr->next = NULL;
    symptr->type = type;
    symptr->offset.offi = 0;
    strcpy(symptr->name.namea, name);
}

PUBLIC struct symstruct *constsym(longconst)
value_t longconst;
{
    register struct symstruct *symptr;

    symptr = exprsym(&constemplate);
    symptr->offset.offv = longconst;
    return symptr;
}

PUBLIC void delsym(symptr)
register struct symstruct *symptr;
{
    if ((*(symptr->prev) = symptr->next) != NULL)
	symptr->next->prev = symptr->prev;
}

/* dumpglbs() - define locations of globals and reserve space for them */

PUBLIC void dumpglbs()
{
    int i;
    register struct symstruct *symptr;
    register struct typestruct *type;

#ifdef TS
extern char *brksize;
struct ts
{
    char *what;
    uvalue_t *where;
};
static struct ts ts[] =
{
    "ts_n_newtypelist                  ", &ts_n_newtypelist,
    "ts_s_newtypelist                  ", &ts_s_newtypelist,
    "ts_n_filename_term                ", &ts_n_filename_term,
    "ts_s_filename_term                ", &ts_s_filename_term,
    "ts_n_filename                     ", &ts_n_filename,
    "ts_s_filename                     ", &ts_s_filename,
    "ts_s_filename_tot                 ", &ts_s_filename_tot,
    "ts_n_pathname                     ", &ts_n_pathname,
    "ts_s_pathname                     ", &ts_s_pathname,
    "ts_s_pathname_tot                 ", &ts_s_pathname_tot,
    "ts_n_inputbuf                     ", &ts_n_inputbuf,
    "ts_s_inputbuf                     ", &ts_s_inputbuf,
    "ts_s_inputbuf_tot                 ", &ts_s_inputbuf_tot,
    "ts_n_includelist                  ", &ts_n_includelist,
    "ts_s_includelist                  ", &ts_s_includelist,
    "ts_s_outputbuf                    ", &ts_s_outputbuf,
    "ts_n_macstring_ident              ", &ts_n_macstring_ident,
    "ts_n_macstring_ordinary           ", &ts_n_macstring_ordinary,
    "ts_n_macstring_param              ", &ts_n_macstring_param,
    "ts_n_macstring_quoted             ", &ts_n_macstring_quoted,
    "ts_n_macstring_term               ", &ts_n_macstring_term,
    "ts_s_macstring                    ", &ts_s_macstring,
    "ts_n_defines                      ", &ts_n_defines,
    "ts_s_defines                      ", &ts_s_defines,
    "ts_n_macparam                     ", &ts_n_macparam,
    "ts_s_macparam                     ", &ts_s_macparam,
    "ts_s_macparam_tot                 ", &ts_s_macparam_tot,
    "ts_n_macparam_string_ordinary     ", &ts_n_macparam_string_ordinary,
    "ts_n_macparam_string_quoted       ", &ts_n_macparam_string_quoted,
    "ts_n_macparam_string_term         ", &ts_n_macparam_string_term,
    "ts_s_macparam_string              ", &ts_s_macparam_string,
    "ts_s_macparam_string_tot          ", &ts_s_macparam_string_tot,
    "ts_s_macparam_string_alloced      ", &ts_s_macparam_string_alloced,
    "ts_s_macparam_string_alloced_tot  ", &ts_s_macparam_string_alloced_tot,
    "ts_s_fakeline                     ", &ts_s_fakeline,
    "ts_s_fakeline_tot                 ", &ts_s_fakeline_tot,
    "ts_n_string                       ", &ts_n_string,
    "ts_n_case                         ", &ts_n_case,
    "ts_n_case_realloc                 ", &ts_n_case_realloc,
    "ts_s_case                         ", &ts_s_case,
    "ts_s_case_tot                     ", &ts_s_case_tot,
    "ts_n_structname                   ", &ts_n_structname,
    "ts_s_structname                   ", &ts_s_structname,
    "ts_n_type                         ", &ts_n_type,
    "ts_s_type                         ", &ts_s_type,
    "ts_n_global                       ", &ts_n_global,
    "ts_size_global                    ", &ts_size_global,
    "ts_n_holdstr                      ", &ts_n_holdstr,
    "ts_size_holdstr                   ", &ts_size_holdstr,
    "ts_n_growobj                      ", &ts_n_growobj,
    "ts_size_growobj_wasted            ", &ts_size_growobj_wasted,
    "ts_n_growheap                     ", &ts_n_growheap,
    "ts_s_growheap                     ", &ts_s_growheap,
};
struct ts *tp;

for (tp = &ts[0]; tp < &ts[sizeof ts / sizeof ts[0]]; ++tp)
{
    comment();
    outstr(tp->what);
    outstr("");
    outuvalue(*tp->where);
    outnl();
}
comment();
outstr("brksize                           ");
outhex((uvalue_t) brksize);
outnl();
#endif

#ifndef DIRECTPAGE
    bssseg();
#endif
    for (i = 0; i < HASHTABSIZE; ++i)
	for (symptr = hashtab[i]; symptr != NULL; symptr = symptr->next)
	    if (symptr->storage == GLOBAL && !(symptr->flags & EXTERNAL)
		&& *symptr->name.namea >= 'A' && symptr->flags <= MAXDUMPFLAG
		/* Don't access type unless flags <= MAXDUMPFLAG, because
		 * type is punned to a symbol pointer for the label chain
		 * and might be null.
		 */
		&& symptr->type->constructor != FUNCTION)
	    {
#ifdef DIRECTPAGE
		if (symptr->flags & DIRECTPAGE)
		    dpseg();
		else
		    dseg();
#endif
		type = symptr->type;
		if (symptr->flags & STATIC)
		    lcommon(symptr->name.namea);
		else
		    common(symptr->name.namea);
		outnhex(type->typesize);
	    }
    if (printf_fp)
	globl("__xfpcvt");
    if (scanf_fp)
	globl("__xfptvc");
}

/* dumplocs() - define offsets of current locals */

PUBLIC void dumplocs()
{
    register struct symstruct *symptr;
    int i;

#ifdef ASM_USES_CALLEE_REGS
    if (framep && optimise && !callersaves) {
       regfuse |= callee1mask;
       outnstr("! Assuming #asm uses all callee saves registers");
    }
#endif

    for (i = 0; i < HASHTABSIZE; ++i)
	for (symptr = hashtab[i]; symptr != NULL; symptr = symptr->next)
	    if (symptr->storage == LOCAL)
		set(symptr->name.namea, symptr->offset.offi - sp);
}

#ifdef HOLDSTRINGS

/* dumpstrings() - dump held-up strings */

PUBLIC void dumpstrings()
{
    struct string *stringp;

    dseg();
    for (stringp = laststring; stringp != NULL; stringp = stringp->snext)
    {
	outnlabel(stringp->slabel);
	defstr(stringp->sptr, stringp->stop, TRUE);
	dataoffset += stringp->stop - stringp->sptr + 1;
    }
}

#endif

PUBLIC struct symstruct *exprsym(symptr)
struct symstruct *symptr;
{
    register struct symstruct *newsymptr;

    newsymptr = exprptr++;
    if (exprptr >= &exprsyms[MAXEXPR])
#if MAXEXPR == 500
	limiterror("expression too complex (501 symbols)");
#else
	limiterror("expression too complex (MAXEXPR)");
#endif
    *newsymptr = *symptr;
    newsymptr->level = EXPRLEVEL;
    newsymptr->name.namep = symptr->name.namea;
    return newsymptr;
}

PUBLIC struct symstruct *findlorg(name)
char *name;
{
    struct symstruct *symptr;

    symptr = *gethashptr(name);
    while (symptr != NULL && (strcmp(symptr->name.namea, name) != 0 ||
			      symptr->flags == STRUCTNAME))
	symptr = symptr->next;
    return symptr;
}

PUBLIC struct symstruct *findstruct(name)
char *name;
{
    struct symstruct *symptr;

    symptr = *gethashptr(name);
    while (symptr != NULL && (symptr->flags != STRUCTNAME ||
			      strcmp(symptr->name.namea, name) != 0))
	symptr = symptr->next;
    return symptr;
}

/* convert name to a hash table ptr */

PUBLIC struct symstruct **gethashptr(sname)
char *sname;
{
    register int hashval;
    register char *rsname;

    hashval = 0;
    rsname = sname;
#if 1
    while (*rsname)
	hashval = hashval * 2 + *rsname++;
#else
    hashval = rsname[0];
    if (rsname[1] != 0)
	hashval = (rsname[2] << 2) ^ rsname[1] ^ (rsname[0] << 1);
    else
	hashval = rsname[0];
#endif
    return hashtab + ((hashval * GOLDEN) & (HASHTABSIZE - 1));
}

PRIVATE void heapcorrupterror()
{
    outofmemoryerror(" (heap corrupt - stack overflow?)");
}

#ifdef HOLDSTRINGS

/* hold string for dumping at end, to avoid mixing it with other data */

PUBLIC label_no holdstr(sptr, stop)
char *sptr;
char *stop;
{
    register struct string *stringp;

    stringp = qmalloc(sizeof *stringp);
#ifdef TS
++ts_n_holdstr;
ts_size_holdstr += sizeof *stringp;
#endif
    stringp->snext = laststring;
    stringp->sptr = sptr;
    stringp->stop = stop;
    laststring = stringp;
    return stringp->slabel = getlabel();
}

#endif /* HOLDSTRINGS */

PUBLIC void newlevel()
{
    if (*(unsigned *) chartop != MARKER)
	heapcorrupterror();
    if (level >= MAXLEVEL)
	limiterror("compound statements nested too deeply (126 levels)");
    else
	++level;
}

PUBLIC void oldlevel()
{
    register struct symstruct *symptr;

    if (*(unsigned *) chartop != MARKER)
	heapcorrupterror();
    if (level == 0)
	bugerror("not in a compound statement");
    else
	--level;
    for (symptr = &locsyms[0]; symptr < locptr;
	 symptr = (struct symstruct *)
		  align(&symptr->name.namea[strlen(symptr->name.namea) + 1]))
	if (symptr->level > level)
	    delsym(symptr);
}

PUBLIC void ourfree(ptr)
void *ptr;
{
    free(ptr);
}

PUBLIC void *ourmalloc(nbytes)
unsigned nbytes;
{
    void *ptr;

    if ((ptr = malloc(nbytes)) == NULL)
	outofmemoryerror("");
    return ptr;
}

PUBLIC void outofmemoryerror(message)
char *message;
{
    error2error("compiler out of memory", message);

#ifdef __AS386_16__
    {
	unsigned size;
	char *ptr;

	for (size = 0x1000; size != 0; --size)
	    if ((ptr = malloc(size)) != NULL)
	    {
		outstr("found free memory at ");
		outuvalue((uvalue_t) ptr);
		outstr(" size ");
		outuvalue((uvalue_t) size);
		outnl();
	    }
    }
#endif

    finishup();
}

PUBLIC void *growobject(object, extra)
void *object;
unsigned extra;
{
    /* size_t */ unsigned oblength;

    /* It would be nice to realloc the previous memory if it can be left in
     * the same place. Since a realloc that moves the memory can almost be
     * relied on to mess up pointers before *obptr, and since malloc(extra)
     * would be unlikely to produce memory adjoining chartop, it is not
     * clear how to do this.
     */
#ifdef TS
ts_size_growobj_wasted += chartop - (char *) object;
#endif
    oblength = (/* size_t */ unsigned) (charptr - (char *) object);
    growheap(oblength + extra);
#ifdef TS
++ts_n_growobj;
#endif
    memcpy(charptr, object, oblength);
    object = charptr;
    charptr += oblength;
    return object;
}

#define ALLOC_UNIT ((unsigned) 0x400)
#ifdef S_ALIGNMENT
#define ALLOC_OVERHEAD (S_ALIGNMENT - 1 + sizeof (unsigned))
#else
#define ALLOC_OVERHEAD (sizeof (unsigned))
#endif

PUBLIC void growheap(size)
unsigned size;
{
    register char *newptr;

    if ((newptr = malloc(size += ALLOC_UNIT + ALLOC_OVERHEAD)) == NULL
	&& (newptr = malloc(size -= ALLOC_UNIT)) == NULL)
	outofmemoryerror("");
#ifdef TS
++ts_n_growheap;
ts_s_growheap += size;
#endif
    charptr = newptr;
    newptr = (char *) align(newptr + size - ALLOC_OVERHEAD);
    chartop = newptr;
    char1top = newptr - 1;
    char3top = newptr - 3;
    *(unsigned *) newptr = MARKER;
}

PUBLIC void *qmalloc(size)
unsigned size;
{
    register char *ptr;

    if ((charptr = (char *) align(charptr)) + size > chartop)
	growheap(size);
    ptr = charptr;
    charptr += size;
    return ptr;
}

PUBLIC void swapsym(sym1, sym2)
struct symstruct *sym1;
struct symstruct *sym2;
{
    struct symstruct swaptemp;

    swaptemp = *sym1;
    *sym1 = *sym2;
    *sym2 = swaptemp;
}

PUBLIC void syminit()
{
    struct keywordstruct *kwptr;
    struct typedatastruct *tdptr;
    struct typestruct *type;

    exprptr = exprsyms;
    locptr = locsyms;
    for (tdptr = scaltypes; tdptr < scaltypes + NSCALTYPES; ++tdptr)
    {
	(*tdptr->tdtypeptr = type = newtype())->scalar = tdptr->tdscalar;
	type->alignmask = ~((type->typesize = tdptr->tdsize) - 1) |
	    alignmask;
	type->tname = tdptr->tdname;
	if (tdptr->tdkeyscalar)
	    addkeyword(tdptr->tdname, TYPEDECL)->type = type;
    }
    for (kwptr = keywords; kwptr < keywords + NKEYWORDS; ++kwptr)
	addkeyword(kwptr->kwname, kwptr->kwcode);
    constemplate.type = itype;
}
