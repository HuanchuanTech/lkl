#ifndef __BUILD_SALT_H
#define __BUILD_SALT_H

#include <linux/elfnote.h>

#define LINUX_ELFNOTE_BUILD_SALT       0x100

#ifdef __ASSEMBLER__

#define BUILD_SALT \
       ELFNOTE(Linux, LINUX_ELFNOTE_BUILD_SALT, .asciz CONFIG_BUILD_SALT)

#elif defined(__MACH__)

#define BUILD_SALT	/* macOS PoC: ELF note is ld64-incompatible and unused here */

#else

#define BUILD_SALT \
       ELFNOTE32("Linux", LINUX_ELFNOTE_BUILD_SALT, CONFIG_BUILD_SALT)

#endif

#endif /* __BUILD_SALT_H */
