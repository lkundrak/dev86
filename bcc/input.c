/* input.c - input for bcc */

/* Copyright (C) 1992 Bruce Evans */

#define ARBITRARY_BACKSLASH_NEWLINES_NOT
#define INSERT_BACKSLASH_NEWLINES_NOT

#include "bcc.h"
#include "gencode.h"
#include "output.h"
#include "os.h"
#include "sc.h"
#include "scan.h"
#include "table.h"

#undef EXTERN
#define EXTERN
#include "input.h"

#define INBUFSIZE 2048
#define NO_EOFHACK

struct fbufstruct		/* file buffer structure */
{
    struct fcbstruct fcb;	/* status after opening an include sub-file */
    char *fname;		/* file name */
    bool_t fname_malloced;	/* nonzero if fname was malloced */
    char pushback[3];		/* allows pushback to 2 before start of fbuf
				 * XXX 3 chars before?
				 * XXX should do everything in fbuf */
    char fbuf[INBUFSIZE + 1];	/* buffer to read into */
};

#ifdef BUILTIN_CPP
struct inclist			/* list of include file directories */
{
    char *incdirname;
    struct inclist *incnext;
};

PRIVATE char filemacro[] = "__FILE__";
PRIVATE struct inclist incabsolute;	/* dummy list for absolute names */
				/* depends on zero (NULL) init */
PRIVATE struct inclist incfirst;
				/* begin include searches here for "" */
				/* at next in list for <> */
				/* -I directories are put in list */
				/* between first and last */
PRIVATE struct inclist inclast =
{
#ifdef DEFAULT_INCLUDE_DIR
    DEFAULT_INCLUDE_DIR,
#endif
    NULL,
};
PRIVATE fastin_t inclevel;	/* nest level of include files */
				/* depends on zero init */
#endif
PRIVATE struct fbufstruct *inputbuf;	/* current input file buffer */
					/* its fcb only to date in includes */
					/* depends on zero (NULL) init */
PRIVATE bool_t suppress_line_numbers;

#ifdef ARBITRARY_BACKSLASH_NEWLINES
FORWARD void backslash P((void));
#endif
#ifdef BUILTIN_CPP
FORWARD void definefile P((char *fname));
FORWARD void leaveinclude P((void));
#endif
FORWARD void inputinit P((char *fname, fd_t fd));
FORWARD void usage P((void));

#ifdef ARBITRARY_BACKSLASH_NEWLINES
PRIVATE void backslash()
{
    static unsigned nbackslash;

    if (nbackslash != 0)
	--nbackslash;
more:
    ++nbackslash;
    while (*(lineptr + 1) == '\\')
    {
	++nbackslash;
	++lineptr;
    }
    if (*(lineptr + 1) != EOL)
    {
	if (--nbackslash != 0)
	    *--lineptr = '\\';	/* force look at current backslash again */
	return;
    }
    ch = *++lineptr;
more1:
    if (!eofile && lineptr >= input.limit)
	skipeol();
    if (ch == EOL)
    {
	--nbackslash;
	if (eofile)
	    eofin("backslash-newline");
	else
	{
	    skipeol();
	    if (ch == EOL && !eofile && lineptr >= input.limit)
		skipeol();
#ifdef COEOL /* XXX - this should go through specialchar() */
	    if (ch == COEOL)
	    {
		ch = *++lineptr;
		if (ch == EOL && !eofile && lineptr >= input.limit)
		    skipeol();
	    }
#endif
	}
	if (ch == '\\')
	    goto more;
	if (nbackslash != 0 && ch == EOL && !eofile)
	    goto more1;
	if (nbackslash != 0)
	{
	    ch = *--lineptr = '\\';
	    if (--nbackslash != 0)
		ch = *--lineptr = '\\';
	}
	return;
    }
    if (ch == '\\')
	goto more;
    if (ch == EOL && !eofile)
	goto more1;
    ch = *--lineptr = '\\';	/* pushback */
    if (--nbackslash != 0)
	ch = *--lineptr = '\\';
}
#endif

PUBLIC void closein()
{
#ifdef FAKE_INBUFSIZE_1
    fclose(input.fp);
#else
    close(input.fd);
#endif
#ifdef BUILTIN_CPP
    while (inclevel != 0)
	leaveinclude();
#endif
}

#ifdef BUILTIN_CPP
PRIVATE void definefile(fname)
char *fname;
{
    char *def;

    def = ourmalloc(sizeof filemacro - 1 + 2 + strlen(fname) + 1 + 1);
    strcpy(def, filemacro);
    strcat(def, "=\"");
    strcat(def, fname);
    strcat(def, "\"");
    definestring(def);
    ourfree(def);
}
#endif

PUBLIC void errorloc()
{
    register struct fbufstruct *infbuf;

    if ((infbuf = inputbuf) == NULL)
	return;
    outstr(infbuf->fname);
    outbyte(':');
    if (eofile)
	outstr("eof");
    else
    {
	outudec(input.linenumber);
	outbyte('.');
#ifdef BUILTIN_CPP
	if (maclevel == 0)
	    outudec((unsigned) (lineptr - inputbuf->fbuf) - input.lineoffset);
	else
	{
	    outudec((unsigned) (savedlineptr() - inputbuf->fbuf)
		    - input.lineoffset);
	    outstr(" (macro level ");
	    outudec((unsigned) maclevel);
	    outbyte(')');
	}
#else
	outudec((unsigned) (lineptr - inputbuf->fbuf) - input.lineoffset);
#endif
    }
    infbuf->fcb.includer = input.includer;
    while ((infbuf = infbuf->fcb.includer) != NULL)
    {
	outstr(" (from ");
	outstr(infbuf->fname);
	outbyte(':');
	outudec(infbuf->fcb.linenumber);
	outbyte(')');
    }
    outstr(": ");
}

/* gch1() - get next char, advance ptr (only works on current line) */

PUBLIC void gch1()
{
    if (SYMOFCHAR(ch = *++lineptr) != SPECIALCHAR)
	return;
    specialchar();
}

#ifdef BUILTIN_CPP
/* process #include */

PUBLIC void include()
{
    char *dirnameptr;
    char *dirnamend;
    unsigned dirnamelen;
    fd_t fd;
    char *fnameptr;
    char *fullnameptr;
    struct inclist *incptr;
    char terminator;

    while (blanksident())
    {
	if ((gsymptr = findlorg(gsname)) == NULL ||
	    gsymptr->flags != DEFINITION)
	    break;
	entermac();
    }
    if ((terminator = ch) == '<')
	terminator = '>';
    else if (terminator != '"')
    {
	error("bad file name");
	return;
    }
    gch1();
    fnameptr = charptr;
    while (TRUE)
    {
	if (ch == EOL)		/* no escapes in file names */
	    break;
	if (ch == terminator)
	{
	    gch1();
	    blanks();
	    break;
	}
	if (charptr >= chartop)
	    fnameptr = growobject(fnameptr, 1);
#ifdef TS
++ts_n_filename;
++ts_s_filename;
++ts_s_filename_tot;
#endif
	*charptr++ = ch;
	gch1();
    }
    if (charptr >= chartop)
	fnameptr = growobject(fnameptr, 1);
#ifdef TS
++ts_n_filename_term;
++ts_s_filename_term;
++ts_s_filename_tot;
#endif
    *charptr++ = 0;
    dirnamend = NULL;
    if (isabspath(fnameptr, &ch))
	incptr = &incabsolute;
    else
    {
	incptr = &incfirst;
	if (terminator == '>')
	    incptr = incptr->incnext;
	else
	{
	    dirnameptr = inputbuf->fname;
	    if ((dirnamend = strrchr(dirnameptr, DIRCHAR)) == NULL)
		incptr->incdirname = NULL;
	    else
	    {
		*dirnamend = 0;
		incptr->incdirname = dirnameptr;
	    }
	}
    }
    do
    {
	if (incptr->incdirname == NULL)
	{
	    fullnameptr = ourmalloc(strlen(fnameptr) + 1);
#ifdef TS
++ts_n_pathname;
ts_s_pathname += strlen(fnameptr) + 1;
ts_s_pathname_tot += strlen(fnameptr) + 1;
#endif
	    strcpy(fullnameptr, fnameptr);
	}
	else
	{
	    dirnamelen = strlen(incptr->incdirname);
	    fullnameptr = ourmalloc(dirnamelen + (int) (charptr - fnameptr)
				    + 2);
				/* 2 extra for null and maybe DIRCHAR */
#ifdef TS
++ts_n_pathname;
ts_s_pathname += dirnamelen + (charptr - fnameptr) + 2;
ts_s_pathname_tot += dirnamelen + (charptr - fnameptr) + 2;
#endif
	    dirnameptr = fullnameptr + dirnamelen;
	    strcpy(fullnameptr, incptr->incdirname);
	    if (*fullnameptr != 0 && *(dirnameptr - 1) != DIRCHAR)
		strcat(fullnameptr, DIRSTRING);
	    strcat(fullnameptr, fnameptr);
	    if (dirnamend != NULL)
	    {
		*dirnamend = DIRCHAR;
		dirnamend = NULL;
	    }
	}
	fd = open(fullnameptr, 0);
	if (fd >= 0)
	{
#ifdef TS
ts_s_filename_tot -= charptr - fnameptr;
#endif
	    charptr = fnameptr;
	    input.lineptr = lineptr;
	    inputbuf->fcb = input;
	    ++inclevel;		/* XXX - will run out of memory before overflow */
	    inputinit(fullnameptr, fd);
	    inputbuf->fname_malloced = TRUE;
	    return;
	}
#ifdef TS
ts_s_pathname_tot -= strlen(fullnameptr) + 1;
#endif
	ourfree(fullnameptr);
    }
    while ((incptr = incptr->incnext) != NULL);

    fullnameptr = ourmalloc(strlen(fnameptr) + 40);
    strcpy(fullnameptr, "cannot find include file ");
    strcat(fullnameptr, fnameptr);
    error(fullnameptr);
    ourfree(fullnameptr);

#ifdef TS
ts_s_filename_tot -= charptr - fnameptr;
#endif
    charptr = fnameptr;
}
#endif

/* initialise current input file */

PRIVATE void inputinit(fname, fd)
char *fname;
fd_t fd;
{
    register struct fbufstruct *newinputbuf;

    /* don't allocate after saying input.includer = inputbuf (to save a reg)
     * or an error in the alloc would cycle trying to print the include list
     */
    newinputbuf = (struct fbufstruct *) ourmalloc(sizeof *inputbuf);
#ifdef TS
++ts_n_inputbuf;
ts_s_inputbuf += sizeof *inputbuf;
ts_s_inputbuf_tot += sizeof *inputbuf;
#endif
    input.fd = fd;
#ifdef FAKE_INBUFSIZE_1
    input.fp = fdopen(fd, "r");
#endif
    input.linenumber = 0;
    input.lineoffset = 0;
    input.includer = inputbuf;
    inputbuf = newinputbuf;
    newinputbuf->fname = fname;
    newinputbuf->fname_malloced = FALSE;
#ifdef BUILTIN_CPP
    undefinestring(filemacro);
    definefile(fname);
    if (orig_cppmode && !suppress_line_numbers)
	outcpplinenumber(1, fname, input.includer == NULL ? "" : " 1");
#endif
    *(input.limit = newinputbuf->fbuf) = EOL;

    /* dummy line so processing can start with skipline() */
    ch = *(lineptr = newinputbuf->fbuf - 1) = EOL;
}

PUBLIC void linecontol()
{
   char linename[256];
   char * ptr;
   int i=0;

   blanks();
   input.linenumber = atoi(lineptr)-1;
   while( SYMOFCHAR(ch) == INTCONST ) gch1();
   blanks();
   if( ch != '"' ) return;
   for(ptr=lineptr+1; *ptr
                   && *ptr != EOL
		   && *ptr != '"'
		   && i<sizeof(linename)-1; i++, ptr++)
      linename[i] = *ptr;
   linename[i] = '\0';

    if (inputbuf->fname_malloced)
{
#ifdef TS
ts_s_pathname_tot -= strlen(inputbuf->fname) + 1;
#endif
	ourfree(inputbuf->fname);
}
    inputbuf->fname_malloced = TRUE;
    ptr = ourmalloc(strlen(linename)+1);
    strcpy(ptr, linename);
    inputbuf->fname = ptr;

    ptr=lineptr;
#ifdef BUILTIN_CPP
    undefinestring(filemacro);
    definefile(inputbuf->fname);
#endif
    ch = *(lineptr = ptr);
}

#ifdef BUILTIN_CPP
/* switch from include file to file which included it */

PRIVATE void leaveinclude()
{
    --inclevel;
    if (inputbuf->fname_malloced)
{
#ifdef TS
ts_s_pathname_tot -= strlen(inputbuf->fname) + 1;
#endif
	ourfree(inputbuf->fname);
}
#ifdef TS
ts_s_inputbuf_tot -= sizeof *inputbuf;
#endif
    ourfree((char *) inputbuf);
#ifndef NO_EOFHACK
    if(input.fd>=0)
#endif
       close(input.fd);
#ifdef FAKE_INBUFSIZE_1
    fclose(input.fp);
#endif
    inputbuf = input.includer;
    input = inputbuf->fcb;
    undefinestring(filemacro);
    definefile(inputbuf->fname);
    ch = *(lineptr = input.lineptr);
    skipline();
    if (orig_cppmode && !suppress_line_numbers)
	outcpplinenumber(input.linenumber, inputbuf->fname, " 2");
}
#endif

/* open input and output files and get options */

PUBLIC void openio(argc, argv)
int argc;
char *argv[];
{
    register char *arg;
    int argn;
    fd_t fd;
    char *fname;
#ifdef BUILTIN_CPP
    struct inclist *incnew;
    struct inclist *incptr;
#endif
    bool_t flag[128];

#if 0
    lineptr = "\n";		/* empty line in case error message */
#endif
    fd = 0;			/* standard input */
    memset(flag, 0, sizeof flag);
#ifdef I80386
    flag['3'] = sizeof (int) >= 4;
#endif
    fname = "stdin";
#ifdef BUILTIN_CPP
    (incptr = &incfirst)->incnext = &inclast;
#endif
    initout();
    for (argn = 1; argn < argc; ++argn)
    {
	arg = argv[argn];
	if (*arg != '-')
	{
	    if (fd != 0)
		fatalerror("more than one input file");
	    fname = arg;
	    if ((fd = open(arg, 0)) < 0)
		fatalerror("cannot open input");
	}
	else
	    switch (arg[1])
	    {
#ifdef I8088
	    case '0':		/* generate 16-bit code */
#endif
#ifdef I80386
	    case '3':		/* generate 32-bit code */
#endif
	    case 'c':		/* caller saves */
#ifdef DBNODE
	    case 'd':		/* print debugging information in asm output */
#endif
#ifdef BUILTIN_CPP
	    case 'E':		/* acting as cpp */
#endif
	    case 'f':		/* pass first argument in register */
#ifdef DYNAMIC_LONG_ORDER
	    case 'l':		/* long big-endian */
#endif
	    case 'P':		/* if acting as cpp, suppress line numbers */
#ifdef POSINDEPENDENT
	    case 'p':		/* generate almost-position-independent code */
#endif
	    case 't':		/* print source code in asm output */
	    case 'w':		/* watch location counter */
	    case 'O':		/* Optimisation. */
		if (arg[2] == 0)
		    flag[(int)arg[1]] = TRUE;
		else if (arg[2] == '-' && arg[3] == 0)
		    flag[(int)arg[1]] = FALSE;
		else
		    usage();
		if (arg[1] == '0')	/* flag 0 is negative logic flag 3 */
		    flag['3'] = TRUE + FALSE - flag['0'];
		break;
#ifdef BUILTIN_CPP
	    case 'D':
		definestring(arg + 2);
		break;
	    case 'I':
		(incnew = (struct inclist *) ourmalloc(sizeof *incnew))
			  ->incdirname = arg + 2;
#ifdef TS
++ts_n_includelist;
ts_s_includelist += sizeof *incnew;
#endif
		incnew->incnext = incptr->incnext;
		incptr = incptr->incnext = incnew;
		break;
	    case 'U':
		undefinestring(arg + 2);
		break;
#endif
	    case 'o':
		if (arg[2] != 0 || ++argn >= argc)
		    usage();
		openout(argv[argn]);
		break;
	    default:
		usage();
		break;
	    }
    }
#ifdef BUILTIN_CPP
#ifdef I8088
#ifdef I80386
    if (flag['3'])
    {
	i386_32 = TRUE;
	definestring("__AS386_32__");
	definestring("__i386__");
    }
    else
#endif
    {
	definestring("__AS386_16__");
	definestring("__8086__");
    }
#endif
#ifdef MC6809
    definestring("__AS09__");
#endif
    if (flag['c'])
    {
	callersaves = TRUE;
	definestring("__CALLER_SAVES__");
    }
#ifdef DBNODE
    dbnodeon = flag['d'];
#endif
    orig_cppmode = cppmode = flag['E'];
    if (flag['f'])
    {
	arg1inreg = TRUE;
#ifdef I8088
	definestring("__FIRST_ARG_IN_AX__");
#endif
#ifdef MC6808
	definestring("__FIRST_ARG_IN_X__");
#endif
    }
    arg1op = arg1inreg ? ROOTLISTOP : LISTOP;
#ifdef DYNAMIC_LONG_ORDER
    if (flag['l'])
    {
	long_big_endian = TRUE;
	definestring("__LONG_BIG_ENDIAN__");
    }
#endif
    suppress_line_numbers = flag['P'];
#ifdef POSINDEPENDENT
    if (flag['p'])
    {
	posindependent = TRUE;
	definestring("__POS_INDEPENDENT__");
    }
#endif
    if (flag['O'])
    {
	optimise = TRUE;
	definestring("__OPTIMISED__");
    }
#ifdef NOFLOAT
    definestring("__HAS_NO_FLOATS__");
#endif

#else /* !BUILTIN_CPP */

#ifdef I80386
    if (flag['3']) i386_32 = TRUE;
#endif
    if (flag['c']) callersaves = TRUE;
#ifdef DBNODE
    dbnodeon = flag['d'];
#endif
    if (flag['f']) arg1inreg = TRUE;
    arg1op = arg1inreg ? ROOTLISTOP : LISTOP;
#ifdef DYNAMIC_LONG_ORDER
    if (flag['l']) long_big_endian = TRUE;
#endif
    suppress_line_numbers = flag['P'];
#ifdef POSINDEPENDENT
    if (flag['p']) posindependent = TRUE;
#endif
    if (flag['O']) optimise = TRUE;

#endif
    ctext = flag['t'];
#ifdef DBNODE
    if (ctext) dbnodeon = 1;
#endif
    watchlc = flag['w'];
    setoutbufs();
    inputinit(fname, fd);
}

/* advance over EOL sentinel to new line */

PUBLIC void skipeol()
{
#ifdef FAKE_INBUFSIZE_1
    static int ich;
#endif
#ifdef INSERT_BACKSLASH_NEWLINES
    static int bs_state;
    static bool_t skip_printing_nl;
#endif
    int nread;
    debug(7, "skipeol %s:%d", inputbuf->fname, input.linenumber);

    if (eofile)
	return;
    if (lineptr < input.limit)
    {
	++input.linenumber;
	ch = *++lineptr;
	if (ctext && !asmmode)
	{
	    comment();
	    outudec(input.linenumber);
	    outbyte(' ');
	    outline(lineptr);
	}
#ifdef BUILTIN_CPP
#ifndef ASM_BARE
	if (!virtual_nl && (orig_cppmode || asmmode))
#else
	if (orig_cppmode && !asmmode)
#endif
#ifdef INSERT_BACKSLASH_NEWLINES
	    if (!skip_printing_nl)
#endif
		outbyte('\n');
#else	/* !BUILTIN_CPP */
	if (asmmode)
#ifdef INSERT_BACKSLASH_NEWLINES
	    if (!skip_printing_nl)
#endif
		outbyte('\n');
#endif

#ifdef INSERT_BACKSLASH_NEWLINES
	if (bs_state == 1 && *(lineptr - 1) == EOL)
#endif
	    input.lineoffset = (int) (lineptr - inputbuf->fbuf);
	return;
    }
    input.lineoffset -= (int) (lineptr - inputbuf->fbuf);
#ifdef FAKE_INBUFSIZE_1
    lineptr = inputbuf->fbuf;
#ifdef INSERT_BACKSLASH_NEWLINES
    switch (bs_state)
    {
case0:
    case 0:
	bs_state = nread = (ich = getc(input.fp)) == EOF ? 0 : 1;
	skip_printing_nl = FALSE;
	break;
    case 1:
	if (ich == '\\')
	    goto case0;		/* avoid chance of splitting \EOL */
	ich = '\\';
	nread = 1;
	bs_state = 2;
	++input.lineoffset;
	break;
    case 2:
	ich = EOL;
	nread = 1;
	bs_state = 0;
	skip_printing_nl = TRUE;
	++input.lineoffset;
	--input.linenumber;
	break;
    }
#else
    nread = (ich = getc(input.fp)) == EOF ? 0 : 1;
#endif
    *lineptr = ich;
#else
#ifndef NO_EOFHACK
    if(input.fd<0)
        nread=0;
    else
    {
#endif
        nread = read(input.fd, lineptr = inputbuf->fbuf, INBUFSIZE);
#ifndef NO_EOFHACK
#ifdef BUILTIN_CPP
        if( nread == 0 && inclevel > 0 )
	{
	   close(input.fd);
	   input.fd = -1;
	   memcpy(inputbuf->fbuf, "\n", 1);
	   nread = 1;
	}
#endif
    }
#endif
#endif
    if (nread < 0)
	fatalerror("input error");
    *(input.limit = lineptr + nread) = EOL;
    ch = *lineptr;
    if (nread == 0)
    {
#ifdef BUILTIN_CPP
	if (inclevel == 0)
	{
	    eofile = TRUE;
	    checknotinif();
	}
	else
	{
	    leaveinclude();
	    skipeol();
	}
#else
	eofile = TRUE;
#endif
	return;
    }
    if (ctext && !asmmode)
    {
	comment();
	outudec(input.linenumber);
	outbyte(' ');
	outline(lineptr);
    }
}

PUBLIC void specialchar()
{
#ifdef BUILTIN_CPP
    if (maclevel != 0)
    {
	if (ch == EOL)		/* it might also be backslash or COEOL */
	    leavemac();
	if (ch != EOL)
	    return;
    }
#endif
more:
#ifdef ARBITRARY_BACKSLASH_NEWLINES
    if (ch == '\\')
	backslash();
#endif
    if (!eofile && lineptr >= input.limit)
    {
	skipeol();
#ifdef ARBITRARY_BACKSLASH_NEWLINES
	if (ch == '\\')
	    backslash();
#endif
    }
#ifndef ARBITRARY_BACKSLASH_NEWLINES
    if (ch == '\\')
    {
	if (*(lineptr + 1) == EOL)
	{
	    if (lineptr + 1 >= input.limit)
	    {
		++lineptr;
		skipeol();
		ch = *--lineptr = '\\';	/* pushback */
	    }
	    if (*(lineptr + 1) == EOL)
	    {
		if (eofile)
		    eofin("backslash-newline");
		else
		{
		    ++lineptr;
		    skipeol();
		    if (SYMOFCHAR(ch) == SPECIALCHAR)
		    {
#ifdef COEOL
			if (ch != COEOL
			    || SYMOFCHAR(ch = *++lineptr) == SPECIALCHAR)
#endif
			    goto more;
		    }
		}
	    }
	}
#endif
    }
#ifdef COEOL
    if (ch == EOL && !eofile)
    {
	if (*(lineptr + 1) == EOL && lineptr + 1 >= input.limit)
	{
	    ++lineptr;
	    skipeol();		/* refill buffer */
	    ch = *--lineptr = EOL;	/* pushback */
	}
	if (*(lineptr + 1) == COEOL)
	    *++lineptr = EOL;	/* discard COEOL */
    }
#endif
}

PRIVATE void usage()
{
    fatalerror(
#ifdef BUILTIN_CPP
#  ifdef MC6809
"usage: cc1 [-cdfptw[-]] [-Ddefine] [-Iincdir] [-Uundef] [-o outfile] [infile]"
#  else 
#    ifdef I80386
"usage: cc1 [-03cdfltw[-]] [-Ddefine] [-Iincdir] [-Uundef] [-o outfile] [infile]"
#    else 
"usage: cc1 [-cdfltw[-]] [-Ddefine] [-Iincdir] [-Uundef] [-o outfile] [infile]"
#    endif
#  endif
#else 
#  ifdef I80386
"usage: cc1 [-03cdfltw[-]] [-o outfile] [infile]"
#  else 
"usage: cc1 [-cdfltw[-]] [-o outfile] [infile]"
#  endif
#endif
   );
}
