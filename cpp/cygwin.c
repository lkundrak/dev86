/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/*
 *  CTYPE	Character classification and conversion
 */

/*
 * I've copied this here from the bcc libs because cygwin's version has
 * the _very_ silly feature of using _P as a private library constant.
 *
 * Single underscore variables are generally used for private variables 
 * in user libraries; the few stdio ones being 'leftovers' from version7
 * where user and system libraries were one and the same.
 *
 */
#ifndef __CTYPE_H
#define __CTYPE_H

static	unsigned char	__ctype[];

#define	__CT_d	0x01		/* numeric digit */
#define	__CT_u	0x02		/* upper case */
#define	__CT_l	0x04		/* lower case */
#define	__CT_c	0x08		/* control character */
#define	__CT_s	0x10		/* whitespace */
#define	__CT_p	0x20		/* punctuation */
#define	__CT_x	0x40		/* hexadecimal */

/* Define these as simple old style ascii versions */
#define	toupper(c)	(((c)>='a'&&(c)<='z') ? (c)^0x20 : (c))
#define	tolower(c)	(((c)>='A'&&(c)<='Z') ? (c)^0x20 : (c))
#define	_toupper(c)	((c)^0x20)
#define	_tolower(c)	((c)^0x20)
#define	toascii(c)	((c)&0x7F)

#define __CT(c) (__ctype[1+(unsigned char)c])

/* Note the '!!' is a cast to 'bool' and even BCC deletes it in an if()  */
#define	isalnum(c)	(!!(__CT(c)&(__CT_u|__CT_l|__CT_d)))
#define	isalpha(c)	(!!(__CT(c)&(__CT_u|__CT_l)))
#define	isascii(c)	(!((c)&~0x7F))
#define	iscntrl(c)	(!!(__CT(c)&__CT_c))
#define	isdigit(c)	(!!(__CT(c)&__CT_d))
#define	isgraph(c)	(!(__CT(c)&(__CT_c|__CT_s)))
#define	islower(c)	(!!(__CT(c)&__CT_l))
#define	isprint(c)	(!(__CT(c)&__CT_c))
#define	ispunct(c)	(!!(__CT(c)&__CT_p))
#define	isspace(c)	(!!(__CT(c)&__CT_s))
#define	isupper(c)	(!!(__CT(c)&__CT_u))
#define	isxdigit(c)	(!!(__CT(c)&__CT_x))

static unsigned char __ctype[257] =
{
   0,							/* -1 */
   __CT_c, __CT_c, __CT_c, __CT_c,			/* 0x00..0x03 */
   __CT_c, __CT_c, __CT_c, __CT_c,			/* 0x04..0x07 */
   __CT_c, __CT_c|__CT_s, __CT_c|__CT_s, __CT_c|__CT_s,	/* 0x08..0x0B */
   __CT_c|__CT_s, __CT_c|__CT_s, __CT_c, __CT_c,	/* 0x0C..0x0F */

   __CT_c, __CT_c, __CT_c, __CT_c,			/* 0x10..0x13 */
   __CT_c, __CT_c, __CT_c, __CT_c,			/* 0x14..0x17 */
   __CT_c, __CT_c, __CT_c, __CT_c,			/* 0x18..0x1B */
   __CT_c, __CT_c, __CT_c, __CT_c,			/* 0x1C..0x1F */

   __CT_s, __CT_p, __CT_p, __CT_p,			/* 0x20..0x23 */
   __CT_p, __CT_p, __CT_p, __CT_p,			/* 0x24..0x27 */
   __CT_p, __CT_p, __CT_p, __CT_p,			/* 0x28..0x2B */
   __CT_p, __CT_p, __CT_p, __CT_p,			/* 0x2C..0x2F */

   __CT_d|__CT_x, __CT_d|__CT_x, __CT_d|__CT_x, __CT_d|__CT_x,/* 0x30..0x33 */
   __CT_d|__CT_x, __CT_d|__CT_x, __CT_d|__CT_x, __CT_d|__CT_x,/* 0x34..0x37 */
   __CT_d|__CT_x, __CT_d|__CT_x, __CT_p, __CT_p,	/* 0x38..0x3B */
   __CT_p, __CT_p, __CT_p, __CT_p,			/* 0x3C..0x3F */

   __CT_p, __CT_u|__CT_x, __CT_u|__CT_x, __CT_u|__CT_x,	/* 0x40..0x43 */
   __CT_u|__CT_x, __CT_u|__CT_x, __CT_u|__CT_x, __CT_u,	/* 0x44..0x47 */
   __CT_u, __CT_u, __CT_u, __CT_u,			/* 0x48..0x4B */
   __CT_u, __CT_u, __CT_u, __CT_u,			/* 0x4C..0x4F */

   __CT_u, __CT_u, __CT_u, __CT_u,			/* 0x50..0x53 */
   __CT_u, __CT_u, __CT_u, __CT_u,			/* 0x54..0x57 */
   __CT_u, __CT_u, __CT_u, __CT_p,			/* 0x58..0x5B */
   __CT_p, __CT_p, __CT_p, __CT_p,			/* 0x5C..0x5F */

   __CT_p, __CT_l|__CT_x, __CT_l|__CT_x, __CT_l|__CT_x,		/* 0x60..0x63 */
   __CT_l|__CT_x, __CT_l|__CT_x, __CT_l|__CT_x, __CT_l,		/* 0x64..0x67 */
   __CT_l, __CT_l, __CT_l, __CT_l,				/* 0x68..0x6B */
   __CT_l, __CT_l, __CT_l, __CT_l,				/* 0x6C..0x6F */

   __CT_l, __CT_l, __CT_l, __CT_l,				/* 0x70..0x73 */
   __CT_l, __CT_l, __CT_l, __CT_l,				/* 0x74..0x77 */
   __CT_l, __CT_l, __CT_l, __CT_p,				/* 0x78..0x7B */
   __CT_p, __CT_p, __CT_p, __CT_c				/* 0x7C..0x7F */
};

#endif /* __CTYPE_H */
