/*++
/* NAME
/*	tok_class 3
/* SUMMARY
/*	token classification
/* PACKAGE
/*	unproto
/* SYNOPSIS
/*	#include "token.h"
/*
/*	struct token *tok_class(skip)
/*	int skip;
/* DESCRIPTION
/*	tok_class() collects a single and composite tokens, and
/*	recognizes keywords.
/*	At present, the only composite tokens are ()-delimited,
/*	comma-separated lists.
/*
/*	The skip argument has the same meaning as with the tok_get()
/*	function.
/* DIAGNOSTICS
/*	The code complains if input terminates in the middle of a list.
/* BUGS
/*	Does not preserve white space at the beginning of a list element
/*	or after the end of a list.
/* AUTHOR(S)
/*	Wietse Venema
/*	Eindhoven University of Technology
/*	Department of Mathematics and Computer Science
/*	Den Dolech 2, P.O. Box 513, 5600 MB Eindhoven, The Netherlands
/* LAST MODIFICATION
/*	91/11/30 21:10:28
/* VERSION/RELEASE
/*	1.3
/*--*/

static char class_sccsid[] = "@(#) tok_class.c 1.3 91/11/30 21:10:28";

/* C library */

#include <stdio.h>

extern char *strcpy();

/* Application-specific stuff */

#include "error.h"
#include "vstring.h"
#include "token.h"
#include "symbol.h"

static struct token *tok_list();
static void tok_list_struct();
static void tok_list_append();

/* tok_space_append - append trailing space except after list */

#define	tok_space_append(list,t) { \
				    if (list == 0 	/* leading space*/ \
					|| list->tokno == TOK_LIST) \
					tok_free(t); \
				    else \
					tok_list_append(list, t); \
				}

/* tok_class - discriminate single tokens, keywords, and composite tokens */

struct token *tok_class(skip)
int     skip;
{
    register struct token *t;
    register struct symbol *s;

    if (t = tok_get(skip)) {
	switch (t->tokno) {
	case '(':				/* beginning of list */
	    t = tok_list(t);
	    break;
	case TOK_WORD:				/* look up keyword */
	    if (s = sym_find(t->vstr->str))
		t->tokno = s->type;
	    break;
	}
    }
    return (t);
}

/* tok_list - collect ()-delimited, comma-separated list of tokens */

static struct token *tok_list(t)
struct token *t;
{
    register struct token *list = tok_alloc();
    char    filename[BUFSIZ];
    int     lineno;

    /* Save context of '(' for diagnostics. */

    strcpy(filename, curr_path);
    lineno = curr_line;

    list->tokno = TOK_LIST;
    list->head = list->tail = t;
#ifdef DEBUG
    strcpy(list->vstr->str, "LIST");
#endif

    for (;;) {
	if ((t = tok_get(DO_WSPACE)) == 0) {	/* skip blanks */
	    error_where(0, filename, lineno, "unmatched '('");
	    return (list);			/* do not waste any data */
	}
	switch (t->tokno) {
	case ')':				/* end of list */
	    tok_free(t);
	    return (list);
	case '{':				/* struct/union type */
	    tok_list_struct(list->tail, t);
	    break;
	case TOK_WSPACE:			/* preserve trailing blanks */
	    tok_space_append(list->tail->tail, t);	/* except after list */
	    break;
	case '\n':				/* preserve line count */
	    tok_flush(t);
	    break;
	case ',':				/* list separator */
	    tok_list_append(list, t);
	    break;
	case '(':				/* beginning of list */
	    tok_list_append(list->tail, tok_list(t));
	    break;
	default:				/* ordinary token */
	    tok_list_append(list->tail, t);
	    break;
	}
    }
}

/* tok_list_struct - collect structured type info within list */

static void tok_list_struct(list, t)
register struct token *list;
register struct token *t;
{
    tok_list_append(list, t);

    while (t = tok_class(DO_WSPACE)) {
	switch (t->tokno) {
	case '\n':				/* preserve line count */
	    tok_flush(t);
	    break;
	case TOK_WSPACE:			/* preserve trailing blanks */
	    tok_space_append(list->tail, t);	/* except after list */
	    break;
	case '{':				/* recurse */
	    tok_list_struct(list, t);
	    break;
	case '}':				/* done */
	    tok_list_append(list, t);
	    return;
	default:				/* other */
	    tok_list_append(list, t);
	    break;
	}
    }
}

/* tok_list_append - append data to list */

static void tok_list_append(h, t)
struct token *h;
struct token *t;
{
    if (h->head == 0) {
	h->head = h->tail = t;
    } else {
	h->tail->next = t;
	h->tail = t;
    }
}
