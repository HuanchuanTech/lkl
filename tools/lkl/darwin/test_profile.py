#!/usr/bin/env python3
"""Check the fixed filesystem profile without compiling the kernel."""
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest


HERE = Path(__file__).resolve().parent
PROFILE = HERE / "profile"
SOURCE = HERE.parents[2]


class ProfileTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.config = dict(re.findall(r"^(CONFIG_\w+)=(.*)$",
                                    (PROFILE / "kernel.config").read_text(), re.MULTILINE))
        cls.objects = json.loads((PROFILE / "objects.json").read_text())

    def test_required_filesystem_features(self):
        required = (
            "64BIT", "EXT4_FS", "EXT4_USE_FOR_EXT2", "EXT4_FS_POSIX_ACL",
            "EXT4_FS_SECURITY", "XFS_FS", "XFS_POSIX_ACL", "XFS_SUPPORT_V4",
            "BTRFS_FS", "BTRFS_FS_POSIX_ACL", "PROC_FS", "SYSFS",
            "VIRTIO_BLK", "VIRTIO_MMIO", "VIRTIO_MMIO_CMDLINE_DEVICES",
            "ZLIB_INFLATE", "ZLIB_DEFLATE", "LZO_COMPRESS", "LZO_DECOMPRESS",
            "ZSTD_COMPRESS", "ZSTD_DECOMPRESS", "CRYPTO_CRC32C", "CRYPTO_XXHASH",
            "CRYPTO_SHA256", "CRYPTO_BLAKE2B", "RAID6_PQ", "XOR_BLOCKS",
        )
        for name in required:
            with self.subTest(name=name):
                self.assertEqual(self.config.get("CONFIG_" + name), "y")

    def test_unused_subsystems_disabled(self):
        for name in ("NET", "PCI", "FAT_FS", "VFAT_FS", "NLS", "INPUT",
                     "HID", "TTY", "SERIO", "BLK_DEV_NVME"):
            with self.subTest(name=name):
                self.assertNotIn(self.config.get("CONFIG_" + name), ("y", "m"))
        prefixes = ("net/", "fs/fat/", "fs/nls/", "drivers/pci/", "drivers/nvme/",
                    "drivers/input/", "drivers/hid/", "drivers/tty/", "drivers/input/serio/")
        for member in self.objects:
            self.assertFalse(member["source"].startswith(prefixes), member["source"])

    def test_generated_config_matches(self):
        generated = dict(re.findall(r"^#define (CONFIG_\w+) (.*)$",
                                    (PROFILE / "include/generated/autoconf.h").read_text(),
                                    re.MULTILINE))
        expected = {name: "1" if value == "y" else value
                    for name, value in self.config.items()}
        self.assertEqual(generated, expected)

    def test_manifest_is_portable_and_complete(self):
        names = [member["object"] for member in self.objects]
        self.assertEqual(len(names), len(set(names)))
        for member in self.objects:
            for key in ("source", "object"):
                path = Path(member[key])
                self.assertFalse(path.is_absolute())
                self.assertNotIn("..", path.parts)
            self.assertTrue((PROFILE / member["source"]).is_file() or
                            (SOURCE / member["source"]).is_file(), member["source"])
            self.assertNotIn("/Users/", " ".join(member["flags"]))
        self.assertIn("init/version-timestamp.o", names)
        self.assertNotIn("init/version.o", names)
        scheduler = ["kernel/sched/build_policy.o", "kernel/sched/fair.o",
                     "kernel/sched/build_idle.o"]
        start = names.index(scheduler[0])
        self.assertEqual(names[start:start + len(scheduler)], scheduler)

    def test_host_abi_and_stable_identity(self):
        abi = (PROFILE / "host-include/lkl_autoconf.h").read_text()
        self.assertIn("#define LKL_CONFIG_64BIT 1", abi)
        self.assertNotIn("LKL_CONFIG_CPU_BIG_ENDIAN", abi)
        identity = (PROFILE / "include/generated/compile.h").read_text()
        self.assertIn('#define LINUX_COMPILE_BY "lkl"', identity)
        self.assertIn('#define LINUX_COMPILE_HOST "darwin"', identity)
        self.assertEqual((PROFILE / "include/generated/utsversion.h").read_text(),
                         '#define UTS_VERSION "#1 Darwin filesystem profile"\n')

    def test_host_headers_match_sources(self):
        for name in ("lkl.h", "lkl_host.h", "lkl_config.h"):
            self.assertEqual((PROFILE / "host-include" / name).read_bytes(),
                             (SOURCE / "tools/lkl/include" / name).read_bytes())

    @unittest.skipUnless(shutil.which("cc"), "A host C compiler is needed")
    def test_network_wrappers_without_generated_socket_helpers(self):
        with tempfile.TemporaryDirectory(prefix="lkl-host-wrappers-") as directory:
            source = Path(directory) / "test.c"
            program = Path(directory) / "test"
            source.write_text('''#include <assert.h>
#include <lkl.h>
static char buffer[16];
static long expected;
long lkl_syscall(long number, long *args)
{
    assert(number == expected);
    assert(args[0] == 42 && args[1] == (long)buffer);
    assert(args[2] == sizeof(buffer) && args[3] == 7);
    assert(args[4] == 0 && args[5] == 0);
    return -LKL_ENOSYS;
}
int main(void)
{
    expected = __lkl__NR_sendto;
    assert(lkl_sys_send(42, buffer, sizeof(buffer), 7) == -LKL_ENOSYS);
    expected = __lkl__NR_recvfrom;
    assert(lkl_sys_recv(42, buffer, sizeof(buffer), 7) == -LKL_ENOSYS);
    return 0;
}
''')
            subprocess.run(["cc", "-std=c11", "-Werror=implicit-function-declaration",
                            "-I" + str(PROFILE / "host-include"), str(source),
                            "-o", str(program)], check=True)
            subprocess.run([str(program)], check=True)


if __name__ == "__main__":
    unittest.main()
