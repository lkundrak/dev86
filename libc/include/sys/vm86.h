#if !__AS386_16__

#ifndef _SYS_VM86_H
#define _SYS_VM86_H

#include <features.h>
#ifndef _LINUX_VM86_H
#define _LINUX_VM86_H

/*
 * I'm guessing at the VIF/VIP flag usage, but hope that this is how
 * the Pentium uses them. Linux will return from vm86 mode when both
 * VIF and VIP is set.
 *
 * On a Pentium, we could probably optimize the virtual flags directly
 * in the eflags register instead of doing it "by hand" in vflags...
 *
 * Linus
 */

#define TF_MASK		0x00000100
#define IF_MASK		0x00000200
#define IOPL_MASK	0x00003000
#define NT_MASK		0x00004000
#define VM_MASK		0x00020000
#define AC_MASK		0x00040000
#define VIF_MASK	0x00080000	/* virtual interrupt flag */
#define VIP_MASK	0x00100000	/* virtual interrupt pending */
#define ID_MASK		0x00200000

#define BIOSSEG		0x0f000

#define CPU_086		0
#define CPU_186		1
#define CPU_286		2
#define CPU_386		3
#define CPU_486		4
#define CPU_586		5

/*
 * Return values for the 'vm86()' system call
 */
#define VM86_TYPE(retval)	((retval) & 0xff)
#define VM86_ARG(retval)	((retval) >> 8)

#define VM86_SIGNAL	0	/* return due to signal */
#define VM86_UNKNOWN	1	/* unhandled GP fault - IO-instruction or similar */
#define VM86_INTx	2	/* int3/int x instruction (ARG = x) */
#define VM86_STI	3	/* sti/popf/iret instruction enabled virtual interrupts */

/*
 * This is the stack-layout when we have done a "SAVE_ALL" from vm86
 * mode - the main change is that the old segment descriptors aren't
 * useful any more and are forced to be zero by the kernel (and the
 * hardware when a trap occurs), and the real segment descriptors are
 * at the end of the structure. Look at ptrace.h to see the "normal"
 * setup.
 */

struct vm86_regs {
/*
 * normal regs, with special meaning for the segment descriptors..
 */
	long ebx;
	long ecx;
	long edx;
	long esi;
	long edi;
	long ebp;
	long eax;
	long __null_ds;
	long __null_es;
	long __null_fs;
	long __null_gs;
	long orig_eax;
	long eip;
	unsigned short cs, __csh;
	long eflags;
	long esp;
	unsigned short ss, __ssh;
/*
 * these are specific to v86 mode:
 */
	unsigned short es, __esh;
	unsigned short ds, __dsh;
	unsigned short fs, __fsh;
	unsigned short gs, __gsh;
};

struct revectored_struct {
	unsigned long __map[8];			/* 256 bits */
};

struct vm86_struct {
	struct vm86_regs regs;
	unsigned long flags;
	unsigned long screen_bitmap;
	unsigned long cpu_type;
	struct revectored_struct int_revectored;
	struct revectored_struct int21_revectored;
};

/*
 * flags masks
 */
#define VM86_SCREEN_BITMAP	0x0001

#ifdef __KERNEL__

void handle_vm86_fault(struct vm86_regs *, long);
void handle_vm86_debug(struct vm86_regs *, long);

#endif

#endif

__BEGIN_DECLS

extern vm86(struct vm86_struct * __info);

__END_DECLS

#endif /*_SYS_VM86_H */

#endif
