#!/usr/bin/env python3
"""Build the fixed filesystem profile as a universal macOS static library."""
import argparse
import concurrent.futures
import json
import os
from pathlib import Path
import shutil
import subprocess


def main():
    here = Path(__file__).resolve().parent
    source = here.parents[2]
    profile = here / "profile"
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--clang", default=os.environ.get("CLANG", "clang"))
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--work", type=Path, required=True)
    parser.add_argument("--jobs", type=int, default=min(os.cpu_count() or 4, 8))
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    compiler = shutil.which(args.clang)
    if not compiler:
        parser.error("LLVM clang 19 is required; pass --clang /path/to/clang")
    version = subprocess.check_output([compiler, "--version"], text=True)
    if "clang version 19." not in version or "Apple clang" in version:
        parser.error("This fixed profile requires upstream LLVM clang 19")
    args.output = args.output.resolve()
    args.work = args.work.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    args.work.mkdir(parents=True, exist_ok=True)
    sdk = subprocess.check_output(["xcrun", "--show-sdk-path"], text=True).strip()
    sdk_version = subprocess.check_output(["xcrun", "--show-sdk-version"], text=True).strip()
    members = json.loads((profile / "objects.json").read_text())

    def run(command, **kwargs):
        return subprocess.run(command, cwd=source, check=True, text=True, **kwargs)

    archives = []
    for arch in ("arm64", "x86_64"):
        work = args.work / arch
        work.mkdir(parents=True, exist_ok=True)
        target = [f"--target={arch}-apple-macos15.4", "-mmacosx-version-min=15.4"]

        def compile_object(member):
            output = work / member["object"]
            output.parent.mkdir(parents=True, exist_ok=True)
            # Generated headers precede the source tree. No in-tree .config,
            # .cmd, .o, or generated include directory is consulted.
            flags = []
            for flag in member["flags"]:
                if flag.startswith("-I") and "generated" in flag:
                    flags.append("-I" + str(profile / flag[2:].removeprefix("./")))
                else:
                    flags.append(flag)
            command = [compiler, *target, "-ffreestanding", "-nostdinc",
                       "-I" + str(profile / "include"), *flags,
                       "-include", str(here / "macho_section_compat.h"),
                       "-D__DISABLE_EXPORTS", "-std=gnu11", "-fshort-wchar",
                       "-funsigned-char", "-fno-common", "-fno-strict-aliasing",
                       "-fno-builtin", "-fPIC", "-fno-stack-protector", "-O2", "-w",
                       "-c", member["source"], "-o", str(output)]
            result = subprocess.run(command, cwd=source, text=True, capture_output=True)
            output.with_suffix(".log").write_text(result.stdout + result.stderr)
            if result.returncode:
                raise RuntimeError(f"{member['source']}: {result.stderr[:2000]}")

        print(f"Compiling {len(members)} LKL objects for {arch}", flush=True)
        failures = []
        with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as executor:
            futures = [executor.submit(compile_object, member) for member in members]
            for index, future in enumerate(concurrent.futures.as_completed(futures), 1):
                try:
                    future.result()
                except Exception as error:
                    failures.append(str(error))
                    print(error, flush=True)
                if index % 100 == 0:
                    print(f"{arch}: {index}/{len(members)}", flush=True)
        if failures:
            raise SystemExit(f"{len(failures)} compilation failures; see {work}")
        objects = [work / m["object"] for m in members]
        filelist = work / "objects.txt"
        filelist.write_text("\n".join(map(str, objects)) + "\n")
        hidden = work / "hidden.txt"
        hidden.write_text("_memcpy\n_memset\n_memmove\n")
        kernel = work / "vmlinux_macho.o"
        run(["xcrun", "ld", "-r", "-arch", arch, "-platform_version", "macos", "15.4",
             sdk_version, "-unexported_symbols_list", str(hidden), "-filelist", str(filelist),
             "-o", str(kernel)])
        boot = work / "boot_glue.o"
        run([compiler, *target, "-c", str(here / "boot_glue.S"), "-o", str(boot)])
        host_objects = [kernel, boot]
        for name in ("posix-host", "utils", "iomem", "jmp_buf", "virtio", "virtio_blk", "fs"):
            obj = work / (name + ".o")
            run([compiler, *target, "-isysroot", sdk, "-O2", "-g",
                 "-I" + str(profile / "host-include"), "-I" + str(source / "tools/lkl/lib"),
                 "-D_FILE_OFFSET_BITS=64", "-DLKL_HOST_CONFIG_POSIX", "-c",
                 str(source / "tools/lkl/lib" / (name + ".c")), "-o", str(obj)])
            host_objects.append(obj)
        archive = work / "liblkl.a"
        run(["xcrun", "libtool", "-static", "-o", str(archive), *map(str, host_objects)])
        archives.append(archive)
        for obj in set(objects + host_objects):
            obj.unlink()
    temporary = args.output / "liblkl.new.a"
    run(["xcrun", "lipo", "-create", *map(str, archives), "-output", str(temporary)])
    run(["xcrun", "lipo", "-verify_arch", "arm64", "x86_64", str(temporary)])
    shutil.copytree(profile / "host-include", args.output / "include", dirs_exist_ok=True)
    temporary.replace(args.output / "liblkl.a")
    for archive in archives:
        archive.unlink()
    print(f"Built {args.output / 'liblkl.a'}", flush=True)


if __name__ == "__main__":
    main()
