
/* asm/types.h - Basic sized C data types. */

#ifndef __ASM_8086_TYPES
#define __ASM_8086_TYPES
	
/* First we define all of the __u and __s types...*/

typedef unsigned char __u8;
typedef unsigned char * __pu8;

#ifndef __BCC__		/* NOTE! BCC does _not_ have a signed char type! */
typedef char __s8;
typedef char * __ps8;
#endif

typedef unsigned short __u16;
typedef unsigned short * __pu16;
typedef short __s16;
typedef short * __ps16;

typedef unsigned long __u32;
typedef unsigned long * __pu32;
typedef long __s32;
typedef long * __ps32;

/* __uint == 16bit on 8086 32bit on i386 */

typedef unsigned int __uint;
typedef int __sint;
typedef unsigned int * __puint;
typedef int * __psint;

#endif
