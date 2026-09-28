// SPDX-License-Identifier: GPL-2.0-only
/*
 * These are the scheduling policy related scheduler files, built
 * in a single compilation unit for build efficiency reasons.
 *
 * ( Incidentally, the size of the compilation unit is roughly
 *   comparable to core.c and fair.c, the other two big
 *   compilation units. This helps balance build time, while
 *   coalescing source files to amortize header inclusion
 *   cost. )
 *
 * core.c and fair.c are built separately.
 */

/* Headers: */
#include <linux/sched/clock.h>
#include <linux/sched/cputime.h>
#include <linux/sched/hotplug.h>
#include <linux/sched/isolation.h>
#include <linux/sched/posix-timers.h>
#include <linux/sched/rt.h>

#include <linux/cpuidle.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/livepatch.h>
#include <linux/pm.h>
#include <linux/psi.h>
#include <linux/rhashtable.h>
#include <linux/seq_buf.h>
#include <linux/seqlock_api.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/tsacct_kern.h>
#include <linux/vtime.h>
#include <linux/sysrq.h>
#include <linux/percpu-rwsem.h>

#include <uapi/linux/sched/types.h>

#include "sched.h"
#include "smp.h"

#include "autogroup.h"
#include "stats.h"
#include "pelt.h"

/* Source code modules: */

/*
 * idle.c is built separately (build_idle.c) and deadline.c is pulled in before
 * rt.c so that, on macOS/Mach-O (no linker script), the DEFINE_SCHED_CLASS
 * instances land in the right address order. The vmlinux linker script needs
 * __sched_class_highest..lowest laid out as dl < rt < fair < idle; with one
 * Mach-O section ordered by object/emit order that means this object must emit
 * dl, rt (in that order) and idle must follow fair (a separate object). The
 * order is irrelevant to the ELF build (its lds re-sorts by section).
 */

#ifdef CONFIG_SMP
# include "cpudeadline.c"
# include "pelt.c"
#endif

#include "cputime.c"
#include "deadline.c"

#include "rt.c"

#ifdef CONFIG_SCHED_CLASS_EXT
# include "ext.c"
#endif

#include "syscalls.c"
