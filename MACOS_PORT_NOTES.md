# LKL → macOS (Mach-O) in-process port — feasibility findings

> Historical bring-up journal (June 2026). Some conclusions and commands below
> were superseded during development. The maintained build entry point and
> current constraints are in [tools/lkl/darwin/README.md](tools/lkl/darwin/README.md).
> The `.macboot`, `.hostinc`, `/tmp` paths and compiled objects mentioned here
> are not required by the versioned Darwin build.

## ✅✅✅ MOUNT + READ ACHIEVED (2026-06-27) — real filesystems work end-to-end

The harness (`.macboot/harness.c`) now boots the kernel, attaches an ext2/3/4 image over **virtio-blk backed by a host file**, `lkl_mount_dev`-mounts it, and reads a file — printing e.g. `hello from ext4 inside in-process LKL on macOS!`. Verified for **ext2, ext3, ext4** (all via the ext4 driver: ext2 "without journal", ext3/ext4 "ordered data mode", journal recovery works). XFS and Btrfs drivers load at boot (`SGI XFS …`, `Btrfs loaded`) and use the identical mount path; only untested because the container lacks `mkfs.xfs`/`mkfs.btrfs` to make images. Run: `.macboot/run.sh` (uses `test.ext4`); or `./lkl_boot <image> <fstype>`.

### Fixes from "boots" → "mounts" (the host/syscall layer)
1. **Apple arm64 variadic-syscall ABI (THE big one)** — `arch/lkl/kernel/syscalls.c` dispatched via `typedef long (*)(long, ...)` (variadic). Apple's arm64 ABI passes variadic args **on the stack**, but the real handlers are non-variadic and read fixed args from **registers x0-x5**, so every syscall arg after the first was misplaced (1-arg `chdir` worked; multi-arg `mkdir`/`mount` got garbage path pointers). Fixed with a non-variadic 6-arg typedef for `__MACH__`. (On Linux/AAPCS64 both forms use registers, so upstream's variadic typedef happens to work.)
2. **`LKL_CONFIG_64BIT`** — the empty `lkl_autoconf.h` left it undefined, so the host UAPI headers thought the target was 32-bit (`__LKL__BITS_PER_LONG=32` → wrong syscall-arg marshalling + type sizes). Wrote a minimal `tools/lkl/include/lkl_autoconf.h` (`LKL_CONFIG_64BIT 1`, `JSON_TOKEN_MAX`, LE).
3. **`__param` section** — module params (e.g. `virtio_mmio.device=`, the cmdline that creates the virtio-blk device) were scattered to `.data` by the neutralized `__section`, so `start_kernel`'s `parse_args(__start___param..__stop___param)` saw an empty table and the disk was never registered. Re-sectioned via a `__MACH__` `__module_param_call` (raw `__attribute__((section("__DATA,__kparam")))`); glue brackets it as `__start/__stop___param`. Recompile each param-owning file (only `virtio_mmio.c` needed here).
4. **per-file `KBUILD_MODNAME`** — `mo_compile.sh` hard-codes `-DKBUILD_MODNAME='"x"'`, so the param registered as `x.device` not `virtio_mmio.device` (and all drivers logged as `'x'`). Recompiled `virtio_mmio.c` with the real modname so the boot-cmdline param matches.
5. **endian + thread-stack host shims** — `tools/lkl/lib/endian.h` got an `__APPLE__` branch (`OSSwap*` for `le*toh`/`htole*`); `posix-host.c thread_stack()` got an `__APPLE__` path (`pthread_get_stackaddr_np`+`pthread_get_stacksize_np`, no glibc `pthread_getattr_np`). Host files compile with `-isysroot $(xcrun --show-sdk-path) -DLKL_HOST_CONFIG_POSIX`.
6. `lib/strncpy_from_user.c` / `lib/strnlen_user.c` — forced byte-at-a-time for `__MACH__` (the word-at-a-time path has no exception table in this build); defensive, not strictly required once #1 landed.

Host objects for mount: `posix-host.o utils.o iomem.o jmp_buf.o virtio.o virtio_blk.o fs.o`. Disk created with `mke2fs -d <dir>` (container). **Next:** XFS/Btrfs images (need xfsprogs/btrfs-progs), then port this host layer into the xlinuxfs `.appex` (fd → `FSBlockDeviceResource`). GPLv2 unchanged (deferred).

---

## ✅✅ BOOT ACHIEVED (2026-06-27) — the kernel fully boots in-process on macOS

`lkl_start_kernel()` now **boots the real Linux 6.12 kernel to userspace `/init` and returns 0**, as a native Mach-O arm64 program on macOS (no loader, no VM, no FUSE). Boot log shows MM/zone/SLUB, IRQs, timers, clocksource, PF_NETLINK/INET/INET6, **`SGI XFS …`**, **`Btrfs loaded`**, io schedulers, then `Run /init as init process` → `lkl_start_kernel = 0`. Deterministic (clean exit 0 every run). ext4 is built-in (no banner). This moves the project from "feasible" to "booting".

### Build pipeline (all in `/path/to/lkl/.macboot/`, run on the Mac via hostexec)
- **Kernel object:** every kernel `.c` compiled to Mach-O via `/tmp/mo_compile.sh <path-no-ext>` (clang19 `--target=arm64-apple-darwin`, force-include `.hostinc/macho_section_compat.h`), then merged:
  `ld -r -arch arm64 -unexported_symbols_list /tmp/hide_syms.txt -filelist /tmp/macho_objs.txt -o /tmp/vmlinux_macho_ns.o`
  - `/tmp/hide_syms.txt` = `_memcpy _memset _memmove` (see fix #2).
  - `/tmp/macho_objs.txt` = the ~1047 `*.o.macho` **in correct kernel link order** (see fix #6).
- **Host side:** `posix-host.c utils.c iomem.c jmp_buf.c` compiled native (`-isysroot $(xcrun --show-sdk-path) -DLKL_HOST_CONFIG_POSIX`). Headers generated by `arch/lkl/scripts/headers_install.py` (needs GNU sed=`gsed` + native-built `scripts/unifdef`; `syscall_defs.h` via `objcopy -j .syscall_defs` from the ELF `vmlinux`).
- **Glue:** `.macboot/boot_glue.S` defines the ~73 non-kernel symbols (section boundaries, initcall placeholders, kallsyms/PCI stubs, `jiffies`/`blake2s` aliases, the `__setup` sentinel, sched_class boundaries, init_thread_union/init_stack aliases).
- **Link:** `bash .macboot/build.sh` (harness + kernel-ns + glue + host + libSystem). Run: `bash .macboot/run.sh`.

### The 6 root-cause fixes that took it from "links" to "boots" (in order hit)
1. **initcall iteration** (`init/main.c` `#if __MACH__`): ld64 doesn't lay out the per-level `__DATA,__ic<N>` sections contiguously, so the lds level-range walk is invalid — replaced with an explicit per-section walk via `section$start$/$end$`, skipping NULL placeholders.
2. **`memcpy`/`memset`/`memmove` infinite loop**: the kernel's host-redirect `_memcpy` thunk (→`lkl_ops->memcpy`=`posix_memcpy`) collides with libc; ld64 binds `posix_memcpy`'s own `memcpy` call to the kernel thunk → infinite tail-call self-loop. Fix: hide the 3 symbols in the `ld -r` merge (`-unexported_symbols_list`) so the host binds libc. (GNU `objcopy --localize-symbol` CORRUPTS Mach-O relocations — must use ld64's unexported list.)
3. **`current` == NULL** (crash in first `printk`→`vprintk_store`): glue stubbed `init_thread_union` as zeros, so `_current_thread_info->task` = NULL. Fix: alias `init_thread_union`/`init_stack` to the real, initialized `init_thread_info` (init/init_task.c, `.task=&init_task`); the union overlays them at one address.
4. **`obsolete_checksetup` strlen(NULL)**: it's a `do{}while`, so an empty `__setup` range still derefs `__setup_start->str`. Fix: glue points `__setup_start`==`__setup_end` at a benign sentinel `obs_kernel_param` (str never matches, func NULL).
5. **`sched_init` BUG_ON / scheduler**: `__section` was neutralized so the 4 sched_class structs scattered; the lds requires `&dl<&rt<&fair<&idle` contiguous. Fix: patched `DEFINE_SCHED_CLASS` (Mach-O) to emit into one `__DATA,__schedclass` section; split `idle.c` into `kernel/sched/build_idle.c` and reordered `build_policy.c` (deadline before rt) so object/emit order yields dl,rt (build_policy.o) + fair (fair.o) + idle (build_idle.o); glue brackets via `section$start$/$end$__DATA$__schedclass`.
6. **within-level initcall ordering** (crash: `sock_create_lite`, sock_mnt==NULL — netlink ran before sock_init): the merge filelist was alphabetical, not kernel link order. Fix: regenerated `/tmp/macho_objs.txt` from `ar t vmlinux.a` order (the true vmlinux link order) mapped to `.o.macho`, with the sched trio kept consecutive. This fixed ALL within-level ordering at once.

Plus a debug aid: `arch/lkl/kernel/console.c` `lkl_console_init` moved `core_initcall`→`early_initcall` so the buffered boot log flushes before `do_initcalls`.

### Next: mount a filesystem
Boot works; the harness just halts. To prove end-to-end FS access: compile `virtio.c`+`virtio_blk.c`, `lkl_disk_add` a backing image, `lkl_sys_mount` (ext4/xfs/btrfs), read a file. Then port the host layer into the **xlinuxfs** `.appex` (fd → `FSBlockDeviceResource`). GPLv2 still applies (kernel can't link into a closed App-Store appex) — deferred per user.

---

Scratch port tree: `/path/to/lkl` (Linux 6.12.0 + LKL, git clone).
Goal: build `liblkl` as **native Mach-O arm64** so the real Linux kernel (ext2/3/4, XFS, Btrfs, jbd2) links in-process into an Apple **FSKit `.appex`** for xext4 (read-write).

## TL;DR — verdict FLIPPED to FEASIBLE (bounded engineering, not research)

An earlier analysis called in-process LKL→Mach-O a NO-GO. **Hands-on measurement refutes that.** The Linux kernel C code **compiles to native Mach-O arm64**; the ONLY Mach-O-specific obstacles are **two mechanical, macro-funneled classes**, both with verified fixes:

1. **ELF section placement** — clang's Mach-O backend rejects ELF section names (`.init.text` etc.; needs `SEG,SECT`, ≤16 chars). Funneled through `__section()` + a few direct-attribute macros (initcall, percpu).
2. **Symbol aliases** — `__attribute__((alias()))` is "not supported on darwin"; comes almost entirely from `SYSCALL_DEFINEx`. Replaceable with Mach-O assembler `.set` (verified working).

No inline-asm nightmare, no codegen issues, no architectural blocker were found across 34 diverse files. **Why LKL specifically is tractable:** `arch/lkl` is a *virtual* arch implemented in **C** (no arch inline asm for atomics/barriers/bitops), and the asm-heavy kernel features (jump_label / ftrace / tracepoints / objtool / BTF) are **config-disabled**. That removes the exact code that would otherwise be a Mach-O porting nightmare.

## Hard evidence

Toolchain on the Mac (macOS 26.5, arm64):
- `/path/to/llvm19/bin` — LLVM/clang 19.1.6 (clang, ld.lld, llvm-objcopy). Emits ELF and Mach-O.
- Homebrew GNU binutils 2.46.1 at `/opt/homebrew/opt/binutils/bin` — objcopy has a working `mach-o-arm64` target.
- **CORRECTION on LKL's namespacing:** LKL does NOT bulk-rename symbols to `lkl_`. `arch/lkl/Makefile:91-94` does `objcopy -G$(prefix)<entry> … --prefix-symbols=$(prefix)` where `$(prefix)` is EMPTY on ELF (only `_` on PE-i386). So the real mechanism is **`-G` / `--keep-global-symbol`: keep only the ~11 `lkl_*` entry points global and LOCALIZE every other kernel symbol** (after `ld -r` resolves internal refs), so kernel `printk`/`memcpy`/… don't collide with the host libc. (My earlier "--prefix-symbols renames everything" was wrong; it works on Mach-O but isn't the primary mechanism.)
- **CORE MACH-O MECHANISMS NOW VALIDATED (standalone, on the Mac):**
  - **Section boundary symbols (replaces the linker script):** ld64 synthesizes `section$start$SEG$SECT` / `section$end$…`. Test: 3 ints placed in `__DATA,__initcl6`, iterated via `extern int x[] __asm("section$start$__DATA$__initcl6")` → got n=3 sum=66. This is the drop-in replacement for vmlinux.lds's `__initcall_start/_end` etc.
  - **Localize (real namespacing):** `objcopy -G _lkl_entry ns.o` → `_lkl_entry` stays `T`, others become `t` (local). Works on Mach-O. (ld64 `-exported_symbols_list` does NOT localize for `-r`, so use brew `objcopy -G`.)
- Apple `ld -r` merges Mach-O objects (verified). `ld.lld -r` also works.
- **END-TO-END back-end pipeline verified on REAL ext4 code:** 6 ext4 files (inode/balloc/extents/mballoc/dir/ialloc) → clean Mach-O `.o` → `ld -r` merge (one Mach-O object, 236 ext4 functions) → `objcopy --prefix-symbols=lkl_` (all symbols namespaced: `lkl__ext4_alloc_da_blocks` …, still valid Mach-O) → `ar` → a 253 KB `liblkl_ext4.a`. So the whole chain real-kernel-C → Mach-O → merge → namespace → static lib works on the Mac. Only the runtime-correct section map + Darwin host backend remain to make it a *loadable, functioning* liblkl.
- ELF→Mach-O *object conversion* via `objcopy -O mach-o-arm64` is a **façade** (malformed output; ld64 crashes; section bodies drop to size 0) — so you cannot compile-to-ELF-then-convert; the kernel must compile **directly** to Mach-O. That's what the section/alias work enables.

Compile measurements (clang19 `--target=arm64-apple-darwin`, real kernel headers from `make ARCH=lkl prepare`):
- `fs/ext4/inode.c` (~6000 lines): ELF control = 0 errors. Mach-O raw = **445 errors, ALL one class** (section format). With a tiny section-compat header (+1 percpu patch): **0 errors → clean `Mach-O 64-bit object arm64`, 219 symbols incl `_ext4_block_write_begin` etc.**
- Broad sweep, 34 files across ext4/jbd2/fs/mm/kernel/block/lib (+xfs/btrfs): **14 compiled clean to Mach-O**; every failure was one of: (a) more section macros (same mechanical class), (b) `alias` (the syscall macro), (c) **false alarms** — `mm/vmalloc.c`/`mm/mmap.c` use MMU page-table identifiers but LKL is **non-MMU** (those files aren't in LKL's object set); `fs/xfs/xfs_super.c` just needs `-Ifs/xfs` + XFS enabled. `fs/btrfs/super.c` failed on a section error only → Btrfs behaves like ext4.

The two fix classes, verified:
- Section: redefining `__section(s)` (+ percpu `__PCPU_ATTRS`) made ext4/inode.c compile to a valid Mach-O `.o`. A real port maps each of ~62 distinct ELF sections to its own ≤16-char Mach-O section (text vs data) and aliases the 72 `__start_/__stop_` boundary symbols to ld64's `section$start$/$end$` synthetics.
- Alias: `__asm__(".globl _alias_fn\n.set _alias_fn, _real_fn")` compiles on Mach-O and produces both symbols. **VERIFIED on real kernel code:** redefining the `#ifndef`-guarded `__SYSCALL_DEFINEx` (include/linux/syscalls.h:249) to emit `.set` instead of `__attribute__((alias))` eliminated ALL alias errors — fs/open.c 22→0, namei/read_write/exit likewise → 0, with **zero "other" errors** (only the section class remains). So both Mach-O-specific classes are confirmed mechanically fixable on real kernel sources (sections via ext4/inode.c→clean .o; aliases via the syscall macro).

## Remaining real work to a loadable Mach-O liblkl (all bounded, none research-grade)

1. **Section map (the meatiest — CRUX is the linker script):** LKL gathers sections + defines boundary symbols (`__initcall_start/_end`, `__setup_start/_end`, `__start___param`, … — 72 distinct) via a GNU **linker script** `arch/lkl/kernel/vmlinux.lds.S` (→ `asm-generic/vmlinux.lds.h`) used with `ld -r`. **macOS ld64 has NO linker scripts** → must replace that whole mechanism with: (a) map each runtime-iterated ELF section to a Mach-O `SEG,SECT`; (b) alias each `__start_X`/`__stop_X` to ld64's auto-generated `section$start$SEG$SECT`/`section$end$…` synthetics (resolve at final `.appex` link); (c) preserve **initcall level ordering** (`.initcall0..7s.init` are concatenated in order by the lds — Mach-O needs the levels kept ordered, e.g. separate sections iterated 0→7). Cosmetic sections (`.data.once`, `.text.unlikely`, …) can collapse to defaults. Initcall uses the `__attribute__((__section__))` form (HAVE_ARCH_PREL32_RELOCATIONS unset) — easy to redirect; the hard part is the boundary/ordering scheme. — the dominant multi-day piece; needs iterative boot debugging.
2. **Alias macros → `.set`:** redefine `SYSCALL_DEFINEx` family for `__MACH__`. — small, verified.
3. **Darwin host backend (smaller than expected — scoped from `posix-host.c`):** MOST of it works on macOS unchanged — pthread threads/mutexes(incl recursive)/TLS, `clock_gettime(CLOCK_MONOTONIC)`, `mmap(MAP_ANON)`, `preadv/pwritev/fsync` block I/O, and `jmp_buf.c` is plain `setjmp/longjmp` (portable). Only real piece: **timers** — `timer_create`/`timer_settime` don't exist on macOS → reimplement the 3 timer ops with **GCD `dispatch_source` timers** (~50 lines). Plus tiny fixes: force the **existing** pthread_cond semaphore fallback (`#undef _POSIX_SEMAPHORES` on `__APPLE__`; macOS `sem_init` is unsupported but the `#else` branch in posix-host.c already implements sems via mutex+cond), and a few mmap macro aliases (`MAP_ANONYMOUS`/`MAP_FIXED_NOREPLACE`/`MAP_NORESERVE`). **~1–2 days**, not a week. (The block backend is also exactly where xext4 swaps the fd for `FSBlockDeviceResource`.)
4. **Build orchestration to Mach-O (concretely characterized):** you CANNOT just point the kernel CC at `--target=arm64-apple-darwin` and run kbuild — kbuild's **build machinery is ELF-bound**: `scripts/mod/mk_elfconfig` reads `scripts/mod/empty.o` and dies `Error: not ELF` on a Mach-O object; `modpost` parses ELF symbol tables of `vmlinux.o`. Practical path = **HYBRID**: run kbuild in **ELF mode** for orchestration (it computes the per-config object list + exact per-file flags + builds the ELF `vmlinux.o` modpost wants), then compile the kernel sources to **Mach-O separately** (replay flags with `--target=arm64-apple-darwin` + the compat header) and **assemble by hand** — `ld -r` (Apple ld) → `objcopy -G <lkl_entry_points>` (brew binutils; localizes the rest) → `ar` → `liblkl.a`. All three assembly steps verified on real ext4 objects. (Alternative: make mk_elfconfig/modpost Mach-O-aware — bigger.) The `arch/lkl/Makefile:19` `mach-o` gate + `Makefile.autoconf` host entry are only needed if going fully-kbuild instead.
5. **Host tools on macOS — DONE.** `make ARCH=lkl … prepare` now runs **fully Mac-native** (clang19 + Apple `HOSTCC`): all host tools (fixdep/kallsyms/mk_elfconfig/modpost/file2alias/sumversion/symsearch) build + link on macOS. Shims: `.hostinc/{elf.h,byteswap.h,endian.h}` (force-included via `HOSTCFLAGS=-I.hostinc`) + a 3-line `__APPLE__` patch in `scripts/mod/file2alias.c` (rename its `uuid_t`/`guid_t` to dodge macOS's array `uuid_t`). No container needed.
6. **Final assembly:** `ld -r` the Mach-O kernel objects → `objcopy --prefix-symbols=lkl_` (GNU binutils, works on Mach-O) → `liblkl.a` → link into the `.appex`. Integration side (`lkl_sys_*`, FSBlockDeviceResource→lkl_disk, one DispatchQueue) is fully mapped; `lklfuse.c` is the wiring template (FUSE itself not needed).

**Rough total: ~1–2 weeks of focused kernel-build engineering**, dominated by the runtime-correct **section map** (item 1). Everything else is days-or-known: aliases (verified), Darwin host (~1–2 days, scoped above), build-system mach-o path (small), host-tool shims (small). The compile + back-end (ld -r → prefix-symbols → archive) are already proven on real ext4 code. Not the "months / research-grade / no precedent" the earlier analysis claimed.

## The other (non-technical) blocker: GPL vs App Store

ext4/jbd2/XFS/Btrfs/kernel are **GPLv2**. Linking GPLv2 into a closed `.appex` distributed via the **App Store** is a license/terms conflict (this is why lwext4 (MIT) was the earlier pragmatic pick — but lwext4 can't do XFS/Btrfs). Options if pursuing LKL: ship open-source/GPL outside the App Store (Developer-ID + notarization), or run the kernel **out-of-process** (Virtualization.framework / a helper) so GPL code isn't linked into the app — cleaner license boundary, also gives the real kernel. The in-process technical feasibility proven here does not resolve the licensing question.

## MILESTONE (this session): the whole ext4 kernel is now ONE Mach-O object

Full ELF build on the Mac succeeded (1053 objects → `vmlinux`, 0 errors), proving the entire LKL ext4 kernel compiles. Then the **hybrid Mach-O build**: replayed every kernel `.c` with `--target=arm64-apple-darwin` + the compat header → **1046/1050 (99.6%) compiled clean to Mach-O** (the 4 left are 3 PCI-quirk files + version.c, none needed for ext4-over-virtio). `ld -r` merged all 1046 → a single **`Mach-O 64-bit object arm64`, ~15 MB, 60025 defined symbols, 83 undefined**. `_lkl_start_kernel`/`_lkl_syscall`/`_ext4_fill_super` are all defined. So the kernel-side spine is DONE on macOS.

The compile tail that got handled (all macro-funneled, `__MACH__`/`__APPLE__`-guarded): `__section` + percpu + initcall + `__cacheline_aligned` + `__noinstr_section` (sections), `SYSCALL_DEFINEx` + `__alias` (aliases via `.set`), `cond_syscall` (weak function stub `b _sys_ni_syscall` — `.weak_definition`+`.set` makes an *indirect* symbol that ld -r rejects as a dup, so a weak stub is required), `EXPORT_SYMBOL` (`-D__DISABLE_EXPORTS`, like the PE port), `__SYSCALL_DEFINE_ARCH`/PCI-fixup/BUILD_SALT (ELF inline-asm `.section`).

**The 83 undefined symbols ARE the precise remaining spec**, dominated by the linker-script boundary symbols (since the sections were neutralized for compile):
- **Section-map boundaries (~55):** `__initcall{0..7}_start`/`__initcall_start/_end`, `__setup_start/_end`, `__con_initcall_*`, `__start___param/__stop___param`, `__start/__stop___ex_table`, `__{s,e}text`, `__sinittext/__einittext`, `__init_begin/_end`, `__bss_start/stop`, `__sched/irqentry/softirqentry/cpuidle_text_*`, `jiffies`, `init_thread_union`, … — provide via ld64 `section$start$SEG$SECT` aliases (validated) once initcalls/setup/etc. are placed in real Mach-O sections, + a small init/main.c patch to iterate per-level initcall sections.
- **host/libc (~10):** `lkl_printf`/`lkl_bug` (host side), `__stack_chk_fail/guard` (`-fno-stack-protector` or libc), `bzero` (libc), kallsyms_* (CONFIG_KALLSYMS off or stub).
- **stubs (~8):** PCI-quirk hooks + `vmalloc_huge_noprof`/`blake2s_compress` (the neutralized `__alias`es) → `.set` stubs or compile the 4 skipped files.

## Section-map mechanism validated on a REAL initcall

`__define_initcall(fn, id)` patched (`__MACH__`) to place each initcall in `__DATA,__ic<id>`. Verified: `fs/ext4/super.c` → `ext4_init_fs` (device_initcall=6) lands in section `__DATA,__ic6` with `___mach_ic_ext4_init_fs_6` at its start. After `ld -r`, `section$start$__DATA$__ic6`/`section$end$…` bracket every level-6 initcall — the drop-in for the lds's `__initcall6_start`. VALIDATED AT SCALE: re-merging all 1046 objects yields a kernel object with `__icearly`(9), `__ic0`(2), `__ic1`(14), `__ic2`(10), `__ic4`(38), `__ic5`(35), `__ic6`(118 — ext4/jbd2/drivers), `__ic7`(27), `__ic7s`(2), `__icrootfs`(1) — exactly the per-level initcall tables, now gathered as Mach-O sections. So EVERY mechanism the port needs is now individually proven on macOS: compile→Mach-O, alias→`.set`, weak→stub, namespacing→`objcopy -G`, the `ld -r` full-kernel merge, AND section gathering+boundaries. What's left is wiring (an `init/main.c` `do_initcalls()` patch to walk the per-`__ic<id>` sections in order early→0→0s→…→7→7s; a glue object aliasing the remaining boundary/position symbols + `jiffies`; compiling the Darwin host side; libc + a few `.set` stubs) and then iterative **boot** bring-up — runtime debugging, the genuinely open-ended part.

## What is PROVEN vs NOT

PROVEN: kernel C (ext4 fully, +13 diverse files) compiles to native Mach-O arm64 with only the 2 mechanical fix classes; the toolchain steps (prefix-symbols, ld -r) work on Mach-O.
NOT YET DONE: a runtime-correct section map; ld -r + prefix-symbols over the *full* kernel object set into one liblkl; the Darwin host port; actually running/mounting a filesystem. These are bounded next steps, not blockers.

## Reproduction (everything on the Mac; no container)

Mac-native build env (host tools + headers), runs clean on macOS:
```
CL=/path/to/llvm19/bin
cd /path/to/lkl
gmake ARCH=lkl LLVM=$CL/ CLANG_TARGET_FLAGS_lkl=aarch64-linux-gnu CROSS_COMPILE=aarch64-linux-gnu- \
  HOSTCC=/usr/bin/clang HOSTCFLAGS=-I.hostinc prepare
```
(`gmake` = Homebrew GNU make 4.4; Apple make 3.81 is too old. `CONFIG_OUTPUT_FORMAT` is forced to `elf64-littleaarch64` via the patched detection script so the config/headers generate; the Mach-O compile is then driven per-file with `--target=arm64-apple-darwin`.) To re-measure a single file:
```
CL=/path/to/llvm19/bin
cd /path/to/lkl
$CL/clang --target=arm64-apple-darwin -ffreestanding -nostdinc \
  -I./arch/lkl/include -I./arch/lkl/include/generated -I./include \
  -I./arch/lkl/include/uapi -I./arch/lkl/include/generated/uapi \
  -I./include/uapi -I./include/generated/uapi \
  -include ./include/linux/compiler-version.h -include ./include/linux/kconfig.h \
  -include ./include/linux/compiler_types.h \
  -include ./.hostinc/macho_section_compat.h \
  -D__KERNEL__ -DKBUILD_MODNAME='"x"' -DKBUILD_BASENAME='"x"' -DKBUILD_MODFILE='"x"' -D__KBUILD_MODNAME=kmod_x \
  -std=gnu11 -fno-common -fno-strict-aliasing -fno-builtin -fPIC -O2 -w -ferror-limit=0 \
  -c -o /tmp/out.o fs/ext4/inode.c
```

### PoC modifications made in this tree (measurement scaffolding, NOT a real port)
- `.hostinc/macho_section_compat.h` — neutralizes `__section`/percpu sections under `__MACH__` (force-included). PoC only; real port maps sections.
- `include/linux/percpu-defs.h` — `__PCPU_ATTRS` wrapped in `#if defined(__MACH__)` to drop percpu section. (3-line edit; `git diff` shows it.)
- `arch/lkl/scripts/cc-objdump-file-format.sh` — forced to `echo elf64-littleaarch64` so the ELF config could generate headers (the real detection produced `mach-o`, which the build then rejected — itself a finding). Revert for a clean tree.
- `.hostinc/{elf.h,byteswap.h,endian.h}` — macOS host-tool shims (force-included via `HOSTCFLAGS=-I.hostinc`).
- `scripts/mod/file2alias.c` — 3-line `#ifdef __APPLE__` block renaming its `uuid_t`/`guid_t` to dodge macOS's array `uuid_t` (lets host tools build on macOS). Real change a port keeps.
- `include/linux/syscalls.h` path is NOT patched — the alias fix lives in the force-included `.hostinc/macho_section_compat.h` (`__SYSCALL_DEFINEx` is `#ifndef`-guarded, so the compat header's `.set`-based version wins).

These are reversible scaffolding; `git status`/`git diff` in the tree shows the full set.
