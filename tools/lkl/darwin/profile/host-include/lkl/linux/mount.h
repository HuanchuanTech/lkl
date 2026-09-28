#ifndef _LKL_LINUX_MOUNT_H
#define _LKL_LINUX_MOUNT_H

#include <lkl/linux/types.h>

/*
 * These are the fs-independent mount-flags: up to 32 flags are supported
 *
 * Usage of these is restricted within the kernel to core mount(2) code and
 * callers of sys_mount() only.  Filesystems should be using the SB_*
 * equivalent instead.
 */
#define LKL_MS_RDONLY	 1	/* Mount read-only */
#define LKL_MS_NOSUID	 2	/* Ignore suid and sgid bits */
#define LKL_MS_NODEV	 4	/* Disallow access to device special files */
#define LKL_MS_NOEXEC	 8	/* Disallow program execution */
#define LKL_MS_SYNCHRONOUS	16	/* Writes are synced at once */
#define LKL_MS_REMOUNT	32	/* Alter flags of a mounted FS */
#define LKL_MS_MANDLOCK	64	/* Allow mandatory locks on an FS */
#define LKL_MS_DIRSYNC	128	/* Directory modifications are synchronous */
#define LKL_MS_NOSYMFOLLOW	256	/* Do not follow symlinks */
#define LKL_MS_NOATIME	1024	/* Do not update access times. */
#define LKL_MS_NODIRATIME	2048	/* Do not update directory access times */
#define LKL_MS_BIND		4096
#define LKL_MS_MOVE		8192
#define LKL_MS_REC		16384
#define LKL_MS_VERBOSE	32768	/* War is peace. Verbosity is silence.
				   LKL_MS_VERBOSE is deprecated. */
#define LKL_MS_SILENT	32768
#define LKL_MS_POSIXACL	(1<<16)	/* VFS does not apply the umask */
#define LKL_MS_UNBINDABLE	(1<<17)	/* change to unbindable */
#define LKL_MS_PRIVATE	(1<<18)	/* change to private */
#define LKL_MS_SLAVE	(1<<19)	/* change to slave */
#define LKL_MS_SHARED	(1<<20)	/* change to shared */
#define LKL_MS_RELATIME	(1<<21)	/* Update atime relative to mtime/ctime. */
#define LKL_MS_KERNMOUNT	(1<<22) /* this is a kern_mount call */
#define LKL_MS_I_VERSION	(1<<23) /* Update inode I_version field */
#define LKL_MS_STRICTATIME	(1<<24) /* Always perform atime updates */
#define LKL_MS_LAZYTIME	(1<<25) /* Update the on-disk [acm]times lazily */

/* These sb flags are internal to the kernel */
#define LKL_MS_SUBMOUNT     (1<<26)
#define LKL_MS_NOREMOTELOCK	(1<<27)
#define LKL_MS_NOSEC	(1<<28)
#define LKL_MS_BORN		(1<<29)
#define LKL_MS_ACTIVE	(1<<30)
#define LKL_MS_NOUSER	(1<<31)

/*
 * Superblock flags that can be altered by LKL_MS_REMOUNT
 */
#define LKL_MS_RMT_MASK	(LKL_MS_RDONLY|LKL_MS_SYNCHRONOUS|LKL_MS_MANDLOCK|LKL_MS_I_VERSION|\
			 LKL_MS_LAZYTIME)

/*
 * Old magic mount flag and mask
 */
#define LKL_MS_MGC_VAL 0xC0ED0000
#define LKL_MS_MGC_MSK 0xffff0000

/*
 * open_tree() flags.
 */
#define LKL_OPEN_TREE_CLONE		1		/* Clone the target tree and attach the clone */
#define LKL_OPEN_TREE_CLOEXEC	LKL_O_CLOEXEC	/* Close the file on execve() */

/*
 * move_mount() flags.
 */
#define LKL_MOVE_MOUNT_F_SYMLINKS		0x00000001 /* Follow symlinks on from path */
#define LKL_MOVE_MOUNT_F_AUTOMOUNTS		0x00000002 /* Follow automounts on from path */
#define LKL_MOVE_MOUNT_F_EMPTY_PATH		0x00000004 /* Empty from path permitted */
#define LKL_MOVE_MOUNT_T_SYMLINKS		0x00000010 /* Follow symlinks on to path */
#define LKL_MOVE_MOUNT_T_AUTOMOUNTS		0x00000020 /* Follow automounts on to path */
#define LKL_MOVE_MOUNT_T_EMPTY_PATH		0x00000040 /* Empty to path permitted */
#define LKL_MOVE_MOUNT_SET_GROUP		0x00000100 /* Set sharing group instead */
#define LKL_MOVE_MOUNT_BENEATH		0x00000200 /* Mount beneath top mount */
#define LKL_MOVE_MOUNT__MASK		0x00000377

/*
 * fsopen() flags.
 */
#define LKL_FSOPEN_CLOEXEC		0x00000001

/*
 * fspick() flags.
 */
#define LKL_FSPICK_CLOEXEC		0x00000001
#define LKL_FSPICK_SYMLINK_NOFOLLOW	0x00000002
#define LKL_FSPICK_NO_AUTOMOUNT	0x00000004
#define LKL_FSPICK_EMPTY_PATH	0x00000008

/*
 * The type of fsconfig() call made.
 */
enum lkl_fsconfig_command {
	LKL_FSCONFIG_SET_FLAG	= 0,	/* Set parameter, supplying no value */
	LKL_FSCONFIG_SET_STRING	= 1,	/* Set parameter, supplying a string value */
	LKL_FSCONFIG_SET_BINARY	= 2,	/* Set parameter, supplying a binary blob value */
	LKL_FSCONFIG_SET_PATH	= 3,	/* Set parameter, supplying an object by path */
	LKL_FSCONFIG_SET_PATH_EMPTY	= 4,	/* Set parameter, supplying an object by (empty) path */
	LKL_FSCONFIG_SET_FD		= 5,	/* Set parameter, supplying an object by fd */
	LKL_FSCONFIG_CMD_CREATE	= 6,	/* Create new or reuse existing superblock */
	LKL_FSCONFIG_CMD_RECONFIGURE = 7,	/* Invoke superblock reconfiguration */
	LKL_FSCONFIG_CMD_CREATE_EXCL = 8,	/* Create new superblock, fail if reusing existing superblock */
};

/*
 * fsmount() flags.
 */
#define LKL_FSMOUNT_CLOEXEC		0x00000001

/*
 * Mount attributes.
 */
#define LKL_MOUNT_ATTR_RDONLY	0x00000001 /* Mount read-only */
#define LKL_MOUNT_ATTR_NOSUID	0x00000002 /* Ignore suid and sgid bits */
#define LKL_MOUNT_ATTR_NODEV	0x00000004 /* Disallow access to device special files */
#define LKL_MOUNT_ATTR_NOEXEC	0x00000008 /* Disallow program execution */
#define LKL_MOUNT_ATTR__ATIME	0x00000070 /* Setting on how atime should be updated */
#define LKL_MOUNT_ATTR_RELATIME	0x00000000 /* - Update atime relative to mtime/ctime. */
#define LKL_MOUNT_ATTR_NOATIME	0x00000010 /* - Do not update access times. */
#define LKL_MOUNT_ATTR_STRICTATIME	0x00000020 /* - Always perform atime updates */
#define LKL_MOUNT_ATTR_NODIRATIME	0x00000080 /* Do not update directory access times */
#define LKL_MOUNT_ATTR_IDMAP	0x00100000 /* Idmap mount to @userns_fd in struct lkl_mount_attr. */
#define LKL_MOUNT_ATTR_NOSYMFOLLOW	0x00200000 /* Do not follow symlinks */

/*
 * mount_setattr()
 */
struct lkl_mount_attr {
	__lkl__u64 attr_set;
	__lkl__u64 attr_clr;
	__lkl__u64 propagation;
	__lkl__u64 userns_fd;
};

/* List of all mount_attr versions. */
#define LKL_MOUNT_ATTR_SIZE_VER0	32 /* sizeof first published struct */


/*
 * Structure for getting mount/superblock/filesystem info with statmount(2).
 *
 * The interface is similar to statx(2): individual fields or groups can be
 * selected with the @mask argument of statmount().  Kernel will set the @mask
 * field according to the supported fields.
 *
 * If string fields are selected, then the caller needs to pass a buffer that
 * has space after the fixed part of the structure.  Nul terminated strings are
 * copied there and offsets relative to @str are stored in the relevant fields.
 * If the buffer is too small, then LKL_EOVERFLOW is returned.  The actually used
 * size is returned in @size.
 */
struct lkl_statmount {
	__lkl__u32 size;		/* Total size, including strings */
	__lkl__u32 mnt_opts;		/* [str] Mount options of the mount */
	__lkl__u64 mask;		/* What results were written */
	__lkl__u32 sb_dev_major;	/* Device ID */
	__lkl__u32 sb_dev_minor;
	__lkl__u64 sb_magic;		/* ..._SUPER_MAGIC */
	__lkl__u32 sb_flags;		/* SB_{RDONLY,SYNCHRONOUS,DIRSYNC,LAZYTIME} */
	__lkl__u32 fs_type;		/* [str] Filesystem type */
	__lkl__u64 mnt_id;		/* Unique ID of mount */
	__lkl__u64 mnt_parent_id;	/* Unique ID of parent (for root == mnt_id) */
	__lkl__u32 mnt_id_old;	/* Reused IDs used in proc/.../mountinfo */
	__lkl__u32 mnt_parent_id_old;
	__lkl__u64 mnt_attr;		/* MOUNT_ATTR_... */
	__lkl__u64 mnt_propagation;	/* MS_{SHARED,SLAVE,PRIVATE,UNBINDABLE} */
	__lkl__u64 mnt_peer_group;	/* ID of shared peer group */
	__lkl__u64 mnt_master;	/* Mount receives propagation from this ID */
	__lkl__u64 propagate_from;	/* Propagation from in current namespace */
	__lkl__u32 mnt_root;		/* [str] Root of mount relative to root of fs */
	__lkl__u32 mnt_point;	/* [str] Mountpoint relative to current root */
	__lkl__u64 mnt_ns_id;	/* ID of the mount namespace */
	__lkl__u64 __spare2[49];
	char str[];		/* Variable size part containing strings */
};

/*
 * Structure for passing mount ID and miscellaneous parameters to statmount(2)
 * and listmount(2).
 *
 * For statmount(2) @param represents the request mask.
 * For listmount(2) @param represents the last listed mount id (or zero).
 */
struct lkl_mnt_id_req {
	__lkl__u32 size;
	__lkl__u32 spare;
	__lkl__u64 mnt_id;
	__lkl__u64 param;
	__lkl__u64 mnt_ns_id;
};

/* List of all mnt_id_req versions. */
#define LKL_MNT_ID_REQ_SIZE_VER0	24 /* sizeof first published struct */
#define LKL_MNT_ID_REQ_SIZE_VER1	32 /* sizeof second published struct */

/*
 * @mask bits for statmount(2)
 */
#define LKL_STATMOUNT_SB_BASIC		0x00000001U     /* Want/got sb_... */
#define LKL_STATMOUNT_MNT_BASIC		0x00000002U	/* Want/got mnt_... */
#define LKL_STATMOUNT_PROPAGATE_FROM	0x00000004U	/* Want/got propagate_from */
#define LKL_STATMOUNT_MNT_ROOT		0x00000008U	/* Want/got mnt_root  */
#define LKL_STATMOUNT_MNT_POINT		0x00000010U	/* Want/got mnt_point */
#define LKL_STATMOUNT_FS_TYPE		0x00000020U	/* Want/got fs_type */
#define LKL_STATMOUNT_MNT_NS_ID		0x00000040U	/* Want/got mnt_ns_id */
#define LKL_STATMOUNT_MNT_OPTS		0x00000080U	/* Want/got mnt_opts */

/*
 * Special @mnt_id values that can be passed to listmount
 */
#define LKL_LSMT_ROOT		0xffffffffffffffff	/* root mount */
#define LKL_LISTMOUNT_REVERSE	(1 << 0) /* List later mounts first */

#endif /* _LKL_LINUX_MOUNT_H */
