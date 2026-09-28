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
ext2/3), XFS and Btrfs enabled. `profile/objects.json` records 1047 source files,
their Kbuild include/define flags, and link order. The generated kernel and
public UAPI headers are versioned text inputs tied to this configuration.
These are build inputs, not compiled objects. The build replays them for both
Darwin architectures and never reads the original `.cmd`, `.macboot`,
`.hostinc`, `vmlinux.a`, or `.o.macho` files.

The profile was captured from the working ELF preparation of upstream commit
`e50792e72` plus the Darwin patches. The exporter replaces personal build
identity and timestamps with stable values. To change the kernel configuration,
regenerate the ELF preparation and Darwin object selection, run
`export-profile.py --prepared /path/to/prepared/tree --llvm-ar /path/to/llvm-ar`,
and review/commit the entire profile together. This is a fixed-configuration
build, not a general replacement for Kconfig/Kbuild.

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
