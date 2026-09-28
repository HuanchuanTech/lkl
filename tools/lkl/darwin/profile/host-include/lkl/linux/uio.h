/* SPDX-License-Identifier: GPL-2.0+ WITH Linux-syscall-note */
/*
 *	Berkeley style UIO structures	-	Alan Cox 1994.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef __LKL__LINUX_UIO_H
#define __LKL__LINUX_UIO_H


#include <lkl/linux/types.h>


struct lkl_iovec
{
	void *iov_base;	/* BSD uses caddr_t (1003.1g requires void *) */
	__lkl__kernel_size_t iov_len; /* Must be lkl_size_t (1003.1g) */
};

struct lkl_dmabuf_cmsg {
	__lkl__u64 frag_offset;	/* offset into the dmabuf where the frag starts.
				 */
	__lkl__u32 frag_size;	/* size of the frag. */
	__lkl__u32 frag_token;	/* token representing this frag for
				 * DEVMEM_DONTNEED.
				 */
	__lkl__u32  dmabuf_id;	/* dmabuf id this frag belongs to. */
	__lkl__u32 flags;		/* Currently unused. Reserved for future
				 * uses.
				 */
};

struct lkl_dmabuf_token {
	__lkl__u32 token_start;
	__lkl__u32 token_count;
};

/*
 *	LKL_UIO_MAXIOV shall be at least 16 1003.1g (5.4.1.1)
 */
 
#define LKL_UIO_FASTIOV	8
#define LKL_UIO_MAXIOV	1024


#endif /* __LKL__LINUX_UIO_H */
