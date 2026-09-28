/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _LKL_LINUX_MMAN_H
#define _LKL_LINUX_MMAN_H

#include <lkl/asm/mman.h>
#include <lkl/asm-generic/hugetlb_encode.h>
#include <lkl/linux/types.h>

#define LKL_MREMAP_MAYMOVE		1
#define LKL_MREMAP_FIXED		2
#define LKL_MREMAP_DONTUNMAP	4

#define LKL_OVERCOMMIT_GUESS		0
#define LKL_OVERCOMMIT_ALWAYS		1
#define LKL_OVERCOMMIT_NEVER		2

#define LKL_MAP_SHARED	0x01		/* Share changes */
#define LKL_MAP_PRIVATE	0x02		/* Changes are private */
#define LKL_MAP_SHARED_VALIDATE 0x03	/* share + validate extension flags */
#define LKL_MAP_DROPPABLE	0x08		/* Zero memory under memory pressure. */

/*
 * Huge page size encoding when LKL_MAP_HUGETLB is specified, and a huge page
 * size other than the default is desired.  See hugetlb_encode.h.
 * All known huge page size encodings are provided here.  It is the
 * responsibility of the application to know which sizes are supported on
 * the running system.  See mmap(2) man page for details.
 */
#define LKL_MAP_HUGE_SHIFT	LKL_HUGETLB_FLAG_ENCODE_SHIFT
#define LKL_MAP_HUGE_MASK	LKL_HUGETLB_FLAG_ENCODE_MASK

#define LKL_MAP_HUGE_16KB	LKL_HUGETLB_FLAG_ENCODE_16KB
#define LKL_MAP_HUGE_64KB	LKL_HUGETLB_FLAG_ENCODE_64KB
#define LKL_MAP_HUGE_512KB	LKL_HUGETLB_FLAG_ENCODE_512KB
#define LKL_MAP_HUGE_1MB	LKL_HUGETLB_FLAG_ENCODE_1MB
#define LKL_MAP_HUGE_2MB	LKL_HUGETLB_FLAG_ENCODE_2MB
#define LKL_MAP_HUGE_8MB	LKL_HUGETLB_FLAG_ENCODE_8MB
#define LKL_MAP_HUGE_16MB	LKL_HUGETLB_FLAG_ENCODE_16MB
#define LKL_MAP_HUGE_32MB	LKL_HUGETLB_FLAG_ENCODE_32MB
#define LKL_MAP_HUGE_256MB	LKL_HUGETLB_FLAG_ENCODE_256MB
#define LKL_MAP_HUGE_512MB	LKL_HUGETLB_FLAG_ENCODE_512MB
#define LKL_MAP_HUGE_1GB	LKL_HUGETLB_FLAG_ENCODE_1GB
#define LKL_MAP_HUGE_2GB	LKL_HUGETLB_FLAG_ENCODE_2GB
#define LKL_MAP_HUGE_16GB	LKL_HUGETLB_FLAG_ENCODE_16GB

struct lkl_cachestat_range {
	__lkl__u64 off;
	__lkl__u64 len;
};

struct lkl_cachestat {
	__lkl__u64 nr_cache;
	__lkl__u64 nr_dirty;
	__lkl__u64 nr_writeback;
	__lkl__u64 nr_evicted;
	__lkl__u64 nr_recently_evicted;
};

#endif /* _LKL_LINUX_MMAN_H */
