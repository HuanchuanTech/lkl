/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _LKL_LINUX_FS_H
#define _LKL_LINUX_FS_H

/*
 * This file has definitions for some important file table structures
 * and constants and structures used by various generic file system
 * ioctl's.  Please do not make any changes in this file before
 * sending patches for review to linux-fsdevel@vger.kernel.org and
 * linux-api@vger.kernel.org.
 */

#include <lkl/linux/limits.h>
#include <lkl/linux/ioctl.h>
#include <lkl/linux/types.h>
#include <lkl/linux/fscrypt.h>

/* Use of MS_* flags within the kernel is restricted to core mount(2) code. */
#include <lkl/linux/mount.h>

/*
 * It's silly to have LKL_NR_OPEN bigger than LKL_NR_FILE, but you can change
 * the file limit at runtime and only root can increase the per-process
 * nr_file rlimit, so it's safe to set up a ridiculously high absolute
 * upper limit on files-per-process.
 *
 * Some programs (notably those using select()) may have to be 
 * recompiled to take full advantage of the new limits..  
 */

/* Fixed constants first: */
#undef LKL_NR_OPEN
#define LKL_INR_OPEN_CUR 1024	/* Initial setting for nfile rlimits */
#define LKL_INR_OPEN_MAX 4096	/* Hard limit for nfile rlimits */

#define LKL_BLOCK_SIZE_BITS 10
#define LKL_BLOCK_SIZE (1<<LKL_BLOCK_SIZE_BITS)

#define LKL_SEEK_SET	0	/* seek relative to beginning of file */
#define LKL_SEEK_CUR	1	/* seek relative to current file position */
#define LKL_SEEK_END	2	/* seek relative to end of file */
#define LKL_SEEK_DATA	3	/* seek to the next data */
#define LKL_SEEK_HOLE	4	/* seek to the next hole */
#define LKL_SEEK_MAX	LKL_SEEK_HOLE

#define LKL_RENAME_NOREPLACE	(1 << 0)	/* Don't overwrite target */
#define LKL_RENAME_EXCHANGE		(1 << 1)	/* Exchange source and dest */
#define LKL_RENAME_WHITEOUT		(1 << 2)	/* Whiteout source */

struct lkl_file_clone_range {
	__lkl__s64 src_fd;
	__lkl__u64 src_offset;
	__lkl__u64 src_length;
	__lkl__u64 dest_offset;
};

struct lkl_fstrim_range {
	__lkl__u64 start;
	__lkl__u64 len;
	__lkl__u64 minlen;
};

/*
 * We include a length field because some filesystems (vfat) have an identifier
 * that we do want to expose as a UUID, but doesn't have the standard length.
 *
 * We use a fixed size buffer beacuse this interface will, by fiat, never
 * support "UUIDs" longer than 16 bytes; we don't want to force all downstream
 * users to have to deal with that.
 */
struct lkl_fsuuid2 {
	__lkl__u8	len;
	__lkl__u8	uuid[16];
};

struct lkl_fs_sysfs_path {
	__lkl__u8			len;
	__lkl__u8			name[128];
};

/* extent-same (dedupe) ioctls; these MUST match the btrfs ioctl definitions */
#define LKL_FILE_DEDUPE_RANGE_SAME		0
#define LKL_FILE_DEDUPE_RANGE_DIFFERS	1

/* from struct btrfs_ioctl_file_extent_same_info */
struct lkl_file_dedupe_range_info {
	__lkl__s64 dest_fd;		/* in - destination file */
	__lkl__u64 dest_offset;	/* in - start of extent in destination */
	__lkl__u64 bytes_deduped;	/* out - total # of bytes we were able
				 * to dedupe from this file. */
	/* status of this dedupe operation:
	 * < 0 for error
	 * == LKL_FILE_DEDUPE_RANGE_SAME if dedupe succeeds
	 * == LKL_FILE_DEDUPE_RANGE_DIFFERS if data differs
	 */
	__lkl__s32 status;		/* out - see above description */
	__lkl__u32 reserved;		/* must be zero */
};

/* from struct btrfs_ioctl_file_extent_same_args */
struct lkl_file_dedupe_range {
	__lkl__u64 src_offset;	/* in - start of extent in source */
	__lkl__u64 src_length;	/* in - length of extent */
	__lkl__u16 dest_count;	/* in - total elements in info array */
	__lkl__u16 reserved1;	/* must be zero */
	__lkl__u32 reserved2;	/* must be zero */
	struct lkl_file_dedupe_range_info info[];
};

/* And dynamically-tunable limits and defaults: */
struct lkl_files_stat_struct {
	unsigned long nr_files;		/* read only */
	unsigned long nr_free_files;	/* read only */
	unsigned long max_files;		/* tunable */
};

struct lkl_inodes_stat_t {
	long nr_inodes;
	long nr_unused;
	long dummy[5];		/* padding for sysctl ABI compatibility */
};


#define LKL_NR_FILE  8192	/* this can well be larger on a larger system */

/*
 * Structure for LKL_FS_IOC_FSGETXATTR[A] and LKL_FS_IOC_FSSETXATTR.
 */
struct lkl_fsxattr {
	__lkl__u32		fsx_xflags;	/* xflags field value (get/set) */
	__lkl__u32		fsx_extsize;	/* extsize field value (get/set)*/
	__lkl__u32		fsx_nextents;	/* nextents field value (get)	*/
	__lkl__u32		fsx_projid;	/* project identifier (get/set) */
	__lkl__u32		fsx_cowextsize;	/* CoW extsize field value (get/set)*/
	unsigned char	fsx_pad[8];
};

/*
 * Flags for the fsx_xflags field
 */
#define LKL_FS_XFLAG_REALTIME	0x00000001	/* data in realtime volume */
#define LKL_FS_XFLAG_PREALLOC	0x00000002	/* preallocated file extents */
#define LKL_FS_XFLAG_IMMUTABLE	0x00000008	/* file cannot be modified */
#define LKL_FS_XFLAG_APPEND		0x00000010	/* all writes append */
#define LKL_FS_XFLAG_SYNC		0x00000020	/* all writes synchronous */
#define LKL_FS_XFLAG_NOATIME	0x00000040	/* do not update access time */
#define LKL_FS_XFLAG_NODUMP		0x00000080	/* do not include in backups */
#define LKL_FS_XFLAG_RTINHERIT	0x00000100	/* create with rt bit set */
#define LKL_FS_XFLAG_PROJINHERIT	0x00000200	/* create with parents projid */
#define LKL_FS_XFLAG_NOSYMLINKS	0x00000400	/* disallow symlink creation */
#define LKL_FS_XFLAG_EXTSIZE	0x00000800	/* extent size allocator hint */
#define LKL_FS_XFLAG_EXTSZINHERIT	0x00001000	/* inherit inode extent size */
#define LKL_FS_XFLAG_NODEFRAG	0x00002000	/* do not defragment */
#define LKL_FS_XFLAG_FILESTREAM	0x00004000	/* use filestream allocator */
#define LKL_FS_XFLAG_DAX		0x00008000	/* use DAX for IO */
#define LKL_FS_XFLAG_COWEXTSIZE	0x00010000	/* CoW extent size allocator hint */
#define LKL_FS_XFLAG_HASATTR	0x80000000	/* no DIFLAG for this	*/

/* the read-only stuff doesn't really belong here, but any other place is
   probably as bad and I don't want to create yet another include file. */

#define LKL_BLKROSET   _LKL_IO(0x12,93)	/* set device read-only (0 = read-write) */
#define LKL_BLKROGET   _LKL_IO(0x12,94)	/* get read-only status (0 = read_write) */
#define LKL_BLKRRPART  _LKL_IO(0x12,95)	/* re-read partition table */
#define LKL_BLKGETSIZE _LKL_IO(0x12,96)	/* return device size /512 (long *arg) */
#define LKL_BLKFLSBUF  _LKL_IO(0x12,97)	/* flush buffer cache */
#define LKL_BLKRASET   _LKL_IO(0x12,98)	/* set read ahead for block device */
#define LKL_BLKRAGET   _LKL_IO(0x12,99)	/* get current read ahead setting */
#define LKL_BLKFRASET  _LKL_IO(0x12,100)/* set filesystem (mm/filemap.c) read-ahead */
#define LKL_BLKFRAGET  _LKL_IO(0x12,101)/* get filesystem (mm/filemap.c) read-ahead */
#define LKL_BLKSECTSET _LKL_IO(0x12,102)/* set max sectors per request (ll_rw_blk.c) */
#define LKL_BLKSECTGET _LKL_IO(0x12,103)/* get max sectors per request (ll_rw_blk.c) */
#define LKL_BLKSSZGET  _LKL_IO(0x12,104)/* get block device sector size */
#if 0
#define LKL_BLKPG      _LKL_IO(0x12,105)/* See blkpg.h */

/* Some people are morons.  Do not use sizeof! */

#define LKL_BLKELVGET  _LKL_IOR(0x12,106,lkl_size_t)/* elevator get */
#define LKL_BLKELVSET  _LKL_IOW(0x12,107,lkl_size_t)/* elevator set */
/* This was here just to show that the number is taken -
   probably all these _LKL_IO(0x12,*) ioctls should be moved to blkpg.h. */
#endif
/* A jump here: 108-111 have been used for various private purposes. */
#define LKL_BLKBSZGET  _LKL_IOR(0x12,112,lkl_size_t)
#define LKL_BLKBSZSET  _LKL_IOW(0x12,113,lkl_size_t)
#define LKL_BLKGETSIZE64 _LKL_IOR(0x12,114,lkl_size_t)	/* return device size in bytes (lkl_u64 *arg) */
#define LKL_BLKTRACESETUP _LKL_IOWR(0x12,115,struct blk_user_trace_setup)
#define LKL_BLKTRACESTART _LKL_IO(0x12,116)
#define LKL_BLKTRACESTOP _LKL_IO(0x12,117)
#define LKL_BLKTRACETEARDOWN _LKL_IO(0x12,118)
#define LKL_BLKDISCARD _LKL_IO(0x12,119)
#define LKL_BLKIOMIN _LKL_IO(0x12,120)
#define LKL_BLKIOOPT _LKL_IO(0x12,121)
#define LKL_BLKALIGNOFF _LKL_IO(0x12,122)
#define LKL_BLKPBSZGET _LKL_IO(0x12,123)
#define LKL_BLKDISCARDZEROES _LKL_IO(0x12,124)
#define LKL_BLKSECDISCARD _LKL_IO(0x12,125)
#define LKL_BLKROTATIONAL _LKL_IO(0x12,126)
#define LKL_BLKZEROOUT _LKL_IO(0x12,127)
#define LKL_BLKGETDISKSEQ _LKL_IOR(0x12,128,__lkl__u64)
/*
 * A jump here: 130-136 are reserved for zoned block devices
 * (see uapi/linux/blkzoned.h)
 */

#define LKL_BMAP_IOCTL 1		/* obsolete - kept for compatibility */
#define LKL_FIBMAP	   _LKL_IO(0x00,1)	/* bmap access */
#define LKL_FIGETBSZ   _LKL_IO(0x00,2)	/* get the block size used for bmap */
#define LKL_FIFREEZE	_LKL_IOWR('X', 119, int)	/* Freeze */
#define LKL_FITHAW		_LKL_IOWR('X', 120, int)	/* Thaw */
#define LKL_FITRIM		_LKL_IOWR('X', 121, struct lkl_fstrim_range)	/* Trim */
#define LKL_FICLONE		_LKL_IOW(0x94, 9, int)
#define LKL_FICLONERANGE	_LKL_IOW(0x94, 13, struct lkl_file_clone_range)
#define LKL_FIDEDUPERANGE	_LKL_IOWR(0x94, 54, struct lkl_file_dedupe_range)

#define LKL_FSLABEL_MAX 256	/* Max chars for the interface; each fs may differ */

#define	LKL_FS_IOC_GETFLAGS			_LKL_IOR('f', 1, long)
#define	LKL_FS_IOC_SETFLAGS			_LKL_IOW('f', 2, long)
#define	LKL_FS_IOC_GETVERSION		_LKL_IOR('v', 1, long)
#define	LKL_FS_IOC_SETVERSION		_LKL_IOW('v', 2, long)
#define LKL_FS_IOC_FIEMAP			_LKL_IOWR('f', 11, struct fiemap)
#define LKL_FS_IOC32_GETFLAGS		_LKL_IOR('f', 1, int)
#define LKL_FS_IOC32_SETFLAGS		_LKL_IOW('f', 2, int)
#define LKL_FS_IOC32_GETVERSION		_LKL_IOR('v', 1, int)
#define LKL_FS_IOC32_SETVERSION		_LKL_IOW('v', 2, int)
#define LKL_FS_IOC_FSGETXATTR		_LKL_IOR('X', 31, struct lkl_fsxattr)
#define LKL_FS_IOC_FSSETXATTR		_LKL_IOW('X', 32, struct lkl_fsxattr)
#define LKL_FS_IOC_GETFSLABEL		_LKL_IOR(0x94, 49, char[LKL_FSLABEL_MAX])
#define LKL_FS_IOC_SETFSLABEL		_LKL_IOW(0x94, 50, char[LKL_FSLABEL_MAX])
/* Returns the external filesystem UUID, the same one blkid returns */
#define LKL_FS_IOC_GETFSUUID		_LKL_IOR(0x15, 0, struct lkl_fsuuid2)
/*
 * Returns the path component under /sys/fs/ that refers to this filesystem;
 * also /sys/kernel/debug/ for filesystems with debugfs exports
 */
#define LKL_FS_IOC_GETFSSYSFSPATH		_LKL_IOR(0x15, 1, struct lkl_fs_sysfs_path)

/*
 * Inode flags (LKL_FS_IOC_GETFLAGS / LKL_FS_IOC_SETFLAGS)
 *
 * Note: for historical reasons, these flags were originally used and
 * defined for use by ext2/ext3, and then other file systems started
 * using these flags so they wouldn't need to write their own version
 * of chattr/lsattr (which was shipped as part of e2fsprogs).  You
 * should think twice before trying to use these flags in new
 * contexts, or trying to assign these flags, since they are used both
 * as the UAPI and the on-disk encoding for ext2/3/4.  Also, we are
 * almost out of 32-bit flags.  :-)
 *
 * We have recently hoisted LKL_FS_IOC_FSGETXATTR / LKL_FS_IOC_FSSETXATTR from
 * XFS to the generic FS level interface.  This uses a structure that
 * has padding and hence has more room to grow, so it may be more
 * appropriate for many new use cases.
 *
 * Please do not change these flags or interfaces before checking with
 * linux-fsdevel@vger.kernel.org and linux-api@vger.kernel.org.
 */
#define	LKL_FS_SECRM_FL			0x00000001 /* Secure deletion */
#define	LKL_FS_UNRM_FL			0x00000002 /* Undelete */
#define	LKL_FS_COMPR_FL			0x00000004 /* Compress file */
#define LKL_FS_SYNC_FL			0x00000008 /* Synchronous updates */
#define LKL_FS_IMMUTABLE_FL			0x00000010 /* Immutable file */
#define LKL_FS_APPEND_FL			0x00000020 /* writes to file may only append */
#define LKL_FS_NODUMP_FL			0x00000040 /* do not dump file */
#define LKL_FS_NOATIME_FL			0x00000080 /* do not update atime */
/* Reserved for compression usage... */
#define LKL_FS_DIRTY_FL			0x00000100
#define LKL_FS_COMPRBLK_FL			0x00000200 /* One or more compressed clusters */
#define LKL_FS_NOCOMP_FL			0x00000400 /* Don't compress */
/* End compression flags --- maybe not all used */
#define LKL_FS_ENCRYPT_FL			0x00000800 /* Encrypted file */
#define LKL_FS_BTREE_FL			0x00001000 /* btree format dir */
#define LKL_FS_INDEX_FL			0x00001000 /* hash-indexed directory */
#define LKL_FS_IMAGIC_FL			0x00002000 /* AFS directory */
#define LKL_FS_JOURNAL_DATA_FL		0x00004000 /* Reserved for ext3 */
#define LKL_FS_NOTAIL_FL			0x00008000 /* file tail should not be merged */
#define LKL_FS_DIRSYNC_FL			0x00010000 /* dirsync behaviour (directories only) */
#define LKL_FS_TOPDIR_FL			0x00020000 /* Top of directory hierarchies*/
#define LKL_FS_HUGE_FILE_FL			0x00040000 /* Reserved for ext4 */
#define LKL_FS_EXTENT_FL			0x00080000 /* Extents */
#define LKL_FS_VERITY_FL			0x00100000 /* Verity protected inode */
#define LKL_FS_EA_INODE_FL			0x00200000 /* Inode used for large EA */
#define LKL_FS_EOFBLOCKS_FL			0x00400000 /* Reserved for ext4 */
#define LKL_FS_NOCOW_FL			0x00800000 /* Do not cow file */
#define LKL_FS_DAX_FL			0x02000000 /* Inode is DAX */
#define LKL_FS_INLINE_DATA_FL		0x10000000 /* Reserved for ext4 */
#define LKL_FS_PROJINHERIT_FL		0x20000000 /* Create with parents projid */
#define LKL_FS_CASEFOLD_FL			0x40000000 /* Folder is case insensitive */
#define LKL_FS_RESERVED_FL			0x80000000 /* reserved for ext2 lib */

#define LKL_FS_FL_USER_VISIBLE		0x0003DFFF /* User visible flags */
#define LKL_FS_FL_USER_MODIFIABLE		0x000380FF /* User modifiable flags */


#define LKL_SYNC_FILE_RANGE_WAIT_BEFORE	1
#define LKL_SYNC_FILE_RANGE_WRITE		2
#define LKL_SYNC_FILE_RANGE_WAIT_AFTER	4
#define LKL_SYNC_FILE_RANGE_WRITE_AND_WAIT	(LKL_SYNC_FILE_RANGE_WRITE | \
					 LKL_SYNC_FILE_RANGE_WAIT_BEFORE | \
					 LKL_SYNC_FILE_RANGE_WAIT_AFTER)

/*
 * Flags for preadv2/pwritev2:
 */

typedef int __lkl__bitwise __lkl__kernel_rwf_t;

/* high priority request, poll if possible */
#define LKL_RWF_HIPRI	((__lkl__kernel_rwf_t)0x00000001)

/* per-IO LKL_O_DSYNC */
#define LKL_RWF_DSYNC	((__lkl__kernel_rwf_t)0x00000002)

/* per-IO LKL_O_SYNC */
#define LKL_RWF_SYNC	((__lkl__kernel_rwf_t)0x00000004)

/* per-IO, return -LKL_EAGAIN if operation would block */
#define LKL_RWF_NOWAIT	((__lkl__kernel_rwf_t)0x00000008)

/* per-IO LKL_O_APPEND */
#define LKL_RWF_APPEND	((__lkl__kernel_rwf_t)0x00000010)

/* per-IO negation of LKL_O_APPEND */
#define LKL_RWF_NOAPPEND	((__lkl__kernel_rwf_t)0x00000020)

/* Atomic Write */
#define LKL_RWF_ATOMIC	((__lkl__kernel_rwf_t)0x00000040)

/* mask of flags supported by the kernel */
#define LKL_RWF_SUPPORTED	(LKL_RWF_HIPRI | LKL_RWF_DSYNC | LKL_RWF_SYNC | LKL_RWF_NOWAIT |\
			 LKL_RWF_APPEND | LKL_RWF_NOAPPEND | LKL_RWF_ATOMIC)

#define LKL_PROCFS_IOCTL_MAGIC 'f'

/* Pagemap ioctl */
#define LKL_PAGEMAP_SCAN	_LKL_IOWR(LKL_PROCFS_IOCTL_MAGIC, 16, struct lkl_pm_scan_arg)

/* Bitmasks provided in pm_scan_args masks and reported in page_region.categories. */
#define LKL_PAGE_IS_WPALLOWED	(1 << 0)
#define LKL_PAGE_IS_WRITTEN		(1 << 1)
#define LKL_PAGE_IS_FILE		(1 << 2)
#define LKL_PAGE_IS_PRESENT		(1 << 3)
#define LKL_PAGE_IS_SWAPPED		(1 << 4)
#define LKL_PAGE_IS_PFNZERO		(1 << 5)
#define LKL_PAGE_IS_HUGE		(1 << 6)
#define LKL_PAGE_IS_SOFT_DIRTY	(1 << 7)

/*
 * struct lkl_page_region - Page region with flags
 * @start:	Start of the region
 * @end:	End of the region (exclusive)
 * @categories:	PAGE_IS_* category bitmask for the region
 */
struct lkl_page_region {
	__lkl__u64 start;
	__lkl__u64 end;
	__lkl__u64 categories;
};

/* Flags for LKL_PAGEMAP_SCAN ioctl */
#define LKL_PM_SCAN_WP_MATCHING	(1 << 0)	/* Write protect the pages matched. */
#define LKL_PM_SCAN_CHECK_WPASYNC	(1 << 1)	/* Abort the scan when a non-WP-enabled page is found. */

/*
 * struct lkl_pm_scan_arg - Pagemap ioctl argument
 * @size:		Size of the structure
 * @flags:		Flags for the IOCTL
 * @start:		Starting address of the region
 * @end:		Ending address of the region
 * @walk_end		Address where the scan stopped (written by kernel).
 *			walk_end == end (address tags cleared) informs that the scan completed on entire range.
 * @vec:		Address of page_region struct array for output
 * @vec_len:		Length of the page_region struct array
 * @max_pages:		Optional limit for number of returned pages (0 = disabled)
 * @category_inverted:	PAGE_IS_* categories which values match if 0 instead of 1
 * @category_mask:	Skip pages for which any category doesn't match
 * @category_anyof_mask: Skip pages for which no category matches
 * @return_mask:	PAGE_IS_* categories that are to be reported in `page_region`s returned
 */
struct lkl_pm_scan_arg {
	__lkl__u64 size;
	__lkl__u64 flags;
	__lkl__u64 start;
	__lkl__u64 end;
	__lkl__u64 walk_end;
	__lkl__u64 vec;
	__lkl__u64 vec_len;
	__lkl__u64 max_pages;
	__lkl__u64 category_inverted;
	__lkl__u64 category_mask;
	__lkl__u64 category_anyof_mask;
	__lkl__u64 return_mask;
};

/* /proc/<pid>/maps ioctl */
#define LKL_PROCMAP_QUERY	_LKL_IOWR(LKL_PROCFS_IOCTL_MAGIC, 17, struct lkl_procmap_query)

enum lkl_procmap_query_flags {
	/*
	 * VMA permission flags.
	 *
	 * Can be used as part of procmap_query.query_flags field to look up
	 * only VMAs satisfying specified subset of permissions. E.g., specifying
	 * LKL_PROCMAP_QUERY_VMA_READABLE only will return both readable and read/write VMAs,
	 * while having LKL_PROCMAP_QUERY_VMA_READABLE | LKL_PROCMAP_QUERY_VMA_WRITABLE will only
	 * return read/write VMAs, though both executable/non-executable and
	 * private/shared will be ignored.
	 *
	 * PROCMAP_QUERY_VMA_* flags are also returned in procmap_query.vma_flags
	 * field to specify actual VMA permissions.
	 */
	LKL_PROCMAP_QUERY_VMA_READABLE		= 0x01,
	LKL_PROCMAP_QUERY_VMA_WRITABLE		= 0x02,
	LKL_PROCMAP_QUERY_VMA_EXECUTABLE		= 0x04,
	LKL_PROCMAP_QUERY_VMA_SHARED		= 0x08,
	/*
	 * Query modifier flags.
	 *
	 * By default VMA that covers provided address is returned, or -LKL_ENOENT
	 * is returned. With LKL_PROCMAP_QUERY_COVERING_OR_NEXT_VMA flag set, closest
	 * VMA with vma_start > addr will be returned if no covering VMA is
	 * found.
	 *
	 * LKL_PROCMAP_QUERY_FILE_BACKED_VMA instructs query to consider only VMAs that
	 * have file backing. Can be combined with LKL_PROCMAP_QUERY_COVERING_OR_NEXT_VMA
	 * to iterate all VMAs with file backing.
	 */
	LKL_PROCMAP_QUERY_COVERING_OR_NEXT_VMA	= 0x10,
	LKL_PROCMAP_QUERY_FILE_BACKED_VMA		= 0x20,
};

/*
 * Input/output argument structured passed into ioctl() call. It can be used
 * to query a set of VMAs (Virtual Memory Areas) of a process.
 *
 * Each field can be one of three kinds, marked in a short comment to the
 * right of the field:
 *   - "in", input argument, user has to provide this value, kernel doesn't modify it;
 *   - "out", output argument, kernel sets this field with VMA data;
 *   - "in/out", input and output argument; user provides initial value (used
 *     to specify maximum allowable buffer size), and kernel sets it to actual
 *     amount of data written (or zero, if there is no data).
 *
 * If matching VMA is found (according to criterias specified by
 * query_addr/query_flags, all the out fields are filled out, and ioctl()
 * returns 0. If there is no matching VMA, -LKL_ENOENT will be returned.
 * In case of any other error, negative error code other than -LKL_ENOENT is
 * returned.
 *
 * Most of the data is similar to the one returned as text in /proc/<pid>/maps
 * file, but procmap_query provides more querying flexibility. There are no
 * consistency guarantees between subsequent ioctl() calls, but data returned
 * for matched VMA is self-consistent.
 */
struct lkl_procmap_query {
	/* Query struct size, for backwards/forward compatibility */
	__lkl__u64 size;
	/*
	 * Query flags, a combination of enum lkl_procmap_query_flags values.
	 * Defines query filtering and behavior, see enum lkl_procmap_query_flags.
	 *
	 * Input argument, provided by user. Kernel doesn't modify it.
	 */
	__lkl__u64 query_flags;		/* in */
	/*
	 * Query address. By default, VMA that covers this address will
	 * be looked up. PROCMAP_QUERY_* flags above modify this default
	 * behavior further.
	 *
	 * Input argument, provided by user. Kernel doesn't modify it.
	 */
	__lkl__u64 query_addr;		/* in */
	/* VMA starting (inclusive) and ending (exclusive) address, if VMA is found. */
	__lkl__u64 vma_start;		/* out */
	__lkl__u64 vma_end;			/* out */
	/* VMA permissions flags. A combination of PROCMAP_QUERY_VMA_* flags. */
	__lkl__u64 vma_flags;		/* out */
	/* VMA backing page size granularity. */
	__lkl__u64 vma_page_size;		/* out */
	/*
	 * VMA file offset. If VMA has file backing, this specifies offset
	 * within the file that VMA's start address corresponds to.
	 * Is set to zero if VMA has no backing file.
	 */
	__lkl__u64 vma_offset;		/* out */
	/* Backing file's inode number, or zero, if VMA has no backing file. */
	__lkl__u64 inode;			/* out */
	/* Backing file's device major/minor number, or zero, if VMA has no backing file. */
	__lkl__u32 dev_major;		/* out */
	__lkl__u32 dev_minor;		/* out */
	/*
	 * If set to non-zero value, signals the request to return VMA name
	 * (i.e., VMA's backing file's absolute path, with " (deleted)" suffix
	 * appended, if file was unlinked from FS) for matched VMA. VMA name
	 * can also be some special name (e.g., "[heap]", "[stack]") or could
	 * be even user-supplied with prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME).
	 *
	 * Kernel will set this field to zero, if VMA has no associated name.
	 * Otherwise kernel will return actual amount of bytes filled in
	 * user-supplied buffer (see vma_name_addr field below), including the
	 * terminating zero.
	 *
	 * If VMA name is longer that user-supplied maximum buffer size,
	 * -LKL_E2BIG error is returned.
	 *
	 * If this field is set to non-zero value, vma_name_addr should point
	 * to valid user space memory buffer of at least vma_name_size bytes.
	 * If set to zero, vma_name_addr should be set to zero as well
	 */
	__lkl__u32 vma_name_size;		/* in/out */
	/*
	 * If set to non-zero value, signals the request to extract and return
	 * VMA's backing file's build ID, if the backing file is an ELF file
	 * and it contains embedded build ID.
	 *
	 * Kernel will set this field to zero, if VMA has no backing file,
	 * backing file is not an ELF file, or ELF file has no build ID
	 * embedded.
	 *
	 * Build ID is a binary value (not a string). Kernel will set
	 * build_id_size field to exact number of bytes used for build ID.
	 * If build ID is requested and present, but needs more bytes than
	 * user-supplied maximum buffer size (see build_id_addr field below),
	 * -LKL_E2BIG error will be returned.
	 *
	 * If this field is set to non-zero value, build_id_addr should point
	 * to valid user space memory buffer of at least build_id_size bytes.
	 * If set to zero, build_id_addr should be set to zero as well
	 */
	__lkl__u32 build_id_size;		/* in/out */
	/*
	 * User-supplied address of a buffer of at least vma_name_size bytes
	 * for kernel to fill with matched VMA's name (see vma_name_size field
	 * description above for details).
	 *
	 * Should be set to zero if VMA name should not be returned.
	 */
	__lkl__u64 vma_name_addr;		/* in */
	/*
	 * User-supplied address of a buffer of at least build_id_size bytes
	 * for kernel to fill with matched VMA's ELF build ID, if available
	 * (see build_id_size field description above for details).
	 *
	 * Should be set to zero if build ID should not be returned.
	 */
	__lkl__u64 build_id_addr;		/* in */
};

#endif /* _LKL_LINUX_FS_H */
