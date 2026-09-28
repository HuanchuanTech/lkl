// boot_glue.S — macOS/Mach-O boot glue for in-process LKL.
//
// Defines the symbols the vmlinux linker script would normally provide but
// which don't exist in our hand-merged Mach-O kernel object. Symbol names are
// the exact Mach-O asm names (as printed by `nm -u vmlinux_macho.o`); recall
// the C-name -> asm-name rule: C `foo` -> `_foo`, C `_foo` -> `__foo`,
// C `__foo` -> `___foo`.
//
// Three jobs:
//   1. Emit a NULL placeholder into every per-level initcall section so ld64's
//      `section$start$/$end$` synthetic symbols always resolve (init/main.c
//      walks them and skips NULL entries).
//   2. Provide empty section-boundary pairs (start == stop) for sections our
//      compat header collapsed into .data/.text, plus position markers.
//   3. Provide a few concrete stand-ins: jiffies alias, init stack, blake2s
//      alias, kallsyms stubs, PCI-quirk stubs, vmalloc_huge_noprof,
//      linux_proc_banner.

// ===========================================================================
// 1. initcall section NULL placeholders
// ===========================================================================
// Each adds one NULL initcall_t (8 bytes) to the section. For populated levels
// it's an extra skipped entry; for empty levels it's what makes the section —
// and thus section$start$/$end$ — exist at all.
.macro IC_PLACEHOLDER sect
	.section __DATA,\sect
	.p2align 3
	.quad 0
.endm

IC_PLACEHOLDER __icearly
IC_PLACEHOLDER __ic0
IC_PLACEHOLDER __ic0s
IC_PLACEHOLDER __ic1
IC_PLACEHOLDER __ic1s
IC_PLACEHOLDER __ic2
IC_PLACEHOLDER __ic2s
IC_PLACEHOLDER __ic3
IC_PLACEHOLDER __ic3s
IC_PLACEHOLDER __ic4
IC_PLACEHOLDER __ic4s
IC_PLACEHOLDER __ic5
IC_PLACEHOLDER __ic5s
IC_PLACEHOLDER __icrootfs
IC_PLACEHOLDER __ic6
IC_PLACEHOLDER __ic6s
IC_PLACEHOLDER __ic7
IC_PLACEHOLDER __ic7s

// ===========================================================================
// 2. Data anchors + buffers
// ===========================================================================
	.section __DATA,__data
	.p2align 3
// Generic zero anchor for empty boundary ranges and position markers.
_mo_zero:
	.quad 0

// Sentinel obs_kernel_param for the do-while in obsolete_checksetup().
// struct obs_kernel_param { const char *str; int (*setup_func)(char*); int early; }
_mo_setup_str:
	.asciz "\177\177\177\177"        // 4x DEL: never a prefix of a real boot option
	.p2align 3
_mo_setup_sentinel:
	.quad _mo_setup_str              // .str  (non-NULL, strlen-safe)
	.quad 0                          // .setup_func (NULL)
	.long 0                          // .early
	.long 0                          // padding to sizeof(struct obs_kernel_param)
	.globl ___setup_start
	.set ___setup_start, _mo_setup_sentinel
	.globl ___setup_end
	.set ___setup_end, _mo_setup_sentinel

// init_thread_union / init_stack alias the properly-initialized init_thread_info
// (init/init_task.c, INIT_THREAD_INFO(init_task) => .task = &init_task). The
// vmlinux linker script overlays init_thread_union/init_stack at the same
// address as that thread_info; aliasing reproduces that so
// init_thread_union.thread_info.task == &init_task (current is non-NULL).
// LKL kernel threads run on host pthread stacks, so init_stack is only used for
// thread_info location / end_of_stack checks, not as an execution stack.
	.globl _init_thread_union
	.set _init_thread_union, _init_thread_info
	.globl _init_stack
	.set _init_stack, _init_thread_info

// ===========================================================================
// 3a. Aliases to symbols defined in the kernel object (N_INDR)
// ===========================================================================
	.globl _jiffies
	.set _jiffies, _jiffies_64

// ===========================================================================
// 3b. Empty section-boundary pairs and position markers -> _mo_zero
// ===========================================================================
// All of these are iterated as `for (p = start; p < stop; p++)` or used only
// as addresses; pointing start and stop at the same byte makes every range
// empty and harmless.
.macro EMPTY_SYM name
	.globl \name
	.set \name, _mo_zero
.endm

// console initcalls (lkl registers its console via early/core_initcall instead)
EMPTY_SYM ___con_initcall_start
EMPTY_SYM ___con_initcall_end
// boot-option (__setup) handlers: obsolete_checksetup() is a do{}while loop that
// dereferences __setup_start->str BEFORE the bound check, so an empty range still
// runs once. Point start/end at a benign sentinel (str never matches a real
// option, setup_func NULL) so that single iteration is a no-op. (Handled below
// in the data section, not via EMPTY_SYM.)
// kernel params: bracket the __DATA,__kparam section (kernel_param structs from
// the patched __module_param_call) so start_kernel's parse_args() can match
// boot-cmdline module params like virtio_mmio.device=.
	.globl ___start___param
	.set ___start___param, section$start$__DATA$__kparam
	.globl ___stop___param
	.set ___stop___param, section$end$__DATA$__kparam
// exception table (lkl uaccess does not fault)
EMPTY_SYM ___start___ex_table
EMPTY_SYM ___stop___ex_table
// module version table
EMPTY_SYM ___start___modver
EMPTY_SYM ___stop___modver
// tracing tables
EMPTY_SYM ___start___trace_bprintk_fmt
EMPTY_SYM ___stop___trace_bprintk_fmt
EMPTY_SYM ___start___tracepoint_str
EMPTY_SYM ___stop___tracepoint_str
EMPTY_SYM ___start___tracepoints_ptrs
EMPTY_SYM ___stop___tracepoints_ptrs
EMPTY_SYM ___start_ftrace_eval_maps
EMPTY_SYM ___stop_ftrace_eval_maps
EMPTY_SYM ___start_ftrace_events
EMPTY_SYM ___stop_ftrace_events
EMPTY_SYM ___start_notes
EMPTY_SYM ___stop_notes
// special text ranges (functions all live in plain .text here)
EMPTY_SYM ___cpuidle_text_start
EMPTY_SYM ___cpuidle_text_end
EMPTY_SYM ___irqentry_text_start
EMPTY_SYM ___irqentry_text_end
EMPTY_SYM ___softirqentry_text_start
EMPTY_SYM ___softirqentry_text_end
EMPTY_SYM ___sched_text_start
EMPTY_SYM ___sched_text_end
// scheduler class array — bracket the __DATA,__schedclass section (4 contiguous
// sched_class structs: dl, rt, fair, idle) via ld64 synthetic boundary symbols.
	.globl ___sched_class_highest
	.set ___sched_class_highest, section$start$__DATA$__schedclass
	.globl ___sched_class_lowest
	.set ___sched_class_lowest, section$end$__DATA$__schedclass
// image position markers
EMPTY_SYM __stext
EMPTY_SYM __etext
EMPTY_SYM __sdata
EMPTY_SYM __edata
EMPTY_SYM __sinittext
EMPTY_SYM __einittext
EMPTY_SYM __end
EMPTY_SYM ___init_begin
EMPTY_SYM ___init_end
EMPTY_SYM ___bss_start
EMPTY_SYM ___bss_stop
EMPTY_SYM ___start_rodata
EMPTY_SYM ___end_rodata

// ===========================================================================
// 3c. kallsyms stubs (num_syms = 0 -> never indexed)
// ===========================================================================
	.section __DATA,__data
	.p2align 3
	.globl _kallsyms_num_syms
_kallsyms_num_syms:
	.long 0
	.globl _kallsyms_relative_base
_kallsyms_relative_base:
	.quad 0
	.globl _kallsyms_offsets
_kallsyms_offsets:
	.long 0
	.globl _kallsyms_markers
_kallsyms_markers:
	.long 0
	.globl _kallsyms_token_index
_kallsyms_token_index:
	.short 0
	.globl _kallsyms_names
_kallsyms_names:
	.byte 0
	.globl _kallsyms_token_table
_kallsyms_token_table:
	.byte 0
	.globl _kallsyms_seqs_of_names
_kallsyms_seqs_of_names:
	.byte 0

// PCI VPD attribute group (address-taken in a static list; zeroed is inert).
	.p2align 3
	.globl _pci_dev_vpd_attr_group
_pci_dev_vpd_attr_group:
	.space 64

// linux_proc_banner — only read via /proc/version; 3 %s match the reader.
	.section __TEXT,__cstring,cstring_literals
	.globl _linux_proc_banner
_linux_proc_banner:
	.asciz "%s version %s %s\n"

// ===========================================================================
// 3d. Function stubs (never called for ext4-over-virtio: no PCI bus)
// ===========================================================================
	.section __TEXT,__text
	.p2align 2

// vmalloc_huge_noprof(size, gfp) -> __vmalloc_noprof(size, gfp): the huge
// variant is config'd out here; fall back to the normal allocator (same args).
	.globl _vmalloc_huge_noprof
_vmalloc_huge_noprof:
#if defined(__x86_64__)
	jmp ___vmalloc_noprof
#else
	b ___vmalloc_noprof
#endif

// blake2s_compress is __alias(blake2s_compress_generic) upstream; the alias was
// neutralized by the compat header, so tail-call the generic implementation.
	.globl _blake2s_compress
_blake2s_compress:
#if defined(__x86_64__)
	jmp _blake2s_compress_generic
#else
	b _blake2s_compress_generic
#endif

// PCI quirk hooks — return 0 / do nothing.
.macro STUB_RET0 name
	.globl \name
\name:
#if defined(__x86_64__)
	xorl %eax, %eax
#else
	mov x0, #0
#endif
	ret
.endm

STUB_RET0 _pci_dev_specific_acs_enabled
STUB_RET0 _pci_dev_specific_disable_acs_redir
STUB_RET0 _pci_dev_specific_enable_acs
STUB_RET0 _pci_dev_specific_reset
STUB_RET0 _pci_fixup_device
STUB_RET0 _pci_idt_bus_quirk
STUB_RET0 _pci_vpd_init
STUB_RET0 _pcie_failed_link_retrain
