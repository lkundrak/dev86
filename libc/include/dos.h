
#ifndef __DOS_H
#define __DOS_H
#include <features.h>
#include <bios.h>

#ifdef __MSDOS__
extern unsigned int __envseg;
extern unsigned int __psp;
char * __fconv __P((char * fname));

unsigned int __segalloc __P((unsigned int paracount));
void         __setvect __P((int i, long j));
long         __getvect __P((int vecno));
#endif

#endif

