
#ifndef __BIOS_H
#define __BIOS_H
#include <features.h>

union REGS
{
   struct { unsigned int ax, bx, cx, dx, si, di, cflag, flags; } x;
   struct { unsigned char al, ah, bl, bh, cl, ch, dl, dh; } h;
};

struct SREGS
{
   unsigned int es, cs, ss, ds;
};

unsigned int __get_cs  __P((void));
unsigned int __get_ds  __P((void));
unsigned int __get_es  __P((void));
void         __set_es  __P((unsigned int seg));
int          __peek_es __P((unsigned int off));
int          __deek_es __P((unsigned int off));

#define movedata __movedata
long _bios_get_dpt(drive);

#ifdef __LIBC__

extern int __nofiles();
extern int (*__files)();

#define CMD_OPEN	0
#define CMD_READ	1
#define CMD_WRITE	2
#define CMD_LSEEK	3
#define CMD_CLOSE	4

#endif
#endif

