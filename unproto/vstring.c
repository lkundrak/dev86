/*++
/* NAME
/*	vs_alloc(), VS_ADDCH()
/* SUMMARY
/*	auto-resizing string library
/* PACKAGE
/*	vstring
/* SYNOPSIS
/*	#include "vstring.h"
/*
/*	struct vstring *vs_alloc(len)
/*	int len;
/*
/*	int VS_ADDCH(vs, wp, ch)
/*	struct vstring *vs;
/*	char *wp;
/*	int ch;
/* DESCRIPTION
/*	These functions and macros implement a small library for
/*	arbitrary-length strings that grow automatically when
/*	they fill up. The allocation strategy is such that there
/*	will always be place for the terminating null character.
/*
/*	vs_alloc() allocates storage for a variable-length string.
/*
/*	VS_ADDCH() adds a character to a variable-length string
/*	and automagically extends the string if fills up.
/*	\fIvs\fP is a pointer to a vstring structure; \fIwp\fP
/*	the current write position in the corresponding character
/*	array; \fIch\fP the character value to be written.
/*	Note that VS_ADDCH() is a macro that evaluates some
/*	arguments more than once.
/* DIAGNOSTICS
/*	VS_ADDCH() returns zero if it was unable to dynamically
/*	resize a string.
/*
/*	vs_alloc() returns a null pointer in case of problems.
/* BUGS
/*	Auto-resizing may change the address of the string data in
/*	a vstring structure. Beware of dangling pointers.
/* AUTHOR(S)
/*	Wietse Venema
/*	Eindhoven University of Technology
/*	Department of Mathematics and Computer Science
/*	Den Dolech 2, P.O. Box 513, 5600 MB Eindhoven, The Netherlands
/* LAST MODIFICATION
/*	91/09/22 21:21:38
/* VERSION/RELEASE
/*	1.2
/*--*/

static char vstring_sccsid[] = "@(#) vstring.c 1.2 91/09/22 21:21:38";

/* C library */

extern char *malloc();
extern char *realloc();

/* Application-specific stuff */

#include "vstring.h"

/* vs_alloc - initial string allocation */

struct vstring *vs_alloc(len)
int     len;
{
    register struct vstring *vp;

    if (len < 1 
	|| (vp = (struct vstring *) malloc(sizeof(struct vstring))) == 0
	|| (vp->str = malloc(len)) == 0)
	return (0);
    vp->last = vp->str + len - 1;
    return (vp);
}

/* vs_realloc - extend string, update write pointer */

char   *vs_realloc(vp, cp)
register struct vstring *vp;
char   *cp;
{
    int     where = cp - vp->str;
    int     len = vp->last - vp->str + 1;

    if ((vp->str = realloc(vp->str, len *= 2)) == 0)
	return (0);
    vp->last = vp->str + len - 1;
    return (vp->str + where);
}
