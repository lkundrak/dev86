/* ld.c - linker for Introl format (6809 C) object files 6809/8086/80386 */

/* Copyright (C) 1994 Bruce Evans */

#include "syshead.h"
#include "const.h"
#include "byteord.h"
#include "type.h"
#include "globvar.h"

#define MAX_LIBS	(NR_STDLIBS + 5)
#ifdef MC6809
#define NR_STDLIBS	1
#else
#define NR_STDLIBS	0
#endif

PUBLIC long text_base_address;	/* XXX */
PUBLIC int doscomfile = 0;

PRIVATE bool_t flag[128];
PRIVATE char *libs[MAX_LIBS] = {
#ifdef MC6809
    "/usr/local/lib/m09/",
#endif
    0
};
PRIVATE int lastlib = NR_STDLIBS;

FORWARD char *buildname P((char *pre, char *mid, char *suf));
FORWARD char *expandlib P((char *fn));

PRIVATE char *buildname(pre, mid, suf)
char *pre;
char *mid;
char *suf;
{
    char *name;

    name = ourmalloc(strlen(pre) + strlen(mid) + strlen(suf) + 1);
    strcpy(name, pre);
    strcat(name, mid);
    strcat(name, suf);
    return name;
}

PRIVATE char *expandlib(fn)
char *fn;
{
    char *path;
    int i;

    for (i = lastlib - 1; i >= 0; --i)
    {
	path = ourmalloc(strlen(libs[i]) + strlen(fn) + 1);
	strcpy(path, libs[i]);
	strcat(path, fn);
	if (access(path, R_OK) == 0)
	    return path;
	ourfree(path);
    }
    return NUL_PTR;
}

PUBLIC int main(argc, argv)
int argc;
char **argv;
{
    register char *arg;
    int argn;
    static char crtprefix[] = "crt";
    static char crtsuffix[] = ".o";
    char *infilename;
    static char libprefix[] = "lib";
    static char libsuffix[] = ".a";
    char *outfilename;
    char *tfn;

    ioinit(argv[0]);
    objinit();
    syminit();
    typeconv_init(BIG_ENDIAN, LONG_BIG_ENDIAN);
#ifndef MC6809
    flag['z'] = flag['3'] = sizeof(char *) >= 4;
#endif
    outfilename = NUL_PTR;
    for (argn = 1; argn < argc; ++argn)
    {
	arg = argv[argn];
	if (*arg != '-')
	    readsyms(arg, flag['t']);
	else
	    switch (arg[1])
	    {
	    case '0':		/* use 16-bit libraries */
	    case '3':		/* use 32-bit libraries */
	    case 'M':		/* print symbols linked */
	    case 'i':		/* separate I & D output */
	    case 'm':		/* print modules linked */
#ifdef BSD_A_OUT
	    case 'r':		/* relocatable output */
#endif
	    case 's':		/* strip symbols */
	    case 't':		/* trace modules linked */
	    case 'z':		/* unmapped zero page */
		if (arg[2] == 0)
		    flag[(int) arg[1]] = TRUE;
		else if (arg[2] == '-' && arg[3] == 0)
		    flag[(int) arg[1]] = FALSE;
		else
		    usage();
		if (arg[1] == '0')	/* flag 0 is negative logic flag 3 */
		    flag['3'] = !flag['0'];
		break;
	    case 'd':		/* Make DOS com file */
	        flag['3'] = FALSE;
	        flag['z'] = FALSE;
	        flag['0'] = TRUE;
	        flag['s'] = TRUE;
	        flag['d'] = TRUE;
		text_base_address = 0x100;
		break;
	    case 'C':		/* startfile name */
		tfn = buildname(crtprefix, arg + 2, crtsuffix);
		if ((infilename = expandlib(tfn)) == NUL_PTR)
		    fatalerror(tfn);	/* XXX - need to describe failure */
		readsyms(infilename, flag['t']);
		break;
	    case 'L':		/* library path */
		if (lastlib < MAX_LIBS)
		    libs[lastlib++] = arg + 2;
		else
		    fatalerror("too many library paths");
		break;
	    case 'O':		/* library file name */
		if ((infilename = expandlib(arg + 2)) == NUL_PTR)
		    fatalerror(arg);	/* XXX */
		readsyms(infilename, flag['t']);
		break;
	    case 'T':		/* text base address */
		if (arg[2] != 0 || ++argn >= argc)
		    usage();
		errno = 0;    
		text_base_address = strtoul(argv[argn], (char **) NUL_PTR, 16);
		if (errno != 0)
		    use_error("invalid text address");
		break;
	    case 'l':		/* library name */
		tfn = buildname(libprefix, arg + 2, libsuffix);
		if ((infilename = expandlib(tfn)) == NUL_PTR)
		    fatalerror(tfn);	/* XXX */
		readsyms(infilename, flag['t']);
		break;
	    case 'o':		/* output file name */
		if (arg[2] != 0 || ++argn >= argc || outfilename != NUL_PTR)
		    usage();
		outfilename = argv[argn];
		break;
	    default:
		usage();
	    }
    }
    doscomfile = flag['d'];
    linksyms(flag['r']);
    if (outfilename == NUL_PTR)
	outfilename = "a.out";
    writebin(outfilename, flag['i'], flag['3'], flag['s'],
	     flag['z'] & flag['3']);
    if (flag['m'])
	dumpmods();
    if (flag['M'])
	dumpsyms();
    flusherr();
    return errcount ? 1 : 0;
}
