/* extern.c - external declarations (K & R p218-9) */

#include "def.h"
#include "globvar.h"
#include "symbol.h"

struct typestruct *declarator();
struct typestruct *extypespec();
int paramlist();
struct typestruct *typespec();

/*=============================================================================
	-- notation is 1st attempt at EBNF (see Modula-2 book)
	program = { type-definition | function-or-data-definition }.
	type-definition = "typedef" type-definition-list.
=============================================================================*/

program()
{
	nextsym();
	while ( sym != EOFSYM )
	{
		if ( sym = TYPEDEFDECL )
			deftypes();
		else
			functordata();
	}
}

/*=============================================================================
	type-definition-list. -- not implemented
=============================================================================*/

deftypes()
{}

/*=============================================================================
	function-or-data-definition = external-type-specification
								  ( function-definition | init-declarators ).
	function-definition = "(" parameter-list ")" arg-list compound-statement.
	-- this is different from K & R who omit typedefs and static functions
=============================================================================*/

functordata()
{
	struct typestruct *type;
	int nargs;

	type = extypespec();
	if ( sym == LPAREN )
	{
		nextsym();
		nargs = paramlist();
		need( RPAREN );
		arglist( nargs );
		need( LBRACE );
		compound( 1 + locptr-pstartloc );/* 1 = function flag, rest = nargs */
		oldlevel();
	}
	else
		initdecl( type );
}

/*=============================================================================
	external-type-specification = extern-sc type-specifier declarator.
	extern-sc = [ extern | static ].
=============================================================================*/

struct typestruct *extypespec()
{
	int sc;

	switch( sym )
	{
	case EXTERNDECL:
	case STATICDECL:
		sc = sym; nextsym(); break;
	default:
		sc = EXTERNDECL; break;
	}
	return declarator( typespec() );
}

/*=============================================================================
	parameter-list = [identifier { "," identifier }].
=============================================================================*/

int paramlist()
{
	int nargs;

	locptr = pstartloc;
	newlevel();
	nargs = 0;
	while ( sym == IDENT )
	{
		addloc( gsname, itype, 0 );	/* fix up types and offsets later */
		nextsym();
		++nargs;
		if ( sym != COMMA )
			break;
		nextsym();
	}
	return nargs;
}

/*=============================================================================
	arg-list =
=============================================================================*/

arglist( nargs )
int nargs;
{
	struct symstruct *symptr;
	int argsize, argsp;
	int lastsym;
	struct typestruct *basetype;
	char declflag;

	declflag = TRUE;
	do
	{
		switch( sym )
		{
		case TYPEDECL:
			basetype = gsymptr->type;
			nextsym();
			getarg1( basetype );
			break;
		case STRUCTDECL:
		case UNIONDECL:
			lastsym = sym;
			nextsym();
			getarg( declsu( lastsym ) );
			break;
		case AUTODECL:
		case EXTERNDECL:
		case REGDECL:
		case STATICDECL:
		case SEMICOLON:
			nextsym();
			break;
		default:
			declflag = FALSE;
			break;
		}
	}
	while ( declflag );
	argsp = -2;
	symptr = pstartloc;
	while ( nargs && symptr < locptr )	/* convert arg sizes to offsets */
	{
		argsize = symptr->offset;
		if ( symptr->type == ctype )
		{
			++argsp;
			--argsize;
		}
		symptr->offset = argsp;
		argsp += argsize;
		if ( symptr == pstartloc )
			argsp += 2;
		--nargs;
		++symptr;
	}
	if ( nargs )
		error( "arguments not all declared" );
}
	
/* getarg( basetype ) - fill in argument types and sizes */

getarg1( basetype )
struct typestruct *basetype;
{
	char sname[NAMESIZE];
	struct typestruct *type;
	int size;
	struct symstruct *symptr;

	if ( sym != SEMICOLON )
	while ( TRUE )
	{
		type = basetype;
		size = getvar( sname, &type );
		if ( (symptr = findlorg( sname )) == NULL || symptr->level != ARGLEVEL )
			error( "variable not in argument list" );
		else if ( symptr->offset != 0 ) /* already in arg list */
			multidecl( sname, type, symptr );
		else
		{
			if ( size < 2 || type->typeop == ATYPEOP )
				size = 2;
			symptr->offset = size;
			symptr->type = type;
		}
		if ( sym == COMMA )
			nextsym();
		else
			break;
	}
	ns();
}

struct typestruct *declarator( basetype )
struct typestruct *basetype;
{
	char sname[NAMESIZE];
	int size;
	struct symstruct *multi;

	size = getvar( sname, &type );
	if ( multi = findlorg( sname ) )
		multidecl( sname, type, multi );
	else	
		addglb( sname, type, size );
	nextsym();
}

struct typestruct *typespec()
{
	int lastsym;

	switch( sym )
	{
	case TYPEDECL:
		nextsym();
		return gsymptr->type;
	case STRUCTDECL:
	case UNIONDECL:
		lastsym = sym;
		nextsym();
		return declsu( lastsym );
	default:
		return itype;
	}
}
