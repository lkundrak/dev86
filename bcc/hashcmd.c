
/* decode remaining preprocessor lines, minimal version. */

/* Copyright (C) GPL V2, derived from preproc.c by RDB. */

#include "bcc.h"
#include "input.h"
#include "os.h"
#include "output.h"
#include "parse.h"
#include "sc.h"
#include "scan.h"
#include "table.h"
#include "type.h"

#ifndef BUILTIN_CPP
FORWARD void control P((void));
FORWARD void asmcontrol P((void));
FORWARD void warningcntl P((void));
FORWARD void errorcntl P((void));

/* docontrol() - process control statement, #line and #asm only. */
PUBLIC void docontrol()
{
    control();
    skipline();
    return;
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
    if (blanksident())
       strcat(sname, gsname);
    if (sname[1] == 0 && ch == EOL)
	return;
    if (SYMOFCHAR(ch) == INTCONST)
	{ linecontol(); return; }
    if ((symptr = findlorg(sname)) == NULL)
    {
	error(" bad control");
	return;
    }
    ctlcase = symptr->offset.offsym;

    switch (ctlcase)
    {
    case ASMCNTL:
        if (asmmode) 
	   error(" bad control");
	else
	   asmcontrol();
	break;
    case ENDASMCNTL:
        if (!asmmode) 
           error(" bad control");
	asmmode = FALSE;
	break;
    case LINECNTL:
	{ linecontol(); break; }
    case WARNINGCNTL:
	warningcntl();
	break;
    case ERRORCNTL:
	errorcntl();
	break;
    default:
        error(" bad control");
	break;
    }
}

/* asmcontrol() - process #asm */

PRIVATE void asmcontrol()
{
#ifdef ASM_BARE
    char treasure;		/* to save at least one leading blank */
#endif

    asmmode = TRUE;
    if (expect_statement)
       return;

    outnstr("!BCC_ASM");
    dumplocs();
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
    outnstr("!BCC_ENDASM");
}

/* warningcntl() - process #warning */

PRIVATE void warningcntl()
{
    char estr[256], *ep = estr;

    *ep++ = '%'; *ep++ = 'w'; 
    while( ch != EOL ) {
       if (ep < estr+sizeof(estr)-2 )
	  *ep++ = ch;
       gch1();
    }
    *ep = 0;
    error(estr);
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

    error(estr);
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
#endif
