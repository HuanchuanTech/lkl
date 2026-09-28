/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _LKL_LINUX_SCHED_H
#define _LKL_LINUX_SCHED_H

#include <lkl/linux/types.h>

/*
 * cloning flags:
 */
#define LKL_CSIGNAL		0x000000ff	/* signal mask to be sent at exit */
#define LKL_CLONE_VM	0x00000100	/* set if VM shared between processes */
#define LKL_CLONE_FS	0x00000200	/* set if fs info shared between processes */
#define LKL_CLONE_FILES	0x00000400	/* set if open files shared between processes */
#define LKL_CLONE_SIGHAND	0x00000800	/* set if signal handlers and blocked signals shared */
#define LKL_CLONE_PIDFD	0x00001000	/* set if a pidfd should be placed in parent */
#define LKL_CLONE_PTRACE	0x00002000	/* set if we want to let tracing continue on the child too */
#define LKL_CLONE_VFORK	0x00004000	/* set if the parent wants the child to wake it up on mm_release */
#define LKL_CLONE_PARENT	0x00008000	/* set if we want to have the same parent as the cloner */
#define LKL_CLONE_THREAD	0x00010000	/* Same thread group? */
#define LKL_CLONE_NEWNS	0x00020000	/* New mount namespace group */
#define LKL_CLONE_SYSVSEM	0x00040000	/* share system V LKL_SEM_UNDO semantics */
#define LKL_CLONE_SETTLS	0x00080000	/* create a new TLS for the child */
#define LKL_CLONE_PARENT_SETTID	0x00100000	/* set the TID in the parent */
#define LKL_CLONE_CHILD_CLEARTID	0x00200000	/* clear the TID in the child */
#define LKL_CLONE_DETACHED		0x00400000	/* Unused, ignored */
#define LKL_CLONE_UNTRACED		0x00800000	/* set if the tracing process can't force LKL_CLONE_PTRACE on this clone */
#define LKL_CLONE_CHILD_SETTID	0x01000000	/* set the TID in the child */
#define LKL_CLONE_NEWCGROUP		0x02000000	/* New cgroup namespace */
#define LKL_CLONE_NEWUTS		0x04000000	/* New utsname namespace */
#define LKL_CLONE_NEWIPC		0x08000000	/* New ipc namespace */
#define LKL_CLONE_NEWUSER		0x10000000	/* New user namespace */
#define LKL_CLONE_NEWPID		0x20000000	/* New pid namespace */
#define LKL_CLONE_NEWNET		0x40000000	/* New network namespace */
#define LKL_CLONE_IO		0x80000000	/* Clone io context */

/* Flags for the clone3() syscall. */
#define LKL_CLONE_CLEAR_SIGHAND 0x100000000ULL /* Clear any signal handler and reset to LKL_SIG_DFL. */
#define LKL_CLONE_INTO_CGROUP 0x200000000ULL /* Clone into a specific cgroup given the right permissions. */

/*
 * cloning flags intersect with LKL_CSIGNAL so can be used with unshare and clone3
 * syscalls only:
 */
#define LKL_CLONE_NEWTIME	0x00000080	/* New time namespace */

#ifndef __ASSEMBLY__
/**
 * struct lkl_clone_args - arguments for the clone3 syscall
 * @flags:        Flags for the new process as listed above.
 *                All flags are valid except for LKL_CSIGNAL and
 *                LKL_CLONE_DETACHED.
 * @pidfd:        If LKL_CLONE_PIDFD is set, a pidfd will be
 *                returned in this argument.
 * @child_tid:    If LKL_CLONE_CHILD_SETTID is set, the TID of the
 *                child process will be returned in the child's
 *                memory.
 * @parent_tid:   If LKL_CLONE_PARENT_SETTID is set, the TID of
 *                the child process will be returned in the
 *                parent's memory.
 * @exit_signal:  The exit_signal the parent process will be
 *                sent when the child exits.
 * @stack:        Specify the location of the stack for the
 *                child process.
 *                Note, @stack is expected to point to the
 *                lowest address. The stack direction will be
 *                determined by the kernel and set up
 *                appropriately based on @stack_size.
 * @stack_size:   The size of the stack for the child process.
 * @tls:          If LKL_CLONE_SETTLS is set, the tls descriptor
 *                is set to tls.
 * @set_tid:      Pointer to an array of type *lkl_pid_t. The size
 *                of the array is defined using @set_tid_size.
 *                This array is used to select PIDs/TIDs for
 *                newly created processes. The first element in
 *                this defines the PID in the most nested PID
 *                namespace. Each additional element in the array
 *                defines the PID in the parent PID namespace of
 *                the original PID namespace. If the array has
 *                less entries than the number of currently
 *                nested PID namespaces only the PIDs in the
 *                corresponding namespaces are set.
 * @set_tid_size: This defines the size of the array referenced
 *                in @set_tid. This cannot be larger than the
 *                kernel's limit of nested PID namespaces.
 * @cgroup:       If LKL_CLONE_INTO_CGROUP is specified set this to
 *                a file descriptor for the cgroup.
 *
 * The structure is versioned by size and thus extensible.
 * New struct members must go at the end of the struct and
 * must be properly 64bit aligned.
 */
struct lkl_clone_args {
	__lkl__aligned_u64 flags;
	__lkl__aligned_u64 pidfd;
	__lkl__aligned_u64 child_tid;
	__lkl__aligned_u64 parent_tid;
	__lkl__aligned_u64 exit_signal;
	__lkl__aligned_u64 stack;
	__lkl__aligned_u64 stack_size;
	__lkl__aligned_u64 tls;
	__lkl__aligned_u64 set_tid;
	__lkl__aligned_u64 set_tid_size;
	__lkl__aligned_u64 cgroup;
};
#endif

#define LKL_CLONE_ARGS_SIZE_VER0 64 /* sizeof first published struct */
#define LKL_CLONE_ARGS_SIZE_VER1 80 /* sizeof second published struct */
#define LKL_CLONE_ARGS_SIZE_VER2 88 /* sizeof third published struct */

/*
 * Scheduling policies
 */
#define LKL_SCHED_NORMAL		0
#define LKL_SCHED_FIFO		1
#define LKL_SCHED_RR		2
#define LKL_SCHED_BATCH		3
/* SCHED_ISO: reserved but not implemented yet */
#define LKL_SCHED_IDLE		5
#define LKL_SCHED_DEADLINE		6
#define LKL_SCHED_EXT		7

/* Can be ORed in to make sure the process is reverted back to LKL_SCHED_NORMAL on fork */
#define LKL_SCHED_RESET_ON_FORK     0x40000000

/*
 * For the sched_{set,get}attr() calls
 */
#define LKL_SCHED_FLAG_RESET_ON_FORK	0x01
#define LKL_SCHED_FLAG_RECLAIM		0x02
#define LKL_SCHED_FLAG_DL_OVERRUN		0x04
#define LKL_SCHED_FLAG_KEEP_POLICY		0x08
#define LKL_SCHED_FLAG_KEEP_PARAMS		0x10
#define LKL_SCHED_FLAG_UTIL_CLAMP_MIN	0x20
#define LKL_SCHED_FLAG_UTIL_CLAMP_MAX	0x40

#define LKL_SCHED_FLAG_KEEP_ALL	(LKL_SCHED_FLAG_KEEP_POLICY | \
				 LKL_SCHED_FLAG_KEEP_PARAMS)

#define LKL_SCHED_FLAG_UTIL_CLAMP	(LKL_SCHED_FLAG_UTIL_CLAMP_MIN | \
				 LKL_SCHED_FLAG_UTIL_CLAMP_MAX)

#define LKL_SCHED_FLAG_ALL	(LKL_SCHED_FLAG_RESET_ON_FORK	| \
			 LKL_SCHED_FLAG_RECLAIM		| \
			 LKL_SCHED_FLAG_DL_OVERRUN		| \
			 LKL_SCHED_FLAG_KEEP_ALL		| \
			 LKL_SCHED_FLAG_UTIL_CLAMP)

#endif /* _LKL_LINUX_SCHED_H */
