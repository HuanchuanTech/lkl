# Darwin filesystem build

This directory owns the native Mach-O arm64/x86_64 LKL build used by xlinuxfs.
Requires macOS, Xcode command-line tools, Python 3.9+, and upstream LLVM 19.
No Linux VM, prepared sibling checkout, or prebuilt kernel object is required.

```sh
python3 tools/lkl/darwin/build.py --clang /path/to/llvm19/bin/clang \
  --output /path/to/output --work /path/to/build-work
```

Outputs `liblkl.a` (both architectures, minimum macOS 15.4) and matching
public headers in `include/`. Build work belongs outside the source tree.

## Fixed profile

`profile/kernel.config` is the Linux 6.12 LKL configuration with ext4 (including
ext2/3), XFS and Btrfs enabled. `profile/objects.json` records 714 source files,
their Kbuild include/define flags, and link order. The generated kernel and
public UAPI headers are versioned text inputs tied to this configuration.
These are build inputs, not compiled objects. The build replays them for both
Darwin architectures and never reads the original `.cmd`, `.macboot`,
`.hostinc`, `vmlinux.a`, or `.o.macho` files.

The profile was captured from the working ELF preparation of upstream commit
`e50792e72` plus the Darwin patches. The exporter replaces personal build
identity and timestamps with stable values. To change the kernel configuration,
regenerate the ELF build and its installed headers, run
`export-profile.py --prepared /path/to/prepared/tree --llvm-ar /path/to/llvm-ar`,
and review/commit the entire profile together. This is a fixed-configuration
build, not a general replacement for Kconfig/Kbuild.

The filesystem-only profile disables networking, FAT/VFAT and NLS, PCI/NVMe,
input/HID/serio and TTY. It retains virtio-mmio block I/O, procfs (held-inode
reopening), sysfs (block-device discovery), all three filesystems' existing
features, and Btrfs compression/checksum/RAID helpers. Tracing remains selected
by the LKL architecture. Darwin code still uses `-O2`; this reduction does not
depend on more aggressive optimization, stripping, or dropping an architecture.

For profile maintenance, use an in-tree LLVM 19 ELF build on case-sensitive
storage. Start from `profile/kernel.config`, change options with `scripts/config`,
then run Kbuild's `olddefconfig`, `all`, and `install INSTALL_PATH=tools/lkl`
targets with `ARCH=lkl`. Use `CC="clang-19 --target=aarch64-linux-gnu"`,
`CLANG_TARGET_FLAGS_lkl=aarch64-linux-gnu`, and `LLVM=-19` for the preparation.
Linux can run this directly; macOS also needs the ELF host-tool headers and
GNU make/sed used during the original port preparation. None of these maintainer
dependencies are needed by the normal Darwin build.

The exporter reads every object in Kbuild link order, except the four ELF units
explicitly replaced by `boot_glue.S`; it does not look for stale `.o.macho` files.
`--output /path/to/candidate-profile` can export a comparison without replacing
the checked-in profile. Generated headers are replaced rather than overlaid,
and host ABI/build identity headers are generated with stable values. Validate
the profile with `python3 tools/lkl/darwin/test_profile.py`, then build and run
the consuming application's filesystem tests on both architectures.

`boot_glue.S` supplies the linker-script boundaries and profile-specific stubs.
Initcall order follows Kbuild; scheduler classes must remain dl, rt, fair, idle.
The three kernel memory routines are hidden at the partial link so the host
implementation calls libc instead of recursively resolving the kernel thunks.

Linux has case-only filename pairs. A case-insensitive checkout can report
unrelated netfilter/litmus-test modifications; do not commit those as port
patches. The fixed profile does not compile those conflicting files. Use a
case-sensitive filesystem for general kernel development or configuration changes.

The original bring-up notes and binaries remain local historical artifacts;
they are not inputs to this supported build entry point.
