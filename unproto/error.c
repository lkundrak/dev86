/*++
/* NAME
/*	error 3
/* SUMMARY
/*	diagnostics
/* PACKAGE
/*	unproto
/* SYNOPSIS
/*	#include "error.h"
/*
/*	void error(quit, text)
/*	int quit;
/*	char *text;
/*
/*	void error_where(quit, path, line, text)
/*	int quit;
/*	char *path;
/*	int line;
/*	char *text;
/* DESCRIPTION
/*	The routines in this file print a diagnostic (text) and optionally
/*	terminate the program (quit != 0) with exit status "quit".
/*
/*	error() provides a default context, i.e. the source-file
/*	coordinate of the last read token.
/*
/*	error_where() allows the caller to explicitly specify context: path
/*	is a source-file name, and line is a line number.
/*
/*	context is ignored if the line number is zero or if the path
/*	is an empty string.
/* AUTHOR(S)
/*	Wietse Venema
/*	Eindhoven University of Technology
/*	Department of Mathematics and Computer Science
/*	Den Dolech 2, P.O. Box 513, 5600 MB Eindhoven, The Netherlands
/* LAST MODIFICATION
/*	91/11/30 21:10:35
/* VERSION/RELEASE
/*	1.1
/*--*/

static char error_sccsid[] = "@(#) error.c 1.1 91/11/30 21:10:35";

/* C library */

#include <stdio.h>

void exit();

/* Application-specific stuff */

#include "token.h"
#include "error.h"

/* error - report problem (implicit context) and optionally quit */

void    error(quit, text)
int     quit;
char   *text;
{
    error_where(quit, curr_path, curr_line, text);
}

/* error_where - report problem (explicit context) and optionally quit */

void    error_where(quit, path, line, text)
int     quit;
char   *path;
int     line;
char   *text;
{
    if (line && path[0])
	fprintf(stderr, "%s, line %d: ", path, line);
    fprintf(stderr, "%s\n", text);
    if (quit)
	exit(quit);
}
