
#ifndef __DOS_H
#define __DOS_H
#include <features.h>

union REGS
{
   struct { unsigned int ax, bx, cx, dx, si, di, cflag; } x;
   struct { unsigned char al, ah, bl, bh, cl, ch, dl, dh; } h;
};

struct SREGS
{
   unsigned int es, cs, ss, ds;
};

#ifdef __MSDOS__
extern unsigned int __envseg;
extern unsigned int __psp;
char * __fconv __P((char * fname));

unsigned int __segalloc __P((unsigned int paracount));
void         __setvect __P((int i, long j));
long         __getvect __P((int vecno));
#endif

unsigned int __get_ds  __P((void));
unsigned int __get_es  __P((void));
void         __set_es  __P((unsigned int seg));
int          __peek_es __P((unsigned int off));
int          __deek_es __P((unsigned int off));

#define movedata __movedata

#endif

