/*++
/* NAME
/*	tok_io 3
/* SUMMARY
/*	token I/O
/* PACKAGE
/*	unproto
/* SYNOPSIS
/*	#include "token.h"
/*
/*	struct token *tok_get(skip_flag)
/*	int skip_flag;
/*
/*	void tok_unget(t)
/*	struct token *t;
/*
/*	void tok_flush(t)
/*	struct token *t;
/*
/*	void tok_show(t)
/*	struct token *t;
/*
/*	void put_str(s)
/*	char *s;
/*
/*	void put_ch(c)
/*	int c;
/*
/*	void show_line_control()
/*
/*	char curr_path[];
/*	int curr_line;
/* DESCRIPTION
/*	These functions read from stdin and write to stdout. The
/*	output functions maintain some memory so that two successive
/*	words will always be separated by white space.
/*
/*	The input routines eliminate backslash-newline from the input.
/*
/*	tok_get() reads the next token from standard input. It returns
/*	a null pointer when the end of input is reached. If the skip_flag
/*	argument is nonzero, white space (except newline) will be skipped.
/*
/*	tok_unget() implements a limited amount of token push back.
/*
/*	tok_show() displays the contents of a (possibly composite) token
/*	on the standard output.
/*
/*	tok_flush() displays the contents of a (possibly composite) token
/*	on the standard output and makes it available for re-use.
/*
/*	put_str() writes a null-terminated string to standard output.
/*
/*	put_ch() writes one character to standard output.
/*
/*	show_line_control() displays the line number of the next line
/*	to be written to standard output, in a format suitable for the C
/*	compiler parser phase.
/*
/*	The curr_path[] and curr_line variables contain the input file name and 
/*	line number of the most recently read token.
/* BUGS
/*	The tokenizer is just good enough for the unproto filter.
/*	As a benefit, it is quite fast.
/* AUTHOR(S)
/*	Wietse Venema
/*	Eindhoven University of Technology
/*	Department of Mathematics and Computer Science
/*	Den Dolech 2, P.O. Box 513, 5600 MB Eindhoven, The Netherlands
/* LAST MODIFICATION
/*	91/11/30 21:10:26
/* VERSION/RELEASE
/*	1.2
/*--*/

static char io_sccsid[] = "@(#) tok_io.c 1.2 91/11/30 21:10:26";

/* C library */

#include <stdio.h>
#include <ctype.h>

extern char *strchr();
extern char *malloc();
extern char *realloc();
extern char *strcpy();

/* Application-specific stuff */

#include "token.h"
#include "vstring.h"
#include "error.h"

/* Stuff to keep track of original source file name and position */

char    curr_path[BUFSIZ];		/* current file name */
int     curr_line = 0;			/* # of last read line */

/* Forward declarations */

static void read_quoted();
static void read_comment();

/* Buffered i/o stuff */

static struct vstring *buf = 0;		/* read-ahead buffer */
static char *bp = "";			/* buffer position */

#ifdef DEBUG
#define	INITBUF	1			/* small initial buffer size */
#else
#define	INITBUF BUFSIZ			/* reasonable initial buffer size */
#endif

#define	input()		(*bp ? *bp++ : next_line())
#define	unput(c)	(*--bp = (c))

#define	TOK_BUFSIZE	5		/* token push-back buffer size */

static struct token *tok_buf[TOK_BUFSIZE];
static int tok_bufpos = 0;

/* Type of last token sent to output, for pretty printing */

static int last_tok = 0;

/* Directives that should be ignored. */

#ifdef IGNORE_DIRECTIVES

static char *ignore_directives[] = {
    IGNORE_DIRECTIVES,
    0,
};

#endif

/* Modified string and ctype stuff. */

#define	STREQUAL(x,y)	(*(x) == *(y) && strcmp((x),(y)) == 0)

#define	ISALNUM(c)	(isalnum(c) || (c) == '_')
#define	ISALPHA(c)	(isalpha(c) || (c) == '_')
#define	ISSPACE(c)	(isspace(c) && c != '\n')
#define	ISDOT(c)	(c == '.')

/* Collect all characters that satisfy one condition */

#define	COLLECT(v,c,cond) { \
				register struct vstring *vs = v; \
				register char *cp = vs->str; \
				*cp++ = c; \
				for (;;) { \
				    if ((c = input()) == 0) { \
					break; \
				    } else if (cond) { \
					if (VS_ADDCH(vs, cp, c) == 0) \
					    error(1, "out of memory"); \
				    } else { \
					unput(c); \
					break; \
				    } \
				} \
				*cp = 0; \
			    }

/* do_control - parse control line, uses tok_get() */

static int do_control()
{
    struct token *t1;
    struct token *t2;
    int     pass_thru = 1;		/* 0 = ignore, 1 = output */

    (void) input();				/* skip the hash */

    if (t1 = tok_get(NO_WSPACE)) {
	switch (t1->tokno) {

	    /*
	     * In case of line number control, the remainder of the line has
	     * the format: linenumber "pathname".
	     */
	case TOK_NUMBER:
	    if (t2 = tok_get(NO_WSPACE)) {
		if (t2->tokno == '"') {
		    curr_line = atoi(t1->vstr->str) - 1;
		    strcpy(curr_path, t2->vstr->str);
		}
		tok_free(t2);
	    }
	    break;

#ifdef IGNORE_DIRECTIVES
	case TOK_WORD:
	    /* Optionally ignore other #directives, such as #pragma. */
	    {
		char  **cpp;
		char   *cp = t1->vstr->str;

		for (cpp = ignore_directives; *cpp; cpp++) {
		    if (STREQUAL(cp, *cpp)) {
			pass_thru = 0;
			break;
		    }
		}
	    }
	    break;
#endif
	}
	tok_free(t1);
    }
    return (pass_thru);
}

/* next_line - read one logical line, handle #control */

static int next_line()
{
    register int c;
    register char *cp;

    /* Allocate buffer upon first entry */

    if (buf == 0)
	buf = vs_alloc(INITBUF);

    for (;;) {
	cp = buf->str;

	/* Account for EOF and line continuations */

	while ((c = getchar()) != EOF) {
	    if (VS_ADDCH(buf, cp, c) == 0)	/* store character */
		error(1, "out of memory");
	    if (c == '\n') {			/* real end of line */
		curr_line++;
		break;
	    } else if (c == '\\') {
		if ((c = getchar()) == EOF) {	/* XXX strip backslash-EOF */
		    break;
		} else if (c == '\n') {		/* strip backslash-newline */
		    curr_line++;
		    put_ch('\n');		/* preserve line count */
		    cp--;			/* un-store backslash */
		} else {
		    ungetc(c, stdin);		/* keep backslash-other */
		}
	    }
	}
	*cp = 0;
	bp = buf->str;

	/* Account for EOF and #control */

	switch (bp[0]) {
	case 0:					/* EOF */
	    return (0);
	case '#':				/* control */
	    if (do_control())
		fputs(buf->str, stdout);	/* pass through */
	    else
		putchar('\n');			/* filter out */
	    break;
	default:				/* non-control */
	    return (input());
	}
    }
}

/* tok_unget - push back one token */

void    tok_unget(t)
register struct token *t;
{
    if (tok_bufpos >= TOK_BUFSIZE)
	error(1, "too much pushback");
    tok_buf[tok_bufpos++] = t;
}

/* tok_get - get next token */

struct token *tok_get(skip_flag)
int     skip_flag;
{
    register struct token *t;
    register int c;
    int     d;

    /* Use push-back token, if any. */

    if (tok_bufpos) {
	t = tok_buf[--tok_bufpos];
	return (t);
    }

    /*
     * Get one from the pool and fill it in. The loop is here in case we
     * should skip white-space tokens, which happens in a minority of all
     * cases.
     */

    t = tok_alloc();

    for (;;) {
	if ((c = input()) == 0) {
	    tok_free(t);
	    return (0);
	} else if (!isascii(c)) {
	    t->vstr->str[0] = c;
	    t->vstr->str[1] = 0;
	    t->tokno = TOK_OTHER;
	    return (t);
	} else if (c == '"' || c == '\'') {
	    read_quoted(t, c);
	    t->tokno = c;
	    return (t);
	} else if (ISALPHA(c)) {
	    COLLECT(t->vstr, c, ISALNUM(c));
	    t->tokno = TOK_WORD;
	    return (t);
	} else if (isdigit(c)) {
	    COLLECT(t->vstr, c, isdigit(c));
	    t->tokno = TOK_NUMBER;
	    return (t);
	} else if (ISSPACE(c)) {
	    COLLECT(t->vstr, c, ISSPACE(c));
	    if (skip_flag)
		continue;
	    t->tokno = TOK_WSPACE;
	    return (t);
	} else if (ISDOT(c)) {
	    COLLECT(t->vstr, c, ISDOT(c));
	    t->tokno = TOK_OTHER;
	    return (t);
	} else {
	    t->vstr->str[0] = c;
	    if (c == '/') {
		if ((d = input()) == '*') {
		    t->vstr->str[1] = d;	/* comment */
		    read_comment(t->vstr);
		    if (skip_flag)
			continue;
		    t->tokno = TOK_WSPACE;
		    return (t);
		} else {
		    unput(d);
		}
	    }
	    t->vstr->str[1] = 0;
	    t->tokno = c;
	    return (t);
	}
    }
}

/* read_qouted - read string or character literal */

static void read_quoted(t, ch)
register struct token *t;
int     ch;
{
    register char *cp = t->vstr->str;
    register int c;

    *cp++ = ch;

    while (c = input()) {
	if (c == '\n') {			/* newline in string */
	    unput(c);
	    break;
	}
	if (VS_ADDCH(t->vstr, cp, c) == 0)	/* store character */
	    error(1, "out of memory");
	if (c == ch)				/* end of string */
	    break;
	if (c == '\\')				/* eat next character */
	    if ((c = input()) != 0 && VS_ADDCH(t->vstr, cp, c) == 0)
		error(1, "out of memory");
    }
    *cp = 0;
    return;
}

/* read_comment - stuff a whole comment into one huge token */

static void read_comment(vs)
register struct vstring *vs;
{
    register char *cp = vs->str + 2;	/* skip slash star */
    register int c;
    register int d;

    while (c = input()) {
	if (VS_ADDCH(vs, cp, c) == 0)
	    error(1, "out of memory");
	if (c == '*') {
	    if ((d = input()) == '/') {
		if (VS_ADDCH(vs, cp, d) == 0)
		    error(1, "out of memory");
		break;
	    } else {
		unput(d);
	    }
	}
    }
    *cp = 0;
}

/* put_str - output a string */

void    put_str(s)
char   *s;
{
    fputs(s, stdout);
    last_tok = s[0];				/* XXX */
#ifdef DEBUG
    fflush(stdout);
#endif
}

/* put_ch - put character */

void    put_ch(c)
int     c;
{
    last_tok = putchar(c);
#ifdef DEBUG
    fflush(stdout);
#endif
}

/* tok_show - output (possibly composite) token */

void    tok_show(t)
struct token *t;
{
    register struct token *p;
    register struct token *s;

    switch (t->tokno) {
    case TOK_LIST:
	for (s = t->head; s; s = s->next) {
	    put_ch(s->tokno);			/* opening paren or ',' */
	    for (p = s->head; p; p = p->next)
		tok_show(p);
	}
	put_ch(')');				/* closing paren */
	break;
    case TOK_WORD:
	if (ISALPHA(last_tok))
	    putchar(' ');
	/* FALLTRHOUGH */
    default:
	fputs(t->vstr->str, stdout);		/* token contents */
	last_tok = t->vstr->str[0];
#ifdef DEBUG
	fflush(stdout);
#endif
	if (t->head)				/* trailing blanks */
            for (p = t->head; p; p = p->next)
                tok_show(p);
    }
}
