
#ifndef _DEBUG_H_
#define _DEBUG_H_

#if __STDC__
void debug_do_setlevel(char * fname, int lineno, int level);
int debug_pos(char * file, int lineno);
int debug_msg(int level, char * name, ...);
int debug_never(int level, char * name, ...);
#else
void debug_do_setlevel();
int debug_pos();
int debug_msg();
int debug_never();
#endif /* __STDC__ */

/* The new CPP has these. */
#if defined(__BCC__) && (__BCC_VERSION__ >= 0x001011L)
#define VARARG_MACROS
#endif

#ifdef DEBUG

extern int debug_level;
#define debug !debug_level?0:debug_pos(__FILE__,__LINE__)+debug_msg
#define debug_setlevel(lvl) debug_do_setlevel(__FILE__, __LINE__, lvl)

#else /* !DEBUG */

#ifdef VARARG_MACROS
# define debug(junk ...) 0
#else
# define debug	1?0:debug_never
#endif
# define debug_setlevel(lvl)

#endif /* DEBUG */
#endif /* _DEBUG_H_ */
