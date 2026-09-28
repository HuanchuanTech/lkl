/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef __LKL__ASM_GENERIC_MMAN_COMMON_H
#define __LKL__ASM_GENERIC_MMAN_COMMON_H

/*
 Author: Michael S. Tsirkin <mst@mellanox.co.il>, Mellanox Technologies Ltd.
 Based on: asm-xxx/mman.h
*/

#define LKL_PROT_READ	0x1		/* page can be read */
#define LKL_PROT_WRITE	0x2		/* page can be written */
#define LKL_PROT_EXEC	0x4		/* page can be executed */
#define LKL_PROT_SEM	0x8		/* page may be used for atomic ops */
/*			0x10		   reserved for arch-specific use */
/*			0x20		   reserved for arch-specific use */
#define LKL_PROT_NONE	0x0		/* page can not be accessed */
#define LKL_PROT_GROWSDOWN	0x01000000	/* mprotect flag: extend change to start of growsdown vma */
#define LKL_PROT_GROWSUP	0x02000000	/* mprotect flag: extend change to end of growsup vma */

/* 0x01 - 0x03 are defined in linux/mman.h */
#define LKL_MAP_TYPE	0x0f		/* Mask for type of mapping */
#define LKL_MAP_FIXED	0x10		/* Interpret addr exactly */
#define LKL_MAP_ANONYMOUS	0x20		/* don't use a file */

/* 0x0100 - 0x4000 flags are defined in asm-generic/mman.h */
#define LKL_MAP_POPULATE		0x008000	/* populate (prefault) pagetables */
#define LKL_MAP_NONBLOCK		0x010000	/* do not block on IO */
#define LKL_MAP_STACK		0x020000	/* give out an address that is best suited for process/thread stacks */
#define LKL_MAP_HUGETLB		0x040000	/* create a huge page mapping */
#define LKL_MAP_SYNC		0x080000 /* perform synchronous page faults for the mapping */
#define LKL_MAP_FIXED_NOREPLACE	0x100000	/* LKL_MAP_FIXED which doesn't unmap underlying mapping */

#define LKL_MAP_UNINITIALIZED 0x4000000	/* For anonymous mmap, memory could be
					 * uninitialized */

/*
 * Flags for mlock
 */
#define LKL_MLOCK_ONFAULT	0x01		/* Lock pages in range after they are faulted in, do not prefault */

#define LKL_MS_ASYNC	1		/* sync memory asynchronously */
#define LKL_MS_INVALIDATE	2		/* invalidate the caches */
#define LKL_MS_SYNC		4		/* synchronous memory sync */

#define LKL_MADV_NORMAL	0		/* no further special treatment */
#define LKL_MADV_RANDOM	1		/* expect random page references */
#define LKL_MADV_SEQUENTIAL	2		/* expect sequential page references */
#define LKL_MADV_WILLNEED	3		/* will need these pages */
#define LKL_MADV_DONTNEED	4		/* don't need these pages */

/* common parameters: try to keep these consistent across architectures */
#define LKL_MADV_FREE	8		/* free pages only if memory pressure */
#define LKL_MADV_REMOVE	9		/* remove these pages & resources */
#define LKL_MADV_DONTFORK	10		/* don't inherit across fork */
#define LKL_MADV_DOFORK	11		/* do inherit across fork */
#define LKL_MADV_HWPOISON	100		/* poison a page for testing */
#define LKL_MADV_SOFT_OFFLINE 101		/* soft offline page for testing */

#define LKL_MADV_MERGEABLE   12		/* KSM may merge identical pages */
#define LKL_MADV_UNMERGEABLE 13		/* KSM may not merge identical pages */

#define LKL_MADV_HUGEPAGE	14		/* Worth backing with hugepages */
#define LKL_MADV_NOHUGEPAGE	15		/* Not worth backing with hugepages */

#define LKL_MADV_DONTDUMP   16		/* Explicity exclude from the core dump,
					   overrides the coredump filter bits */
#define LKL_MADV_DODUMP	17		/* Clear the LKL_MADV_DONTDUMP flag */

#define LKL_MADV_WIPEONFORK 18		/* Zero memory on fork, child only */
#define LKL_MADV_KEEPONFORK 19		/* Undo LKL_MADV_WIPEONFORK */

#define LKL_MADV_COLD	20		/* deactivate these pages */
#define LKL_MADV_PAGEOUT	21		/* reclaim these pages */

#define LKL_MADV_POPULATE_READ	22	/* populate (prefault) page tables readable */
#define LKL_MADV_POPULATE_WRITE	23	/* populate (prefault) page tables writable */

#define LKL_MADV_DONTNEED_LOCKED	24	/* like DONTNEED, but drop locked pages too */

#define LKL_MADV_COLLAPSE	25		/* Synchronous hugepage collapse */

/* compatibility flags */
#define LKL_MAP_FILE	0

#define LKL_PKEY_DISABLE_ACCESS	0x1
#define LKL_PKEY_DISABLE_WRITE	0x2
#define LKL_PKEY_ACCESS_MASK	(LKL_PKEY_DISABLE_ACCESS |\
				 LKL_PKEY_DISABLE_WRITE)

#endif /* __LKL__ASM_GENERIC_MMAN_COMMON_H */
