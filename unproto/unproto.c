/*++
/* NAME
/*	unproto 1
/* SUMMARY
/*	ANSI C to old C converter
/* PACKAGE
/*	unproto
/* SYNOPSIS
/*	/lib/cpp ... | unproto
/*
/*	/somewhere/cpp ...
/* DESCRIPTION
/*	This document describes a filter that sits between the
/*	C preprocessor (usually \fI/lib/cpp\fP) and the next C compiler
/*	pass. It rewrites ANSI-C style function headers, function type
/*	declarations, function pointer types, and function pointer casts
/*	to old style. Other ANSI-isms are passed on without modification
/*	(token pasting, pragmas, etcetera).
/*
/*	For maximal flexibility, the "cpp | unproto" pipeline can  be
/*	packaged as an executable shell script named "/somewhere/cpp".
/*	This script should then be specified to the C compiler as a 
/*	non-default preprocessor. It will not work if your C compiler
/*	specifies output file names to the preprocessor.
/*
/*	The overhead of shell script interpretation can be avoided by
/*	having the unprototyper itself open the pipe to the preprocessor.
/*	In that case, the source should be compiled with the PIPE_THROUGH_CPP 
/*	macro defined (usually as "/lib/cpp"), and the resulting binary 
/*	should be installed as "/somewhere/cpp".
/* SEE ALSO
/* .ad
/* .fi
/*	cc(1), how to specify a non-default C preprocessor.
/*
/*	Some versions of the lint command are implemented as a shell
/*	script. It should require only minor modification for integration
/*	with the unprotoizer. Other versions of the lint command accept the same
/*	command syntax as the C compiler for the specification of a non-default
/*	preprocessor. Some research may be needed.
/* DIAGNOSTICS
/*	The progam will complain if it unexpectedly
/*	reaches the end of input.
/* BUGS
/*	Should be run on preprocessed source only, i.e. after macro expansion.
/*
/*	Declarations of (whatever) are misunderstood and will result in
/*	syntax errors.
/*
/*	Does not generate explicit type casts for function argument 
/*	expressions.
/* AUTHOR(S)
/*	Wietse Venema (wietse@wzv.win.tue.nl)
/*	Eindhoven University of Technology
/*	Department of Mathematics and Computer Science
/*	Den Dolech 2, P.O. Box 513, 5600 MB Eindhoven, The Netherlands
/* LAST MODIFICATION
/*	91/09/22 21:21:35
/* VERSION/RELEASE
/*	1.2
/*--*/

static char unproto_sccsid[] = "@(#) unproto.c 1.3 91/11/30 21:10:30";

/* C library */

#include <stdio.h>
#include <errno.h>

extern void exit();
extern int optind;
extern char *optarg;
extern int getopt();

/* Application-specific stuff */

#include "vstring.h"
#include "stdarg.h"
#include "token.h"
#include "error.h"
#include "symbol.h"

/* Forward declarations. */

static struct token *dcl_flush();
static void block_flush();
static void block_dcls();
static struct token *show_func_ptr_type();
static struct token *show_struct_type();
static void show_arg_name();
static void show_type();
static void pair_flush();
static void check_cast();

#define	check_cast_flush(t)	(check_cast(t), tok_free(t))

#ifdef PIPE_THROUGH_CPP
static int pipe_stdin_through_cpp();
#endif

/* Disable debugging printfs while preserving side effects. */

#ifdef DEBUG
#define	DPRINTF	printf
#else
#define	DPRINTF (void)
#endif

/* An attempt to make some complicated expressions a bit more readable. */

#define	STREQ(x,y)		(*(x) == *(y) && !strcmp((x),(y)))

#define	LAST_ARG_AND_EQUAL(s,c)	((s)->next == 0 && (s)->head \
				&& ((s)->head == (s)->tail) \
				&& (STREQ((s)->head->vstr->str, (c))))

#define	LIST_BEGINS_WITH_STAR(s) (s->head->head && s->head->head->tokno == '*')

#define	IS_FUNC_PTR_TYPE(s)	(s->tokno == TOK_LIST && s->next \
				&& s->next->tokno == TOK_LIST \
				&& LIST_BEGINS_WITH_STAR(s))

/* main - driver */

int     main(argc, argv)
int     argc;
char  **argv;
{
    register struct token *t;
#ifdef	PIPE_THROUGH_CPP			/* pipe through /lib/cpp */
    int     cpp_status;
    int     wait_pid;
    int     cpp_pid;

    cpp_pid = pipe_stdin_through_cpp(argv);
#else
    if ( argc > 1 ) {
       if( freopen(argv[1], "r", stdin) == 0 ) {
           fprintf(stderr, "Cannot open '%s'\n", argv[1]);
	   exit(9);
       }
    }
    if ( argc > 2 ) {
       if( freopen(argv[2], "w", stdout) == 0 ) {
           fprintf(stderr, "Cannot open '%s'\n", argv[2]);
	   exit(9);
       }
    }
#endif

    sym_init();					/* prime the symbol table */

    while (t = tok_class(DO_WSPACE)) {
	if (t = dcl_flush(t)) {			/* try declaration */
	    if (t->tokno == '{') {		/* examine rejected token */
		block_flush(t);			/* body */
	    } else {
		tok_flush(t);			/* other, recover */
	    }
	}
    }

#ifdef	PIPE_THROUGH_CPP			/* pipe through /lib/cpp */
    while ((wait_pid = wait(&cpp_status)) != -1 && wait_pid != cpp_pid)
	 /* void */ ;
    return (wait_pid != cpp_pid || cpp_status != 0);
#else
    return (0);
#endif
}

#ifdef	PIPE_THROUGH_CPP		/* pipe through /lib/cpp */

/* pipe_stdin_through_cpp - avoid shell script overhead */

static int pipe_stdin_through_cpp(argv)
char  **argv;
{
    int     pipefds[2];
    int     pid;
    char  **cpptr = argv;

    /*
     * With most UNIX implementations, the second non-option argument to
     * /lib/cpp specifies the output file. If an output file other than
     * stdout is specified, we must force /lib/cpp to write to stdout, and we
     * must redirect our own standard output to the specified output file.
     */

#define	IS_OPTION(cp) ((cp)[0] == '-' && (cp)[1] != 0)

    /* Skip to first non-option argument, if any. */

    while (*++cpptr && IS_OPTION(*cpptr))
	 /* void */ ;

    /*
     * Assume that the first non-option argument is the input file name. The
     * next argument could be the output destination or an option (System V
     * Release 2 /lib/cpp gets the options *after* the file arguments).
     */

    if (*cpptr && *++cpptr && **cpptr != '-') {

	/*
	 * The first non-option argument is followed by another argument that
	 * is not an option ("-stuff") or a hyphen ("-"). Redirect our own
	 * standard output before we clobber the file name.
	 */

	if (freopen(*cpptr, "w", stdout) == 0) {
	    perror(*cpptr);
	    exit(1);
	}
	/* Clobber the file name argument so that /lib/cpp writes to stdout */

	*cpptr = "-";
    }
    /* Set up the pipe that connects /lib/cpp to our standard input. */

    if (pipe(pipefds)) {
	perror("pipe");
	exit(1);
    }
    switch (pid = fork()) {
    case -1:					/* error */
	perror("fork");
	exit(1);
    case 0:					/* child */
	close(pipefds[0]);			/* close reading end */
	close(1);				/* connect stdout to pipe */
	if (dup(pipefds[1]) != 1)
	    error(1, "dup() problem");
	close(pipefds[1]);			/* close redundant fd */
	execv(PIPE_THROUGH_CPP, argv);
	perror(PIPE_THROUGH_CPP);
	exit(1);
    default:					/* parent */
	close(pipefds[1]);			/* close writing end */
	close(0);				/* connect stdin to pipe */
	if (dup(pipefds[0]) != 0)
	    error(1, "dup() problem");
	close(pipefds[0]);			/* close redundant fd */
	return (pid);
    }
}

#endif

/* header_flush - rewrite new-style function header to old style */

static void header_flush(t)
register struct token *t;
{
    register struct token *s;

    /* Do argument names, but suppress void and rewrite trailing ... */

    if (LAST_ARG_AND_EQUAL(t->head, "void")) {
	put_str("()\n");			/* no arguments */
    } else {
	for (s = t->head; s; s = s->next) {	/* foreach argument... */
	    if (LAST_ARG_AND_EQUAL(s, "...")) {
#ifdef _VA_ALIST_				/* see ./stdarg.h */
		put_ch(s->tokno);		/* ',' */
		put_str(_VA_ALIST_);		/* varargs magic */
#endif
	    } else {
		put_ch(s->tokno);		/* opening '(' or ',' */
		show_arg_name(s);		/* extract argument name */
	    }
	}
	put_str(")\n");				/* closing ')' */
    }

    /* Do argument types, but suppress void and trailing ... */

    if (!LAST_ARG_AND_EQUAL(t->head, "void")) {
	for (s = t->head; s; s = s->next) {	/* foreach argument... */
	    if (!LAST_ARG_AND_EQUAL(s, "...")) {
		if (s->head != s->tail) {	/* really new-style argument? */
		    show_line_control();	/* fix line number */
		    show_type(s);		/* rewrite type info */
		    put_str(";\n");
		}
	    }
	}
    }
    tok_free(t);
    show_line_control();			/* because '{' follows */
}

/* show_arg_name - extract argument name from argument type info */

static void show_arg_name(s)
register struct token *s;
{
    if (s->head) {
	register struct token *p;
	register struct token *t = 0;

	/* Find the last interesting item. */

	for (p = s->head; p; p = p->next) {
	    if (p->tokno == TOK_WORD) {
		t = p;				/* remember last word */
	    } else if (IS_FUNC_PTR_TYPE(p)) {
		t = p;				/* or function pointer */
		p = p->next;
	    }
	}

	/* Extract argument name from last interesting item. */

	if (t) {
	    if (t->tokno == TOK_LIST)
		show_arg_name(t->head);		/* function pointer, recurse */
	    else
		tok_show(t);			/* print last word */
	}
    }
}

/* show_type - rewrite type to old-style syntax */

static void show_type(s)
register struct token *s;
{
    register struct token *p;

    for (p = s->head; p; p = p->next) {
	if (IS_FUNC_PTR_TYPE(p)) {
	    p = show_func_ptr_type(p);		/* function pointer type */
	} else {
	    tok_show(p);			/* other */
	}
    }
}

/* show_func_ptr_type - display function_pointer type using old-style syntax */

static struct token *show_func_ptr_type(t)
struct token *t;
{
    register struct token *s;

    /*
     * Rewrite (list1) (list2) to (list1) (). Only (list1) is given to us;
     * the caller must have verified the presence of (list2). Account for the
     * rare case that (list1) is a comma-separated list. That should be an
     * error, but we do not want to waste any information.
     */

    for (s = t->head; s; s = s->next) {
	put_ch(s->tokno);			/* opening paren or ',' */
	show_type(s);				/* recurse */
    }
    put_str(")()");				/* closing paren */
    return (t->next);
}

/* show_struct_type - display structured type, rewrite function-pointer types */

static struct token *show_struct_type(p)
register struct token *p;
{
    tok_show(p);				/* opening brace */

    while (p->next) {				/* XXX cannot return 0 */
	p = p->next;
	if (IS_FUNC_PTR_TYPE(p)) {
	    p = show_func_ptr_type(p);		/* function-pointer member */
	} else if (p->tokno == '{') {
	    p = show_struct_type(p);		/* recurse */
	} else {
	    tok_show(p);			/* other */
	    if (p->tokno == '}') {
		return (p);			/* done */
	    }
	}
    }
    DPRINTF("/* missing '}' */");
    return (p);
}

/* is_func_ptr_cast - recognize function-pointer type cast */

static int is_func_ptr_cast(t)
register struct token *t;
{
    register struct token *p;

    /*
     * Examine superficial structure. Require (list1) (list2). Require that
     * list1 begins with a star.
     */

    if (!IS_FUNC_PTR_TYPE(t))
	return (0);

    /*
     * Make sure that there is no name in (list1). Do not worry about
     * unexpected tokens, because the compiler will complain anyway.
     */

    for (p = t->head->head; p; p = p->next) {
	switch (p->tokno) {
	case TOK_LIST:				/* recurse */
	    return (is_func_ptr_cast(p));
	case TOK_WORD:				/* name in list */
	    return (0);
	}
    }
    return (1);					/* no name found */
}

/* check_cast - display ()-delimited, comma-separated list */

static void check_cast(t)
struct token *t;
{
    register struct token *s;
    register struct token *p;

    /*
     * Rewrite function-pointer types and function-pointer casts. Do not
     * blindly rewrite (*list1)(list2) to (*list1)(). Function argument lists
     * are about the only thing we can discard without provoking diagnostics
     * from the compiler.
     */

    for (s = t->head; s; s = s->next) {
	put_ch(s->tokno);			/* opening paren or ',' */
	for (p = s->head; p; p = p->next) {
	    switch (p->tokno) {
	    case TOK_LIST:
		if (is_func_ptr_cast(p)) {	/* not: IS_FUNC_PTR_TYPE(p) */
		    p = show_func_ptr_type(p);	/* or we might take away */
		} else {			/* function-call arguments */
		    check_cast(p);		/* recurse */
		}
		break;
	    case '{':
		p = show_struct_type(p);	/* rewrite func. ptr. types */
		break;
	    default:
		tok_show(p);
		break;
	    }
	}
    }
    put_ch(')');				/* closing paren */
}

/* block_dcls - on the fly rewrite decls/initializers at start of block */

static void block_dcls()
{
    register struct token *t;

    /*
     * Away from the top level, a declaration should be preceded by type or
     * storage-class information. That is why inside blocks, structs and
     * unions we insist on reading one word before passing the _next_ token
     * to the dcl_flush() function.
     * 
     * Struct and union declarations look the same everywhere: we make an
     * exception for these more regular constructs and pass the "struct" and
     * "union" tokens to the type_dcl() function.
     */

    while (t = tok_class(DO_WSPACE)) {
	switch (t->tokno) {
	case TOK_WSPACE:			/* preserve white space */
	case '\n':				/* preserve line count */
	    tok_flush(t);
	    break;
	case TOK_WORD:				/* type declarations? */
	    tok_flush(t);			/* advance to next token */
	    t = tok_class(DO_WSPACE);		/* null return is ok */
	case TOK_COMPOSITE:			/* struct or union */
	    if ((t = dcl_flush(t)) == 0)
		break;
	    /* FALLTRHOUGH */
	default:				/* end of declarations */
	    DPRINTF("/* end dcls */");
	    /* FALLTRHOUGH */
	case '}':				/* end of block */
	    tok_unget(t);
	    return;
	}
    }
}

/* block_flush - rewrite struct, union or statement block on the fly */

static void block_flush(t)
register struct token *t;
{
    static int count = 0;

    tok_flush(t);
    DPRINTF("/*%d*/", ++count);

    /*
     * Rewrite function pointer types in declarations and function pointer
     * casts in initializers at start of block.
     */

    block_dcls();

    /* Remainder of block: only rewrite function pointer casts. */

    while (t = tok_class(DO_WSPACE)) {
	if (t->tokno == TOK_LIST) {
	    check_cast_flush(t);
	} else if (t->tokno == '{') {
	    block_flush(t);
	} else {
	    tok_flush(t);
	    if (t->tokno == '}') {
		DPRINTF("/*%d*/", count--);
		return;
	    }
	}
    }
    DPRINTF("/* missing '}' */");
}

/* pair_flush - on the fly rewrite casts in grouped stuff */

static void pair_flush(t, start, stop)
register struct token *t;
register int start;
register int stop;
{
    tok_flush(t);

    while (t = tok_class(DO_WSPACE)) {
	if (t->tokno == start) {		/* recurse */
	    pair_flush(t, start, stop);
	} else if (t->tokno == TOK_LIST) {	/* expression or cast */
	    check_cast_flush(t);
	} else {				/* other, copy */
	    tok_flush(t);
	    if (t->tokno == stop) {		/* done */
		return;
	    }
	}
    }
    DPRINTF("/* missing '%c' */", stop);
}

/* initializer - on the fly rewrite casts in initializer */

static void initializer()
{
    register struct token *t;

    while (t = tok_class(DO_WSPACE)) {
	switch (t->tokno) {
	case ',':				/* list separator */
	case ';':				/* list terminator */
	    tok_unget(t);
	    return;
	case TOK_LIST:				/* expression or cast */
	    check_cast_flush(t);
	    break;
	case '[':				/* array substript, may nest */
	    pair_flush(t, '[', ']');
	    break;
	case '{':				/* structured data, may nest */
	    pair_flush(t, '{', '}');
	    break;
	default:				/* other, just copy */
	    tok_flush(t);
	    break;
	}
    }
}

/* func_ptr_dcl_flush - rewrite function pointer declaration */

static struct token *func_ptr_dcl_flush(list)
register struct token *list;
{
    register struct token *t;

    /*
     * Ignore blanks because they would be output earlier than the list that
     * preceded them... Recover gracefully from syntax errors.
     */

    while (t = tok_class(NO_WSPACE)) {
	switch (t->tokno) {
	case '\n':				/* preserve line count */
	    tok_flush(t);
	    break;
	case TOK_LIST:
	    /* Function pointer type: (list1) (list2) -> (list1) () */
	    (void) show_func_ptr_type(list);	/* may be recursive */
	    tok_free(list);
	    tok_free(t);
	    return (0);
	default:				/* not a declaration */
	    tok_unget(t);
	    return (list);
	}
    }

    /* Hit EOF; must be mistake, but do not waste any information. */

    return (list);
}

/* function_dcl_flush - rewrite function { heading, type declaration } */

static struct token *function_dcl_flush(list)
register struct token *list;
{
    register struct token *t;

    /*
     * Ignore blanks because they would be output earlier than the list that
     * preceded them...
     */

    while (t = tok_class(NO_WSPACE)) {
	switch (t->tokno) {
	case '\n':
	    /* Preserve line count */
	    tok_flush(t);
	    break;
	case '{':
	    /* Function heading: word (list) { -> old style heading */
	    header_flush(list);
	    tok_unget(t);
	    return (0);
	case TOK_WORD:
	    /* Old-style function heading: word (list) word...{ */
	    tok_flush(list);
	    tok_unget(t);
	    return (0);
	case TOK_LIST:
	    /* Function typedef? word (list1) (list) -> word (list1) () */
	    tok_flush(list);
	    put_str("()");
	    tok_free(t);
	    return (0);
	case ',':
	case ';':
	    /* Function type declaration: word (list) -> word () */
	    tok_free(list);
	    put_str("()");
	    tok_unget(t);
	    return (0);
	default:
	    /* Something else, reject the list. */
	    tok_unget(t);
	    return (list);
	}
    }

    /* Hit EOF; must be mistake, but do not waste any information. */

    return (list);
}

/* dcl_flush - parse declaration on the fly, return rejected token */

static struct token *dcl_flush(t)
register struct token *t;
{
    register int got_word;

    /*
     * Away from the top level, type or storage-class information is required
     * for an (extern or forward) function type declaration or a variable
     * declaration.
     * 
     * With our naive word-counting approach, this means that the caller should
     * read one word before passing the next token to us. This is how we
     * distinguish, for example, function declarations from function calls.
     * 
     * An exception are structs and unions, because they look the same at any
     * level. The caller should give is the "struct" or "union" token.
     */

    for (got_word = 0; t; t = tok_class(DO_WSPACE)) {
	switch (t->tokno) {
	case TOK_WSPACE:			/* advance past blanks */
	case '\n':				/* advance past newline */
	case '*':				/* indirection: keep trying */
	    tok_flush(t);
	    break;
	case TOK_WORD:				/* word: keep trying */
	case TOK_COMPOSITE:			/* struct or union */
	    got_word = 1;
	    tok_flush(t);
	    break;
	default:

	    /*
	     * Function pointer types can be preceded by zero or more words
	     * (at least one when not at the top level). Other stuff can be
	     * accepted only after we have seen at least one word (two words
	     * when not at the top level). See also the above comment on
	     * structs and unions.
	     */

	    if (t->tokno == TOK_LIST && LIST_BEGINS_WITH_STAR(t)) {
		if (t = func_ptr_dcl_flush(t)) {
		    return (t);			/* reject token */
		} else {
		    got_word = 1;		/* for = and [ and , and ; */
		}
	    } else if (got_word == 0) {
		return (t);			/* reject token */
	    } else {
		switch (t->tokno) {
		case TOK_LIST:			/* function type */
		    if (t = function_dcl_flush(t))
			return (t);		/* reject token */
		    break;
		case '[':			/* dimension, does not nest */
		    pair_flush(t, '[', ']');
		    break;
		case '=':			/* initializer follows */
		    tok_flush(t);
		    initializer();		/* rewrite casts */
		    break;
		case '{':			/* struct, union, may nest */
		    block_flush(t);		/* use code for stmt blocks */
		    break;
		case ',':			/* separator: keep trying */
		    got_word = 0;
		    tok_flush(t);
		    break;
		case ';':			/* terminator: succeed */
		    tok_flush(t);
		    return (0);
		default:			/* reject token */
		    return (t);
		}
	    }
	}
    }
    return (0);					/* hit EOF */
}
