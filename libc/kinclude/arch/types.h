/* arch/i86/include/asm/types.h - Basic Linux/MT data types. */

#ifndef __ARCH_8086_TYPES
#define __ARCH_8086_TYPES

#include <asm/types.h>
	
struct _registers {
	__u16 ksp, sp, ss, ax, bx, cx, dx, di, si, ds, es, bp, ip, cs, flags;
};

typedef struct _registers __registers; 
typedef struct _registers * __pregisters;

typedef __u32 __pptr;

struct _mminit {
	__u16 cs, endcs, ds, endds, ss, endss, lowss;
};

typedef struct _mminit __arch_mminit;
typedef struct _mminit * __parch_mminit;

#endif
