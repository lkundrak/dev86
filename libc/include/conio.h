
#ifndef __CONIO_H
#define __CONIO_H
#include <features.h>

int cprintf __P((char *, ...));
int cputs __P((char *));
int getch __P((void));
int getche __P((void));
int kbhit __P((void));
int putch __P((int));
int gotoxy __P((int, int));

#if 0 /* Unimplemented as yet */
char * cgets __P((char *));
int ungetch __P((int));
int cscanf __P((char *, ...));
#endif

#if 0 /* Hummm */
unsigned outpw __P((unsigned int, unsigned int));
unsigned inpw __P((unsigned int));
int outp __P((unsigned int, int));
int inp __P((unsigned int));
#endif

#endif
