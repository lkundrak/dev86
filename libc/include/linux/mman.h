#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#include <features.h>
#include <sys/types.h>

#define PROT_READ	0x1		/* page can be read */
#define PROT_WRITE	0x2		/* page can be written */
#define PROT_EXEC	0x4		/* page can be executed */
#define PROT_NONE	0x0		/* page can not be accessed */

#define MAP_SHARED	0x01		/* Share changes */
#define MAP_PRIVATE	0x02		/* Changes are private */
#define MAP_TYPE	0x0f		/* Mask for type of mapping */
#define MAP_FIXED	0x10		/* Interpret addr exactly */
#define MAP_ANONYMOUS	0x20		/* don't use a file */

#define MAP_GROWSDOWN	0x0100		/* stack-like segment */
#define MAP_DENYWRITE	0x0800		/* ETXTBSY */
#define MAP_EXECUTABLE	0x1000		/* mark it as a executable */
#define MAP_LOCKED	0x2000		/* pages are locked */

#define MS_ASYNC	1		/* sync memory asynchronously */
#define MS_INVALIDATE	2		/* invalidate the caches */
#define MS_SYNC		4		/* synchronous memory sync */

#define MCL_CURRENT	1		/* lock all current mappings */
#define MCL_FUTURE	2		/* lock all future mappings */

/* compatibility flags */
#define MAP_ANON	MAP_ANONYMOUS
#define MAP_FILE	0

#define MREMAP_MAYMOVE	1

extern __ptr_t mmap __P((__ptr_t __addr, size_t __len,
		int __prot, int __flags, int __fd, off_t __off));
extern int munmap __P((__ptr_t __addr, size_t __len));
extern int mprotect __P ((__const __ptr_t __addr, size_t __len, int __prot));

extern int msync __P((__ptr_t __addr, size_t __len, int __flags));

extern int mlock __P((__const __ptr_t __addr, size_t __len));
extern int munlock __P((__const __ptr_t __addr, size_t __len));

extern int mlockall __P((int __flags));
extern int munlockall __P((void));

extern __ptr_t mremap __P((__ptr_t __addr, size_t __old_len,
		size_t __new_len, int __may_move));

#endif /* _SYS_MMAN_H */
