/* ctype.h */

#define _C  1		/* control */
#define _D  2		/* digit */
#define _L  4		/* lower */
#define _P  8		/* punctuation */
#define _S  16		/* space */
#define _U  32		/* upper */
#define _UN 64		/* underline */
#define _X  '\200'	/* hex digit, not digit */

extern char _ctype_[];

#define	_ct1_ (_ctype_+1)

#define isalnum(c)  (_ct1_[c]&(_U|_L|_D))
#define isalpha(c)  (_ct1_[c]&(_U|_L))
#define isascii(i)  ((unsigned)(i)<=0x7f)
#define isalpha(c)  (_ct1_[c]&(_U|_L))
#define iscntrl(c)  (_ct1_[c]&_C)
#define iscsym(c)   (_ct1_[c]&(_U|_L|_D|_UN))
#define iscsymf(c)  (_ct1_[c]&(_U|_L|_UN))
#define isdigit(c)  (_ct1_[c]&_D)
#define isgraph(c)  (_ct1_[c]&(_U|_L|_D|_P))
#define islower(c)  (_ct1_[c]&_L)
/*      isodigit(i) is a function */
/*      isprint(i)  is a function */
#define ispunct(c)  (_ct1_[c]&_P)
#define isspace(c)  (_ct1_[c]&_S)
#define isupper(c)  (_ct1_[c]&_U)
#define isxdigit(c) (_ct1_[c]&(_D|_X))

#define toascii(i)  ((i)&0x7f)
/*      toint(i)    is a function */
/*      tolower(i)  is a function */
#define _tolower(c) ((c)+('a'-'A'))
/*      toupper(i)  is a function */
#define _toupper(c) ((c)+('A'-'a'))
