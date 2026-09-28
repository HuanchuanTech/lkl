// SPDX-License-Identifier: GPL-2.0-only
/*
 * idle.c split out of build_policy.c into its own compilation unit.
 *
 * Why: on macOS/Mach-O there is no linker script, so DEFINE_SCHED_CLASS
 * instances are ordered purely by object/emit order within the single
 * __DATA,__schedclass section. sched_init() requires
 *   &dl_sched_class < &rt_sched_class < &fair_sched_class < &idle_sched_class.
 * dl/rt live in build_policy.o and fair lives in fair.o, so idle_sched_class
 * must come from an object linked *after* fair.o — hence its own unit here.
 * The headers below mirror build_policy.c so idle.c sees the same context.
 * For the ELF build this is just another translation unit; its lds re-sorts by
 * section so the split is harmless there.
 */

/* Headers: (mirror build_policy.c) */
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

/* Source code module: */

#include "idle.c"
