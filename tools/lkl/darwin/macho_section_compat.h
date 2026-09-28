#ifndef _MACHO_SECTION_COMPAT_H
#define _MACHO_SECTION_COMPAT_H
#if defined(__MACH__)
/* Fixed Darwin filesystem profile; paired with boot_glue.S and the
   Mach-O initcall, parameter and scheduler sections in the kernel. */

/* (1) ELF section placement -> neutralized (real port: per-section Mach-O map). */
#undef __section
#define __section(s)

/* (1b) __alias = __attribute__((alias)) is unsupported by ld64; neutralize here
   (a few callsites become undefined-weak; provide `.set` stubs at link). */
#undef __alias
#define __alias(symbol)

/* (1b2) noinstr/__cpuidle use __noinstr_section() -> direct __section__; neutralize. */
#undef __noinstr_section
#define __noinstr_section(section)

/* (1c) cond_syscall: ld64 has no weak ALIAS (.weak_definition+.set makes an
   indirect symbol that ld -r treats as a strong duplicate). Emit a weak FUNCTION
   STUB that tail-calls sys_ni_syscall -> it yields to a real syscall def, and
   provides the -ENOSYS fallback when none exists. #ifndef-guarded. */
#ifndef cond_syscall
#if defined(__x86_64__)
#define LKL_SYSCALL_BRANCH "jmp _sys_ni_syscall"
#else
#define LKL_SYSCALL_BRANCH "b _sys_ni_syscall"
#endif
#define cond_syscall(x) __asm__(				\
	".globl _" #x "\n\t"					\
	".weak_definition _" #x "\n\t"				\
	".p2align 2\n"						\
	"_" #x ":\n\t"						\
	LKL_SYSCALL_BRANCH)
#endif

/* (1d) __cacheline_aligned uses a direct __section__ (not the __section macro);
   it is #ifndef-guarded, so pre-define the section-less form. */
#ifndef __cacheline_aligned
#define __cacheline_aligned __attribute__((__aligned__(SMP_CACHE_BYTES)))
#endif

/* (2) symbol aliases: ld64 has no alias attr -> Mach-O assembler .set.
   __SYSCALL_DEFINEx is #ifndef-guarded, so pre-defining it here wins. */
#ifndef __SYSCALL_DEFINEx
#define __SYSCALL_DEFINEx(x, name, ...)					\
	asmlinkage long sys##name(__MAP(x,__SC_DECL,__VA_ARGS__));	\
	__asm__(".globl _sys" #name "\n.set _sys" #name ", ___se_sys" #name);\
	ALLOW_ERROR_INJECTION(sys##name, ERRNO);			\
	static inline long __do_sys##name(__MAP(x,__SC_DECL,__VA_ARGS__));\
	asmlinkage long __se_sys##name(__MAP(x,__SC_LONG,__VA_ARGS__));	\
	asmlinkage long __se_sys##name(__MAP(x,__SC_LONG,__VA_ARGS__))	\
	{								\
		long ret = __do_sys##name(__MAP(x,__SC_CAST,__VA_ARGS__));\
		__MAP(x,__SC_TEST,__VA_ARGS__);				\
		__PROTECT(x, ret,__MAP(x,__SC_ARGS,__VA_ARGS__));	\
		return ret;						\
	}								\
	static inline long __do_sys##name(__MAP(x,__SC_DECL,__VA_ARGS__))
#endif
#endif
#endif
