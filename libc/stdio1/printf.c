/*   fprintf.c for limited Linux libc
     Copyright (C) 1996  Joel N. Weber II <nemo@koa.iolani.honolulu.hi.us>
     
     This program is free software; you can redistribute it and/or
     modify it under the terms of the GNU General Public License
     as published by the Free Software Foundation; either version 2
     of the License, or (at your option) any later version.
     
     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.
     
     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/* Thanks Alan for writing the hard routine for me :-)
 * Alan said that this works "most of the time".  Something tells me I'm making
 * it even worse! */
/* The basic idea here is to make fprintf the core routine.  printf obviously
   can just call fprintf with stdout followed by all of its arguments.
   sprintf() works using the fake file &the_sprintf.  It's marked as fully
   buffered, so that it will only write(2) when &the_sprintf->bufpos ==
   &the_sprintf->bufend, which I doubt will happen since &the_sprintf->bufend
   = 0.  The trick is that sprintf must set &the_sprintf->bufstart =
   &the_sprintf->bufpos = its first argument.  Not as orthagonal (is that
   the right word?) as glibc's facilities for non-files, but this isn't a
   library for people who have unlimited RAM budgets.  (not like the libc
   I use on linux/i586 enjoys an unlimited RAM budget either; I only have
   8 MB

   I'm not sure what the "correct" way to pass the variable arguments
   from one function to the next is.  Rather than pass the arguments
   themselves, I'm passing a pointer to them.  However, the following
   explaination from Alan is probably a polite way of saying it will not
   work on a 386 anyway.
   Joel Weber

   [ I've migrated all of this code over to the ELKS stdarg that I wrote.
     The accepted way to do it is for the wrapper function to accept a
     variable number of arguments, use stdarg to make an argument pointer,
     and then pass the argument pointer on to the core function, as I've
     done here.  This should definitely work on a 386, as the arguments
     are still passed in the stack, and stack order is maintained. -Nat ]
 */

/*
 *	This is NOT stunningly portable, but works
 *	for pretty dumb non ANSI compilers and is
 *	tight.	Adjust the sizes to taste.
 *
 *	Illegal format strings will break it. Only the
 *	following simple subset is supported
 *
 *	%x	-	hex
 *	%d	-	decimal
 *	%s	-	string
 *	%c	-	char
 *
 *	And the h/l length specifiers for %d/%x
 *
 *		Alan Cox.
 */

#include <stdarg.h>
#include "stdio.h"

/* 17 to make sure that there's room for the trailing newline.
   I'm not really sure if this is ideal... */
static char nstring[17]="0123456789ABCDEF";

static unsigned char *
__numout(long i, int base)
{
  static unsigned char out[16];
  int n;
  int flg = 0;
  unsigned long val;

  if (i<0 && base==10)
    {
      flg = 1;
      i = -i;
    }
  val = i;

  for (n = 0; n < 15; n++)
    out[n] = ' ';
  out[15] = '\0';
  n = 14;
  do{
    out[n] = nstring[val % base];
    n--;
    val /= base;
  }
  while(val);
  if(flg) out[n--] = '-';
  return &out[n+1];
}

int
fprintf(FILE * stream, __const char * fmt, ...)
{
  va_list ap;
  int retval;
  va_start(ap, fmt);
  retval=internal_fprintf(stream, fmt, ap);
  va_end(ap);
  return(retval);
}

int
printf(__const char * fmt, ...)
{
  va_list ap;
  int retval;
  va_start(ap, fmt);
  retval=internal_fprintf(stdout, fmt, ap);
  va_end(ap);
  return(retval);
}

/* This is a strange way of doing sprintf, but it should work */
int sprintf(char * s, __const char * fmt, ...)
{
  static FILE the_sprintf = {
    -1,
    0,
    0,
    0,
    0,
    _IOFBF,
    _MODE_WRITE,
    0, 0,
    0, 0};
  va_list ap;
  int retval;

  va_start(ap, fmt);
  the_sprintf.bufstart = the_sprintf.bufpos = (unsigned char *) s;
  the_sprintf.fc_err = 0;

  retval = internal_fprintf(&the_sprintf, fmt, ap);
  /* null-terminate the string */
  putc('\0', &the_sprintf);

  va_end(ap);
  return retval;
}





/*
 * printf.c - This is the more complete fprintf() replacement for libc8086
 * Copyright (C) 1996 Steven Huang <sthuang@hns.com>.
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
 * This decides if the little test main program gets included
 */
#undef TEST
/*
 * This decides if printf() should act like standard.  When undefined,
 *  - prints out "(err)" if a bad format is encountered
 *  - supports the %b (binary) format
 */
#define STANDARD

/*
 * Shut bcc up about 'const', which doesn't seem to be handled right
 * by unproto.
 */
#ifdef __BCC__
#define const
#endif

#define BUF_SIZE        128
#define OUTC(c)         { putc(c, stream); n++; }
#define max(a, b)       ((a > b) ? a : b)
/*
 * if you change the ff, you need to change the order of characters in
 * the string 'flagstr' defined in _printf()
 */
#define FLAG_PADZERO    (1<<0)
#define FLAG_LEFTJUST   (1<<1)
#define FLAG_SIGNED     (1<<2)
#define FLAG_ALT        (1<<3)
#define FLAG_SPACE      (1<<4)

static char *utoa(unsigned int val, char *buf, int radix)
/*
 * Converts an integer to a variable-radix string representation
 *
 * Note:
 *  Does not perform sanity checking for 'radix'
 *  Assumes 'buf' has memory allocated to it
 */
{
  int divisor;
  char *p = buf;
  const char *digitstr = "0123456789abcdef";

  for (divisor = 1; val / divisor >= radix; divisor *= radix);
  do {
    *p++ = digitstr[val / divisor];
    val %= divisor;
    divisor /= radix;
  } while (divisor >= 1);
  *p = '\0';
  return(buf);
}

static
internal_fprintf(FILE * stream, const char * fmt, char *args)

/* static int _printf(FILE *stream, const char *fmt, char *args) */
/*
 * The one that does all the work.
 * This is a fairly complete implementation of printf(), supporting
 * everything EXCEPT:
 *  - floating point (eEDOUfg formats)
 *  - pointers (realizes them but doesn't understand how to print them)
 *  - short and long (h/l) modifiers (dunno what to do about them)
 * It's even slightly faster than gcc's printf() on Linux.  Can't beat
 * HP-UX's printf() though ;)
 *
 * Supports:
 *   d,   signed integer
 *   i
 *   o    unsigned octal integer
 *   u    unsigned integer
 *   x    unsigned hex lowercase a-f
 *   X    unsigned hex uppercase A-F
 *   c    character
 *   s    string (character pointer)
 *   p    void pointer (ignores it)
 *   %    % character
 *   n    number of characters output so far
 *
 * Special feature:  (no really, it's not a bug =) )
 *   b    prints an integer in binary form (i think this might come
 *        in handy *somewhere*)
 *
 *   #    alternate format for octal (leading 0) and hex (0x/0X)
 *   0    leading zeroes for d, i, o, u, x, and X
 *   -    left justify, overrides '0'
 *  ' '   (space) force a blank in front of positive numbers
 *   +    force a sign in front of any number
 *
 *   width.precision, including support for '*' (reads value from
 *   the parameter list (MUST BE INT))
 *
 *   h,   short/long modifiers, recognized but ignored.
 *   l
 *
 * Warning:
 *   The way varargs is implemented in libc is evil.  Don't think
 *   there's a better way, but misaligned or wrong parameters
 *   passed to printf() can break a lot of things.  I've tried my
 *   best to handle errors in the format string, though.
 *
 *   Each %something field cannot exceed 'BUF_SIZE' characters,
 *   which I set to 128 right now.  %s fields are not subject to
 *   this limit.
 *
 * Note:
 *   The semicolon -looks- missing in a few places but that's
 *   because of the macro def of 'OUTC'.  did it that way to
 *   save a few lines of source ;)
 *
 *   Expects a 'char *' as varargs parameter, unlike libc8086's
 *   printf(), which takes a 'int *' then casts it to a 'char *'.
 *   Either has to change, but it should be trivial.
 *
 *   This function aborts whenever it scans an illegal format, unlike
 *   gcc's libc which prints out the illegal format as if it's -not-
 *   a format string.  The 'STANDARD' preprocessor flag controls if
 *   if just aborts (when defined) or prints out "(err)" (when undefined).
 */
{
  /*
   * the "0-+# " and "dcs..." strings are listed in my idea of their
   * frequency of use, with the most popular in front.  not terribly
   * important but strchr() might have an easier time that way.
   * if you change the ordering of 'flagstr', make sure you update
   * the #define FLAG_* stuff on top of this file too.
   */
  char c, *s, *f;
  const char *flagstr = "0-+# ",
#ifdef STANDARD
    *typestr = "dcsixXuop";
#else
    *typestr = "dcsixXuopb";
#endif
  int n = 0, flags, width, actwidth, prec, bad = 0, neg, i;
  static char buf[BUF_SIZE];

  for (c = *fmt++; c && !bad;) {
    if (c != '%') {                     /* just copy */
      OUTC(c);
      c = *fmt++;
    }
    else {
      c = *fmt++;                       /* chew the % sign */
      flags = width = prec = 0;
      /*
       * Parse the "0-+# " flags
       */
      while ((f = strchr(flagstr, c)) != NULL) {
        flags |= 1 << (f - flagstr);
        c = *fmt++;
      }
      /*
       * The '*' parameter says fetch width value from varargs
       */
      if (c == '*') {
        width = *(int *) args;
        args += sizeof(int);
        if (width < 0) {
          width = abs(width);
          flags |= FLAG_LEFTJUST;       /* set '-' flag */
        }
        c = *fmt++;
      }
      else
        /*
         * scan and convert the width parameter
         */
        if (isdigit(c))
          while (isdigit(c)) {
            width *= 10;
            width += c - '0';
            c = *fmt++;
          }
      /*
       * a '.' means there may be a precision parameter
       */
      if (c == '.') {
        c = *fmt++;
        /*
         * fetch precision value from varargs
         */
        if (c == '*') {
          prec = *(int *) args;
          if (prec < 0)
            prec = 0;
          args += sizeof(int);
          c = *fmt++;
        }
        else
          /*
           * scan and convert precision field
           */
          if (isdigit(c))
            while (isdigit(c)) {
              prec *= 10;
              prec += c - '0';
              c = *fmt++;
            }
      }
      /*
       * short and long modifiers.  ignored for the moment
       */
      if (c == 'h') {
        c = *fmt++;
      }
      else
        if (c == 'l') {
          c = *fmt++;
        }
      /*
       * check if it's a valid type "dioux..."
       */
      if (strchr(typestr, c) != NULL) {
        neg = 0;
        switch (c) {
          case 'd':
          case 'i': {
            int val = *(int *) args;
            args += sizeof(int);
            neg = (val < 0);
            val = abs(val);
            actwidth = strlen(utoa(val, buf, 10)); }
            /*
             * if negative or '+'/' ' flags set
             */
            if (neg || (flags & FLAG_SIGNED) || (flags & FLAG_SPACE))
              actwidth++;
            break;
          case 'u': {
            unsigned int uval = *(unsigned int *) args;
            args += sizeof(unsigned int);
            actwidth = strlen(utoa(uval, buf, 10)); }
            /*
             * if '+'/' ' flags set
             */
            if ((flags & FLAG_SIGNED) || (flags & FLAG_SPACE))
              actwidth++;
            break;
          case 'x':
          case 'X': {
            int val = *(int *) args;
            args += sizeof(int);
            actwidth = strlen(utoa(val, buf, 16)); }
            if (flags & FLAG_ALT)
              actwidth += 2;
            break;
          case 'o': {
            int val = *(int *) args;
            args += sizeof(int);
            actwidth = strlen(utoa(val, buf, 8)); }
            if (flags & FLAG_ALT)
              actwidth++;
            break;
          case 's':
            s = *(char **) args;
            args += sizeof(char *);
            actwidth = strlen(s);
            break;
          case 'c':
            buf[0] = *(char *) args;
            buf[1] = '\0';
            args += sizeof(char);
            actwidth = 1;
            break;
          /*
           * dunno how to handle pointers - what's the format of
           * linux86 pointers?!  right now just prints "(ptr)"
           */
          case 'p':
            strcpy(buf, "(ptr)");
            args += sizeof(void *);
            actwidth = strlen(buf);
            s = buf;                    /* pretend we're a string */
            c = 's';
            break;
#ifndef STANDARD
          case 'b': {
            int val = *(int *) args;
            args += sizeof(int);
            actwidth = strlen(utoa(val, buf, 2)); }
            break;
#endif
        }
        /*
         * strings behave differently to the width.precision
         * parameters, so handle separately.  besides, we avoid
         * an extra 'memcpy' to 'buf'
         */
        if (c == 's') {
          if (prec == 0)
            prec = actwidth;
          width = max(width, prec);
          /*
           * pad to the left if not left justified
           */
          if (!(flags & FLAG_LEFTJUST)) {
            for (i = width; i > prec; i--)
              OUTC(' ');
          }
          /*
           * print out entire string if no precision specified, otherwise
           * that's our upper limit
           */
          if (prec == 0)
            for (; *s; s++)
              OUTC(*s)                  /* no semicolon here */
          else
            for (i = 0; i < prec; i++)
              OUTC(s[i]);
        }
        else {
          /*
           * precision is as wide as width if it's not specified and
           * the leading zero '0' flag is set, and left-justify is
           * -not- set.  c standard says left justify overrides the
           * leading 0.
           */
          if (prec == 0 && (flags & FLAG_PADZERO) && !(flags & FLAG_LEFTJUST))
            prec = width;
          /*
           * expand width.precision to fit the actual width.  printf
           * width specifies the -minimum-, and aside from the
           * precision of %s fields, there's no way to specify maximum
           */
          prec = max(prec, actwidth);
          width = max(width, prec);
          /*
           * pad to the left if we're not left justified
           */
          if (!(flags & FLAG_LEFTJUST)) {
            for (i = width; i > prec; i--)
              OUTC(' ');
          }
          /*
           * check if we might need to print the sign
           */
          if (strchr("diu", c) != NULL) {
            if (neg)                      /* print if negative */
              OUTC('-')                   /* yes, no ';' here =) */
            else
              if (flags & FLAG_SIGNED)    /* or '+' specified */
                OUTC('+')                 /* nor here */
              else
                if (flags & FLAG_SPACE)   /* or ' ' specified */
                  OUTC(' ')               /* nor here */
          }
          /*
           * the alternate '#' flag is set.  doesn't affect all though
           */
          if (flags & FLAG_ALT) {
            switch (c) {
              case 'o':
                OUTC('0');              /* leading zero for octals */
                break;
              case 'x':
              case 'X':                 /* prints 0x or 0X */
                OUTC('0');
                OUTC(c);
                break;
            }
          }
          /*
           * fill the precision field with either spaces or zeroes,
           * depending if we're printing numbers
           */
          if (strchr("diuxXo", c) != NULL)
            for (i = prec; i > actwidth; i--)
              OUTC('0')
          else
            for (i = prec; i > actwidth; i--)
              OUTC(' ');
          /*
           * print the field, except for 'X', which we convert to caps
           */
          if (c != 'X')
            for (f = buf; *f; f++)
              OUTC(*f)                  /* none here either */
          else
            for (f = buf; *f; f++)
              OUTC(toupper(*f));
        }
        /*
         * if we're left justified, we now need to pad spaces to the
         * right so that width will be correct
         */
        if (flags & FLAG_LEFTJUST)
          for (i = width; i > prec; i--)
            OUTC(' ');
      }
      else {
        /*
         * miscellaneous %thingies
         */
        switch (c) {
          case '%':                     /* %% -> % */
            OUTC('%');
            break;
          case 'n':                     /* %n writes current output count */
            *(*(int **) args) = n;
            args += sizeof(int *);
            break;
          default:                      /* oops, got a bad %thingy */
            bad = 1;
        }
      }
      c = *fmt++;
    }
  }
#ifndef STANDARD
  /*
   * dunno what the standard wants if the format string is badly
   * formed, so i print (err) if the debug flag is set
   */
  if (bad) {
    OUTC('(');
    OUTC('e');
    OUTC('r');
    OUTC('r');
    OUTC(')');
  }
#endif
  return(n);
}

#ifdef TEST

#include <time.h>

int main()
{
  static unsigned char xbuf[128], *x;
  char *fmt = "%s, %s %d, %.*d:%.*d\n";
  int rv1, rv2, i, dt1, dt2;
  clock_t t1, t2;

  x = xbuf;
  *(char **) x = "Sun";
  x += sizeof(char *);
  *(char **) x = "Feb";
  x += sizeof(char *);
  *(int *) x = 18;
  x += sizeof(int);
  *(int *) x = 2;
  x += sizeof(int);
  *(int *) x = 10;
  x += sizeof(int);
  *(int *) x = 2;
  x += sizeof(int);
  *(int *) x = 56;
  x += sizeof(int);
  t1 = clock();
  for (i = 0; i < 1000; i++)
    rv1 = _printf(stdout, fmt, xbuf);
  t2 = clock();
  dt1 = t2 - t1;

  t1 = clock();
  for (i = 0; i < 1000; i++)
    rv2 = printf(fmt, "Sun", "Feb", 18, 2, 10, 2, 56);
  t2 = clock();
  dt2 = t2 - t1;

  printf("\nrv1: %d, rv2: %d, dt1: %d, dt2: %d\n", rv1, rv2, dt1, dt2);
  return(0);
}

#endif
