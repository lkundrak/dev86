
#ifndef _P
#if __STDC__
#define _P(x) x
#else
#define _P(x) ()
#endif
#endif

extern void cfatal _P((char*));
extern void cerror _P((char*));
extern void cwarn _P((char*));
extern FILE * open_include _P((char*, char*, int));

extern FILE * curfile;
extern char   curword[];
extern char * c_fname;
extern int    c_lineno;
extern int    in_asm;
extern int    ansi_c;

extern int gettok _P((void));

struct token_trans { char * name; int token; };
struct token_trans * is_ctok _P((const char *str, unsigned int len));
struct token_trans * is_ckey _P((const char *str, unsigned int len));

#define WORDSIZE 128
#define TK_WORD  256
#define TK_NUM   257
#define TK_FLT   258
#define TK_QUOT  259
#define TK_STR   260
#define TK_FILE  261
#define TK_LINE  262
#define TK_COPY  263

#define TK_CTOK  0x200
#define TK_CKEY  0x300

#define TK_NE_OP	(TK_CTOK+ 0)
#define TK_MOD_ASSIGN	(TK_CTOK+ 1)
#define TK_AND_OP	(TK_CTOK+ 2)
#define TK_AND_ASSIGN	(TK_CTOK+ 3)
#define TK_MUL_ASSIGN	(TK_CTOK+ 4)
#define TK_INC_OP	(TK_CTOK+ 5)
#define TK_ADD_ASSIGN	(TK_CTOK+ 6)
#define TK_DEC_OP	(TK_CTOK+ 7)
#define TK_SUB_ASSIGN	(TK_CTOK+ 8)
#define TK_PTR_OP	(TK_CTOK+ 9)
#define TK_ELLIPSIS	(TK_CTOK+10)
#define TK_DIV_ASSIGN	(TK_CTOK+11)
#define TK_LEFT_OP	(TK_CTOK+12)
#define TK_LEFT_ASSIGN	(TK_CTOK+13)
#define TK_LE_OP	(TK_CTOK+14)
#define TK_EQ_OP	(TK_CTOK+15)
#define TK_GE_OP	(TK_CTOK+16)
#define TK_RIGHT_OP	(TK_CTOK+17)
#define TK_RIGHT_ASSIGN	(TK_CTOK+18)
#define TK_XOR_ASSIGN	(TK_CTOK+19)
#define TK_OR_ASSIGN	(TK_CTOK+20)
#define TK_OR_OP	(TK_CTOK+21)

#define TK_AUTO		(TK_CKEY+ 0)
#define TK_BREAK	(TK_CKEY+ 1)
#define TK_CASE		(TK_CKEY+ 2)
#define TK_CHAR		(TK_CKEY+ 3)
#define TK_CONST	(TK_CKEY+ 4)
#define TK_CONTINUE	(TK_CKEY+ 5)
#define TK_DEFAULT	(TK_CKEY+ 6)
#define TK_DO		(TK_CKEY+ 7)
#define TK_DOUBLE	(TK_CKEY+ 8)
#define TK_ELSE		(TK_CKEY+ 9)
#define TK_ENUM		(TK_CKEY+10)
#define TK_EXTERN	(TK_CKEY+11)
#define TK_FLOAT	(TK_CKEY+12)
#define TK_FOR		(TK_CKEY+13)
#define TK_GOTO		(TK_CKEY+14)
#define TK_IF		(TK_CKEY+15)
#define TK_INT		(TK_CKEY+16)
#define TK_LONG		(TK_CKEY+17)
#define TK_REGISTER	(TK_CKEY+18)
#define TK_RETURN	(TK_CKEY+19)
#define TK_SHORT	(TK_CKEY+20)
#define TK_SIGNED	(TK_CKEY+21)
#define TK_SIZEOF	(TK_CKEY+22)
#define TK_STATIC	(TK_CKEY+23)
#define TK_STRUCT	(TK_CKEY+24)
#define TK_SWITCH	(TK_CKEY+25)
#define TK_TYPEDEF	(TK_CKEY+26)
#define TK_UNION	(TK_CKEY+27)
#define TK_UNSIGNED	(TK_CKEY+28)
#define TK_VOID		(TK_CKEY+29)
#define TK_VOLATILE	(TK_CKEY+30)
#define TK_WHILE	(TK_CKEY+31)

#define MAX_INCLUDE 16	/* Nested includes */
#define MAX_DEFINE  32	/* Nested defines */

extern char * set_entry _P((int,char*,void*));
extern void * read_entry _P((int,char*));

struct define_item
{
   struct define_arg * next;
   char * name;
   int arg_count;	/* -1 = none; >=0 = brackets with N args */
   int in_use;		/* Skip this one for looking up #defines */
   char value[1];	/* [arg,]*value */
};
