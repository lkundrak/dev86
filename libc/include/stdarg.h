 /*
  * @(#) stdarg.h 1.2 91/11/30 21:10:39
  * 
  * Sample stdarg.h file for use with the unproto filter.
  * 
  * This file serves two purposes.
  * 
  * 1 - As an include file for use with ANSI-style C source that implements
  * variadic functions.
  * 
  * 2 - To configure the unproto filter itself. If the _VA_ALIST_ macro is
  * defined, its value will appear in the place of the "..." in argument
  * lists of variadic function *definitions* (not declarations).
  * 
  * Compilers that pass arguments via the stack can use the default code at the
  * end of this file (this usually applies for the VAX, MC68k and 80*86
  * architectures).
  * 
  * RISC-based systems often need special tricks. An example of the latter is
  * given for the SPARC architecture. Read your /usr/include/varargs.h for
  * more information.
  * 
  * You can use the varargs.c program provided with the unproto package to
  * verify that the stdarg.h file has been set up correctly.
  */

#ifndef __STDARG_H
#define __STDARG_H

#ifdef sparc
#  define _VA_ALIST_		"__builtin_va_alist"
   typedef char *va_list;
#  define va_start(ap, p)	(ap = (char *) &__builtin_va_alist)
#  define va_arg(ap, type)	((type *) __builtin_va_arg_incr((type *) ap))[0]
#  define va_end(ap)
#else /* vax, mc68k, 80*86 */
   typedef char *va_list;
#  define va_start(ap, p)	(ap = (char *) (&(p)+1))
#  define va_arg(ap, type)	((type *) (ap += sizeof(type)))[-1]
#  define va_end(ap)
#endif

#endif /* __STDARG_H */

#if __FIRST_ARG_IN_AX__
#error First arg is in a register, stdarg.h cannot take its address
#endif
