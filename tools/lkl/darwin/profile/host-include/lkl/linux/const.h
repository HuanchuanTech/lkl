/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/* const.h: Macros for dealing with constants.  */

#ifndef _LKL_LINUX_CONST_H
#define _LKL_LINUX_CONST_H

/* Some constant macros are used in both assembler and
 * C code.  Therefore we cannot annotate them always with
 * 'UL' and other type specifiers unilaterally.  We
 * use the following macros to deal with this.
 *
 * Similarly, _LKL_AT() will cast an expression with a type in C, but
 * leave it unchanged in asm.
 */

#ifdef __ASSEMBLY__
#define _LKL_AC(X,Y)	X
#define _LKL_AT(T,X)	X
#else
#define __LKL__AC(X,Y)	(X##Y)
#define _LKL_AC(X,Y)	__LKL__AC(X,Y)
#define _LKL_AT(T,X)	((T)(X))
#endif

#define _LKL_UL(x)		(_LKL_AC(x, UL))
#define _LKL_ULL(x)		(_LKL_AC(x, ULL))

#define _LKL_BITUL(x)	(_LKL_UL(1) << (x))
#define _LKL_BITULL(x)	(_LKL_ULL(1) << (x))

#if !defined(__ASSEMBLY__)
/*
 * Missing __asm__ support
 *
 * __BIT128() would not work in the __asm__ code, as it shifts an
 * 'unsigned __init128' data type as direct representation of
 * 128 bit constants is not supported in the gcc compiler, as
 * they get silently truncated.
 *
 * TODO: Please revisit this implementation when gcc compiler
 * starts representing 128 bit constants directly like long
 * and unsigned long etc. Subsequently drop the comment for
 * GENMASK_U128() which would then start supporting __asm__ code.
 */
#define _LKL_BIT128(x)	((unsigned __int128)(1) << (x))
#endif

#define __LKL__ALIGN_KERNEL(x, a)		__LKL__ALIGN_KERNEL_MASK(x, (__typeof__(x))(a) - 1)
#define __LKL__ALIGN_KERNEL_MASK(x, mask)	(((x) + (mask)) & ~(mask))

#define __LKL__KERNEL_DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#endif /* _LKL_LINUX_CONST_H */
