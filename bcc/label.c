/* label.c - label handling routines for bcc */

/* Copyright (C) 1992 Bruce Evans */

#include "bcc.h"
#include "condcode.h"
#include "gencode.h"
#include "label.h"
#include "output.h"
#include "sc.h"
#include "scan.h"
#include "sizes.h"
#include "type.h"

#ifdef I8088
# define outlbranch() outop3str( "b")
# define outsbranch() outop2str( "j")
#endif
#ifdef MC6809
# define outlbranch() outop3str( "LB")
# define outsbranch() outop2str( "B")
#endif

#define MAXVISLAB 32

struct labdatstruct
{
    label_no labnum;		/* 0 if not active */
    offset_T lablc;		/* location counter for branch or label */
    char *labpatch;		/* buffer ptr for branch, NULL for label */
    ccode_t labcond;		/* condition code for branch */
};

#ifdef I8088
PRIVATE char lcondnames[][2] =	/* names of long condition codes */
{
    { 'e', 'q', }, { 'n', 'e', }, { 'r', ' ', }, { 'r', 'n', },
    { 'l', 't', }, { 'g', 'e', }, { 'l', 'e', }, { 'g', 't', },
    { 'l', 'o', }, { 'h', 'i', }, { 'l', 'o', }, { 'h', 'i' },
};
PRIVATE char scondnames[][2] =	/* names of short condition codes */
{
    { 'e', ' ', }, { 'n', 'e', }, { 'm', 'p', }, { 'n', 0, },
    { 'l', ' ', }, { 'g', 'e', }, { 'l', 'e', }, { 'g', ' ', },
    { 'b', ' ', }, { 'a', 'e', }, { 'b', 'e', }, { 'a', ' ', }, 
};
#endif

#ifdef MC6809
PRIVATE char condnames[][2] =	/* names of condition codes */
{
    { 'E', 'Q', }, { 'N', 'E', }, { 'R', 'A', }, { 'R', 'N', },
    { 'L', 'T', }, { 'G', 'E', }, { 'L', 'E', }, { 'G', 'T', },
    { 'L', 'O', }, { 'H', 'S', }, { 'L', 'S', }, { 'H', 'I', },
};
#endif

PRIVATE label_no lasthighlab = 0xFFFF+1;	/* temp & temp init so labels fixed */
				/* lint */
PRIVATE label_no lastlab;	/* bss init to 0 */
PRIVATE offset_T lc;		/* bss init to 0 */

PRIVATE struct labdatstruct vislab[MAXVISLAB];	/* bss, all labnum's init 0 */
PRIVATE smalin_t nextvislab;	/* bss init to NULL */
PRIVATE struct symstruct *namedfirst;	/* bss init to NULL */
PRIVATE struct symstruct *namedlast;	/* bss init to NULL */

FORWARD void addlabel P((ccode_pt cond, label_no label, char *patch));
FORWARD struct labdatstruct *findlabel P((label_no label));

/* add label to circular list */

PRIVATE void addlabel(cond, label, patch)
ccode_pt cond;
label_no label;
char *patch;
{
    register struct labdatstruct *labptr;

    labptr = &vislab[(int)nextvislab];
    labptr->labcond = cond;
    labptr->labnum = label;
    labptr->lablc = lc;
    labptr->labpatch = patch;
    if (++nextvislab == MAXVISLAB)
	nextvislab = 0;
}

/* bump location counter */

PUBLIC void bumplc()
{
    ++lc;
}

/* bump location counter by 2 */

PUBLIC void bumplc2()
{
    lc += 2;
}

/* bump location counter by 3 */

PUBLIC void bumplc3()
{
    lc += 3;
}

/* clear out labels in function */

PUBLIC void clearfunclabels()
{
    register struct symstruct *symptr;
    register struct symstruct *tmp;

    for (symptr = namedfirst; symptr != NULL;)
    {
	if (symptr->indcount == 2)
	    error("undefined label");
	symptr->indcount = 0;
	tmp = symptr;
	symptr = (struct symstruct *) symptr->type;
	tmp->type = NULL;
    }
    namedlast = namedfirst = NULL;
}

/* clear out labels no longer in buffer */

PUBLIC void clearlabels(patchbuf, patchtop)
char *patchbuf;
char *patchtop;
{
    register struct labdatstruct *labptr;
    struct labdatstruct *labtop;
    register char *labpatch;

    for (labptr = &vislab[0], labtop = &vislab[MAXVISLAB];
	 labptr < labtop; ++labptr)
	if ((labpatch = labptr->labpatch) >= patchbuf && labpatch < patchtop)
	    labptr->labnum = 0;
}

/* clear out labels in switch statement */

PUBLIC void clearswitchlabels()
{
    register struct symstruct *symptr;

    for (symptr = namedfirst; symptr != NULL;
	 symptr = (struct symstruct *) symptr->type)
	if (symptr->indcount == 3)
	{
	    equlab(symptr->offset.offlabel, lowsp);
	    symptr->indcount = 4;
	}
}

/* return location counter */

PUBLIC uoffset_T getlc()
{
    return (uoffset_T) lc;
}

/* define location of label and backpatch references to it */

PUBLIC void deflabel(label)
label_no label;
{
    char *cnameptr;
    struct labdatstruct *labmin;
    struct labdatstruct *labmax;
    struct labdatstruct *labmid;
    struct labdatstruct *labptrsave;
    offset_T nlonger;

    outnlabel(label);
    {
	register struct labdatstruct *labptr;
	register char *labpatch;

	labmin = &vislab[0];
	labmax = &vislab[MAXVISLAB];
	labptr = labmid = &vislab[(int)nextvislab];
	if (!watchlc)
	    do
	    {
		if (labptr == labmin)
		    labptr = &vislab[MAXVISLAB];
		--labptr;
		if (labptr->labnum == label)
		{
		    if ((labpatch = labptr->labpatch) != NULL &&
			isshortbranch(lc - labptr->lablc))
		    {
#ifdef I8088 /* patch "bcc(c) to j(c)(c)( ) */
			*labpatch = 'j';
			*(labpatch + 1) =
			    *(cnameptr = scondnames[(int)labptr->labcond]);
#endif
#ifdef MC6809
# ifdef NEW_MC6809 /* patch JMP\t> or LBCC\t to BCC \t */
			*labpatch = 'B';
			*(labpatch + 4) = '\t';	/* redundant unless JMP */
			*(labpatch + 1) =
			    *(cnameptr = condnames[labptr->labcond]);
# else
			if (labptr->labcond == RA)
			    strncpy(labpatch, "BRA\t\t", 5);
			else
			    *labpatch = '\t';
			goto over;
# endif
#endif
			*(labpatch + 2) = *(cnameptr + 1);
			*(labpatch + 3) = ' ';
#ifdef MC6809
# ifndef NEW_MC6809 /* patch JMP\t> or LBCC\t to BCC \t */
		over: ;		/* temp regression test kludge */
# endif
#endif
			nlonger = jcclonger;
			if (labptr->labcond == RA)
			    nlonger = jmplonger;
			lc -= nlonger;
			labptrsave = labptr;
			while (++labptr != labmid)
			    if (labptr == labmax)
				labptr = &vislab[-1];
			    else
				labptr->lablc -= nlonger;
			labptr = labptrsave;
		    }
		}
	    }
	    while (labptr != labmid);
    }
    addlabel((ccode_pt) 0, label, (char *) NULL);
}

PRIVATE struct labdatstruct *findlabel(label)
label_no label;
{
    register struct labdatstruct *labptr;
    struct labdatstruct *labtop;

    for (labptr = &vislab[0], labtop = &vislab[MAXVISLAB];
	 labptr < labtop; ++labptr)
	if (labptr->labnum == label)
	{
	    if (labptr->labpatch != 0)
		break;
	    return labptr;
	}
    return (struct labdatstruct *) NULL;
}

/* reserve a new label, from top down to temp avoid renumbering low labels */

PUBLIC label_no gethighlabel()
{
    return --lasthighlab;
}

/* reserve a new label */

PUBLIC label_no getlabel()
{
    return ++lastlab;
}

/* jump to label */

PUBLIC void jump(label)
label_no label;
{
    lbranch(RA, label);
}

/* long branch on condition to label */

PUBLIC void lbranch(cond, label)
ccode_pt cond;
label_no label;
{
#ifdef I8088
    char *cnameptr;

#endif
    struct labdatstruct *labptr;
    char *oldoutptr;

    if ((ccode_t) cond == RN)
	return;
    if ((labptr = findlabel(label)) != NULL &&
	isshortbranch(lc - labptr->lablc + 2))
    {
	sbranch(cond, label);
	return;
    }
    oldoutptr = outbufptr;
    if (cond == RA)
	outjumpstring();
    else
    {
	outlbranch();
#ifdef I8088
	outbyte(*(cnameptr = lcondnames[(int) cond]));
	outbyte(*(cnameptr + 1));
	if ((ccode_t) cond == LS || (ccode_t) cond == HS)
	    outbyte('s');	/* "blos" or "bhis" */
	else
	    outbyte(' ');
	outtab();
	bumplc2();
#ifdef I80386
	if (i386_32)
	    bumplc();
#endif
#endif
#ifdef MC6809
	outcond(cond);
	bumplc();
#endif
    }
    outlabel(label);
    outnl();
    if (labptr == NULL && oldoutptr < outbufptr)	/* no wrap-around */
	addlabel(cond, label, oldoutptr);
}

/* look up the name gsname in label space, install it if new */

PUBLIC struct symstruct *namedlabel()
{
    struct symstruct *symptr;

    gs2name[1] = 0xFF;
    if ((symptr = findlorg(gs2name + 1)) == NULL)
    {
	symptr = addglb(gs2name + 1, vtype);
	symptr->flags = LABELLED;
    }
    if (symptr->indcount < 2)
    {
	symptr->indcount = 2;
	symptr->offset.offlabel = gethighlabel();
	if (namedfirst == NULL)
	    namedfirst = symptr;
	else
	    namedlast->type = (struct typestruct *) symptr;
	namedlast = symptr;
	symptr->type = NULL;
    }
    return symptr;
}

#ifdef MC6809

/* print condition code name */

PUBLIC void outcond(cond)
ccode_pt cond;
{
    char *cnameptr;

    outbyte(*(cnameptr = condnames[(ccode_t) cond]));
    outbyte(*(cnameptr + 1));
    outtab();
}

#endif

/* print label */

PUBLIC void outlabel(label)
label_no label;
{
    outbyte(LABELSTARTCHAR);
    outhexdigs((uoffset_T) label);
}

/* print label and newline */

PUBLIC void outnlabel(label)
label_no label;
{
    outlabel(label);
#ifdef LABELENDCHAR
    outnbyte(LABELENDCHAR);
#else
    outnl();
#endif
}

/* short branch on condition to label */

PUBLIC void sbranch(cond, label)
ccode_pt cond;
label_no label;
{
#ifdef I8088
    char *cnameptr;

    if ((ccode_t) cond != RN)
    {
	outsbranch();
	outbyte(*(cnameptr = scondnames[(int) cond]));
	outbyte(*(cnameptr + 1));
	outtab();
	outlabel(label);
	outnl();
    }
#endif
#ifdef MC6809
    outsbranch();
    outcond(cond);
    outlabel(label);
    outnl();
#endif
}

/* reverse bump location counter */

PUBLIC void unbumplc()
{
    --lc;
}
