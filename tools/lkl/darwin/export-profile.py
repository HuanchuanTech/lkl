#!/usr/bin/env python3
"""Capture the fixed Darwin profile from a prepared ELF/Mach-O bring-up tree.

This maintainer tool is not part of the normal build. Review the exported
configuration, object order and generated headers together when updating LKL.
"""
import argparse
import json
from pathlib import Path
import shlex
import shutil
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--prepared", type=Path, required=True)
    parser.add_argument("--llvm-ar", required=True)
    args = parser.parse_args()
    source = args.prepared.resolve()
    output = Path(__file__).resolve().parent / "profile"
    output.mkdir(parents=True, exist_ok=True)
    members = []
    for archive in ("vmlinux.a", "lib/lib.a"):
        members.extend(subprocess.check_output(
            [args.llvm_ar, "t", archive], cwd=source, text=True).splitlines())
    members.append("init/version-timestamp.o")
    members = [name for name in members if (source / (name + ".macho")).is_file()]
    sched = ["kernel/sched/build_policy.o", "kernel/sched/fair.o", "kernel/sched/build_idle.o"]
    insertion = min(members.index(name) for name in sched)
    members = [name for name in members if name not in sched]
    members[insertion:insertion] = sched
    objects = []
    for member in members:
        obj = source / member
        command = obj.with_name("." + obj.name + ".cmd").read_text().splitlines()[0]
        tokens = shlex.split(command.split(":=", 1)[1])
        unit = next(t for t in reversed(tokens) if t.endswith(".c"))
        flags = []
        i = 1
        while i < len(tokens):
            token = tokens[i]
            if token in ("-include", "-I", "-isystem"):
                flags.extend(tokens[i:i + 2])
                i += 2
                continue
            if token.startswith(("-I", "-D", "-U")):
                flags.append(token)
            i += 1
        if any(str(source.parent) in flag for flag in flags):
            raise ValueError(f"Nonportable flags for {unit}")
        objects.append({"object": member, "source": unit, "flags": flags})
    (output / "objects.json").write_text(json.dumps(objects, indent=2) + "\n")
    generated = ["lib/crc32table.h", "lib/raid6/tables.c"]
    generated.extend(f"lib/raid6/int{n}.c" for n in (1, 2, 4, 8))
    for name in generated:
        target = output / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source / name, target)
    shutil.copyfile(source / ".config", output / "kernel.config")
    for directory in ("include/generated", "arch/lkl/include/generated"):
        for header in (source / directory).rglob("*.h"):
            target = output / header.relative_to(source)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(header, target)
    for header in (source / "tools/lkl/include").rglob("*.h"):
        if "mingw32" in header.parts:
            continue
        target = output / "host-include" / header.relative_to(source / "tools/lkl/include")
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(header, target)
    # Stable build identity: no developer name, host name, or local build time.
    (output / "include/generated/compile.h").write_text(
        '#define UTS_MACHINE "lkl"\n#define LINUX_COMPILE_BY "lkl"\n'
        '#define LINUX_COMPILE_HOST "darwin"\n#define LINUX_COMPILER "LLVM 19"\n')
    (output / "include/generated/utsversion.h").write_text(
        '#define UTS_VERSION "#1 Darwin filesystem profile"\n')
    print(f"Exported {len(objects)} objects and generated headers to {output}")


if __name__ == "__main__":
    main()
