/* type.h - types for bcc */

/* Copyright (C) 1992 Bruce Evans */

/*
  A type is essentially a "constructor", a size, and a list of pointers
  leading to a scalar type.
  The type constructors are codes for the scalar types and (), [], *,
  struct and union.
  The scalar types are char, short, int, long, float and double.
  The type lists are triply linked.

  Part of the type structure might look like

			   int		(int)
			    =
	    func <-------> int		(int ())
	     |		    =
	      --> ptr <--> int		(int *)

  (the exact structure depends on the order of declarations).
  This layout results from the pre-declared (int) and (int ()) followed by
  a declaration using (int *).
  The sideways link (from func to ptr here) allows all types leading to a
  given type to be found.
  This allows different declarations of (int *) to be recognised as the same.
  Type equivalence is equality of type pointers.
*/

/*
  flags for scalar types
  up to 3 of the flags may be set (none for constructed types)
  the 2nd and third flags can only be UNSIGNED or DLONG
  UNSIGNED only applies to integral types
  DLONG only applies to long and unsigned long types and says that these
  are actually longer than an int
*/

#define	CHAR		0x01
#define	SHORT		0x02
#define	INT		0x04
#define	LONG		0x08
#define	FLOAT		0x10
#define DOUBLE		0x20
#define UNSIGNED	0x40
#define DLONG		0x80

#define ISCALAR		(CHAR | SHORT | INT | LONG)
#define RSCALAR		(FLOAT | DOUBLE)

/*
  flags for type constructor
  at most 1 of the flags may be set (none for scalar types)
  flags are used for fast testing for array/pointer
*/

#define ARRAY		1
#define FUNCTION	2
#define POINTER		4
#define STRUCTU		8
#define VOID		0x10

/* type sizes */
/* default sizes and long and float sizes are hard-coded into type data */

extern uoffset_T ctypesize;
extern uoffset_T dtypesize;
extern uoffset_T ftypesize;
extern uoffset_T itypesize;
extern uoffset_T ptypesize;
extern uoffset_T stypesize;

/* basic scalar types */

EXTERN struct typestruct *dtype;
EXTERN struct typestruct *fltype;
EXTERN struct typestruct *itype;
EXTERN struct typestruct *ltype;
EXTERN struct typestruct *sctype;
EXTERN struct typestruct *stype;
EXTERN struct typestruct *uctype;
EXTERN struct typestruct *uitype;
EXTERN struct typestruct *ultype;
EXTERN struct typestruct *ustype;
EXTERN struct typestruct *vtype;

/* working type */

EXTERN struct typestruct *ctype;

/* constructed types */

EXTERN struct typestruct *fitype;
EXTERN struct typestruct *pctype;

/* return type of current function */

EXTERN struct typestruct *returntype;
