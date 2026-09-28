/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef __LKL__ASM_GENERIC_MMAN_H
#define __LKL__ASM_GENERIC_MMAN_H

#include <lkl/asm-generic/mman-common.h>

#define LKL_MAP_GROWSDOWN	0x0100		/* stack-like segment */
#define LKL_MAP_DENYWRITE	0x0800		/* LKL_ETXTBSY */
#define LKL_MAP_EXECUTABLE	0x1000		/* mark it as an executable */
#define LKL_MAP_LOCKED	0x2000		/* pages are locked */
#define LKL_MAP_NORESERVE	0x4000		/* don't check for reservations */

/*
 * Bits [26:31] are reserved, see asm-generic/hugetlb_encode.h
 * for LKL_MAP_HUGETLB usage
 */

#define LKL_MCL_CURRENT	1		/* lock all current mappings */
#define LKL_MCL_FUTURE	2		/* lock all future mappings */
#define LKL_MCL_ONFAULT	4		/* lock all pages that are faulted in */

#endif /* __LKL__ASM_GENERIC_MMAN_H */
