/*
 * debug.c: a generic debugging facility for unix programs.
 *
 * The calling program is required to call debug_setlevel(lvl) to set
 * which messages will be displayed. The level is a two part value
 * where the least significant (decimal) digit is a level as described
 * below. The most significant digits are a class code. For a message
 * to be displayed the class code must either be zero or match the
 * class code of the debug(...) message.
 *
 * The 'debug(lvl, fmt, ...)' function displays debugging messages 
 * complete with source and line number. The function can be used
 * as a normal one in if() smt else smt constructs. It returns the
 * actual number of bytes printed so it's return value can be used
 * inside an if(debug(...)) to enable more debugging code. This code
 * will be removed by the compiler (as dead code) if debugging is
 * not enabled.
 *
 * The level on the debug() statment also consists of a level and class
 * code where the class code must be zero or match the setlevel's class
 * code for the message to be displayed.
 *
 * Level 0
 *    Always displayed if the debugging is enabled.
 *    You probably shouldn't use this.
 *
 * Level 1
 *    Important state changes and errors that cause a significant change
 *    in program flow.
 *
 * Level 2
 *    Rare things that cause a minor program flow adjustment.
 *
 * Level 3
 *    Errors and useful messages that are slightly too verbose or common
 *    for 0-2 or don't quite fit in the classifications.
 *
 * Level 4
 *    All remote responses or major results. (Trace results)
 *
 * Level 5
 *    All remote commands or major tasks. (Trace jobs)
 *
 * Level 6
 *    General information that will not be too verbose but is normally a
 *    little less important. (Trace state)
 *
 * Level 7
 *    Similar to level 3 but verbose or not as useful.
 *
 * Level 8
 *    Very verbose information that'll probably be useful sometime.
 *
 * Level 9
 *    Anything and everything else, debugs that probably won't be useful
 *    ever again.                                            (unclassified)
 *
 * Notes:
 *    If the programmer doesn't set the debug level this is not an important
 *    debug message or is only important right now.
 *    => default debug level == 9
 *
 *    If something fits in one of the lower levels but is very verbose
 *    it should nevertheless be moved upto level 3 or levels 7-9.
 *    (Possibly leaving a single line 'oops' at the lower level)
 *
 *    The general idea is that debug levels 0-3 should not scroll too fast
 *    to read and nothing below level 7 should be much more verbose than
 *    levels 4 or 5.
 *
 *****************************************************************************
 *
 * 2004-06-20: Added __STDC__ to debug.h so it can be called from non-ansi
 *             compiler. This file still needs ansi or unproto.
 *
 * 2004-06-20: Added check of DEBUG environment variable if setlevel isn't
 *             called before a debug().
 *
 * 2004-06-20: Added #define VARARG_MACROS so the preprocessor can remove
 *             all the debugging 'stuff'.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "debug.h"

#define USE_DBPRINTF

#ifndef DEBUG
static char ident[] = 
   "$Id: debug.c: (c) 1995-2004 Robert de Bath. Debugging disabled. $";
#else
static char ident[] = 
   "$Id: debug.c: (c) 1995-2004 Robert de Bath. Debugging enabled. $";

static char * db_file = 0;
static int db_lineno = 0;
static int disp_state = 0;
static int disp_pos(void);

static void debug_envvar(void);

int debug_level = -1;

void debug_do_setlevel(char * fname, int lineno, int level) 
{
   if(level || !debug_level) 
      debug_level = level;
   debug_pos(fname, lineno);
   debug_msg(1, "Debug level now %d", level);
   debug_level = level;
}

int debug_pos(char * file, int lineno)
{
   db_file = file;
   db_lineno = lineno;
   disp_state |= 1;
   return disp_pos();
}

int debug_msg(int level, char * fmt, ...)
{
   va_list ap;
   int rv = 0;
   int disp = 0;

   if (debug_level == -1) debug_envvar();

   if (level == -1) {
      level = 0;
      disp_state |= 1;
      db_lineno = -1;
   }

   disp_state |= 2;

   if (level>9 || debug_level>9) {
      disp = (level%10 <= debug_level%10);
      if (disp && level>9 && debug_level>9 && level/10 != debug_level/10)
	 disp = 0;
   } else disp = (level <= debug_level);

   if (disp)
   {
      disp_state |= 4;

      va_start(ap, fmt);
#ifdef USE_DBPRINTF
      rv = vdbprintf(fmt, ap);
#else
      rv = vfprintf(stderr, fmt, ap);
#endif
      va_end(ap);
   }
   return rv + disp_pos();
}

int
disp_pos()
{
   int rv = 0;
   if (disp_state == 7 && db_lineno != -1)
#ifdef USE_DBPRINTF
      rv = dbprintf(" at %s:%d\n", db_file, db_lineno);
#else
      rv = fprintf(stderr, " at %s:%d\n", db_file, db_lineno);
#endif

   if ((disp_state&3) == 3) {
      db_file = 0;
      db_lineno = disp_state = 0;
   }
   return rv;
}

/* If setlevel isn't called check the environment */

static void debug_envvar(void)
{
   char * p = getenv("DEBUG");
   if (!p || !*p)
      debug_level = 0;
   else 
      debug_level = atoi(p);
   if (debug_level)
#ifdef USE_DBPRINTF
      dbprintf("Debug level now %d from environment.\n", debug_level);
#else
      fprintf(stderr, "Debug level now %d from environment.\n", debug_level);
#endif
}

#endif

#ifndef VARARG_MACROS
/* 
 * This function should never be called.
 *
 * If ident sees the message in a binary then your compiler is wasting
 * space by allocating it for unused strings.
 *
 * We know GNU-C is ok, but it complains.
 */
int debug_never(int level, char * name, ...)
{
#ifndef __GNUC__
   1?0:debug_never(0, "$Warning: Debugging strings exist in non-debug binary $");
#endif
   return 0;
}
#endif

