/* Copyright (C) 1997,1998,1999,2000,2001,2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* 2015-03-08: Modified for Dev86 by Jody Bruchon <jody@jodybruchon.com> */

/*
 *     ISO C99: 7.18 Integer types <stdint.h>
 */

#ifndef _STDINT_H
#define _STDINT_H      1

#include <features.h>

/* Exact integral types.  */

/* Signed.  */

/* There is some amount of overlap with <sys/types.h> as known by inet code */
#ifndef __int8_t_defined
# define __int8_t_defined
typedef signed char            int8_t;
typedef int                    int16_t;
typedef long int               int32_t;
#endif

/* Unsigned.  */
typedef unsigned char          uint8_t;
typedef unsigned int           uint16_t;
#ifndef __uint32_t_defined
typedef unsigned long          uint32_t;
# define __uint32_t_defined
#endif


/* Small types.  */

/* Signed.  */
typedef signed char            int_least8_t;
typedef int                    int_least16_t;
typedef long                   int_least32_t;

/* Unsigned.  */
typedef unsigned char          uint_least8_t;
typedef unsigned int           uint_least16_t;
typedef unsigned long          uint_least32_t;

/* Fast types.  */

/* Signed.  */
typedef signed char            int_fast8_t;
typedef int                    int_fast16_t;
typedef long                   int_fast32_t;

/* Unsigned.  */
typedef unsigned char          uint_fast8_t;
typedef unsigned int           uint_fast16_t;
typedef unsigned long          uint_fast32_t;


/* Types for `void *' pointers.  */
#ifndef __intptr_t_defined
typedef int                    intptr_t;
#define __intptr_t_defined
#endif
typedef unsigned int           uintptr_t;


/* Largest integral types.  */
typedef long int               intmax_t;
typedef unsigned long int      uintmax_t;


/* The ISO C99 standard specifies that in C++ implementations these
   macros should only be defined if explicitly requested.  */
#if !defined __cplusplus || defined __STDC_LIMIT_MACROS

/* Limits of integral types.  */

/* Minimum of signed integral types.  */
# define INT8_MIN              (-128)
# define INT16_MIN             (-32767-1)
# define INT32_MIN             (-2147483647-1)
/* Maximum of signed integral types.  */
# define INT8_MAX              (127)
# define INT16_MAX             (32767)
# define INT32_MAX             (2147483647)

/* Maximum of unsigned integral types.  */
# define UINT8_MAX             (255)
# define UINT16_MAX            (65535)
# define UINT32_MAX            (4294967295U)

/* Minimum of signed integral types having a minimum size.  */
# define INT_LEAST8_MIN                (-128)
# define INT_LEAST16_MIN       (-32767-1)
# define INT_LEAST32_MIN       (-2147483647-1)
/* Maximum of signed integral types having a minimum size.  */
# define INT_LEAST8_MAX                (127)
# define INT_LEAST16_MAX       (32767)
# define INT_LEAST32_MAX       (2147483647)

/* Maximum of unsigned integral types having a minimum size.  */
# define UINT_LEAST8_MAX       (255)
# define UINT_LEAST16_MAX      (65535)
# define UINT_LEAST32_MAX      (4294967295U)

/* Minimum of fast signed integral types having a minimum size.  */
# define INT_FAST8_MIN         (-128)
# define INT_FAST16_MIN                (-32767-1)
# define INT_FAST32_MIN                (-2147483647-1)
/* Maximum of fast signed integral types having a minimum size.  */
# define INT_FAST8_MAX         (127)
# define INT_FAST16_MAX                (32767)
# define INT_FAST32_MAX                (2147483647)

/* Maximum of fast unsigned integral types having a minimum size.  */
# define UINT_FAST8_MAX                (255)
# define UINT_FAST16_MAX       (65535U)
# define UINT_FAST32_MAX       (4294967295U)

/* Values to test for integral types holding `void *' pointer.  */
# define INTPTR_MIN            (-32767-1)
# define INTPTR_MAX            (32767)
# define UINTPTR_MAX           (65535U)


/* Minimum for largest signed integral type.  */
# define INTMAX_MIN            (-2147483647-1)
/* Maximum for largest signed integral type.  */
# define INTMAX_MAX            (2147483647)

/* Maximum for largest unsigned integral type.  */
# define UINTMAX_MAX           (4294967295)

/* Limits of other integer types.  */

/* Limits of `ptrdiff_t' type.  */
# define PTRDIFF_MIN           (-32767-1)
# define PTRDIFF_MAX           (32767)

/* Limits of `sig_atomic_t'.  */
# define SIG_ATOMIC_MIN                (-32767-1)
# define SIG_ATOMIC_MAX                (32767)

/* Limit of `size_t' type.  */
/* FIXME: Linux target has a 32-bit size_t but who really uses that target? */
# define SIZE_MAX              (65535U)

#endif /* C++ && limit macros */


/* The ISO C99 standard specifies that in C++ implementations these
   should only be defined if explicitly requested.  */
#if !defined __cplusplus || defined __STDC_CONSTANT_MACROS

/* Signed.  */
# define INT8_C(c)     c
# define INT16_C(c)    c
# define INT32_C(c)    c
# define INT64_C(c)    c ## LL

/* Unsigned.  */
# define UINT8_C(c)    c
# define UINT16_C(c)   c
# define UINT32_C(c)   c ## U
# define UINT64_C(c)   c ## ULL

/* Maximal type.  */
# define INTMAX_C(c)   c ## LL
# define UINTMAX_C(c)  c ## ULL

#endif /* C++ && constant macros */

#endif /* stdint.h */
