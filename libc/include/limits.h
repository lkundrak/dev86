/* Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

#ifndef __LIMITS_H
#define __LIMITS_H

#if __AS386_16__
#define MB_LEN_MAX	1		/* Longest multi-byte character */
#define CHAR_MAX	127		/* maximum char value */
#define CHAR_MIN	(-127)		/* mimimum char value */
#define SCHAR_MAX	127		/* maximum signed char value */
#define SCHAR_MIN	(-127)		/* minimum signed char value */
#define CHAR_BIT	8		/* number of bits in a char */
#define SHRT_MAX	32767		/* maximum (signed) short value */
#define SHRT_MIN	(-32767)	/* minimum (signed) short value */
#define INT_MAX		32767		/* maximum (signed) int value */
#define INT_MIN		(-32767)	/* minimum (signed) int value */
#define LONG_MAX	2147483647	/* maximum (signed) long value */
#define LONG_MIN	(-2147483647)	/* minimum (signed) long value */
#define UCHAR_MAX	255		/* maximum unsigned char value */
#define USHRT_MAX	0xffff		/* maximum unsigned short value */
#define UINT_MAX	0xffff		/* maximum unsigned int value */
#define ULONG_MAX	0xffffffff	/* maximum unsigned long value */
#ifndef RAND_MAX
#define RAND_MAX	INT_MAX
#endif
#endif

#if __AS386_32__
#define MB_LEN_MAX	1		/* Longest multi-byte character */
#define CHAR_MAX	127		/* maximum char value */
#define CHAR_MIN	(-127)		/* mimimum char value */
#define SCHAR_MAX	127		/* maximum signed char value */
#define SCHAR_MIN	(-127)		/* minimum signed char value */
#define CHAR_BIT	8		/* number of bits in a char */
#define SHRT_MAX	32767		/* maximum (signed) short value */
#define SHRT_MIN	(-32767)	/* minimum (signed) short value */
#define INT_MAX		2147483647	/* maximum (signed) int value */
#define INT_MIN		(-2147483647)	/* minimum (signed) int value */
#define LONG_MAX	2147483647	/* maximum (signed) long value */
#define LONG_MIN	(-2147483647)	/* minimum (signed) long value */
#define UCHAR_MAX	255		/* maximum unsigned char value */
#define USHRT_MAX	0xffff		/* maximum unsigned short value */
#define UINT_MAX	0xffffffff	/* maximum unsigned int value */
#define ULONG_MAX	0xffffffff	/* maximum unsigned long value */
#ifndef RAND_MAX
#define RAND_MAX	INT_MAX
#endif
#endif

#ifndef CHAR_MAX
#error "Limits.h not implemented"
#endif

#endif
