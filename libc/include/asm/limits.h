/* Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#ifndef __ASM_LIMITS_H
#define __ASM_LIMITS_H

/* Common defines for current processors */
#define MB_LEN_MAX	1		/* Longest multi-byte character */
#define CHAR_BIT	8		/* number of bits in a char */
#define SHRT_MAX	32767		/* maximum (signed) short value */
#define SHRT_MIN	(-32768)	/* minimum (signed) short value */
#define LONG_MAX	2147483647	/* maximum (signed) long value */
#define LONG_MIN	(-2147483648)	/* minimum (signed) long value */
#define UCHAR_MAX	255		/* maximum unsigned char value */
#define USHRT_MAX	0xffff		/* maximum unsigned short value */
#define ULONG_MAX	0xffffffff	/* maximum unsigned long value */

#ifdef __BCC__
#define CHAR_MAX	255		/* maximum char value */
#define CHAR_MIN	(0)		/* mimimum char value */

#ifdef __AS386_32__
#define INT_MAX		2147483647	/* maximum (signed) int value */
#define INT_MIN		(-2147483648)	/* minimum (signed) int value */
#define UINT_MAX	0xffffffff	/* maximum unsigned int value */
#else
#define INT_MAX		32767		/* maximum (signed) int value */
#define INT_MIN		(-32768)	/* minimum (signed) int value */
#define UINT_MAX	0xffff		/* maximum unsigned int value */
#endif

/* BCC does have signed char now. */
#define SCHAR_MAX	127		/* maximum signed char value */
#define SCHAR_MIN	(-128)		/* minimum signed char value */
#endif

#if defined(__GNUC__) && defined(__i386__)
#define CHAR_MAX	127		/* maximum char value */
#define CHAR_MIN	(-128)		/* mimimum char value */
#define SCHAR_MAX	127		/* maximum signed char value */
#define SCHAR_MIN	(-128)		/* minimum signed char value */
#define INT_MAX		2147483647	/* maximum (signed) int value */
#define INT_MIN		(-2147483648)	/* minimum (signed) int value */
#define UINT_MAX	0xffffffff	/* maximum unsigned int value */
#endif

#ifndef RAND_MAX
#define RAND_MAX	INT_MAX
#endif

#ifndef INT_MAX
#error "Limits.h not fully implemented, INT_MAX undefined!"
#endif

#endif
