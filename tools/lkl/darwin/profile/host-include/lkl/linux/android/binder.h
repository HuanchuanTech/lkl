/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (C) 2008 Google, Inc.
 *
 * Based on, but no longer compatible with, the original
 * OpenBinder.org binder driver interface, which is:
 *
 * Copyright (c) 2005 Palmsource, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LKL_LINUX_BINDER_H
#define _LKL_LINUX_BINDER_H

#include <lkl/linux/types.h>
#include <lkl/linux/ioctl.h>

#define LKL_B_PACK_CHARS(c1, c2, c3, c4) \
	((((c1)<<24)) | (((c2)<<16)) | (((c3)<<8)) | (c4))
#define LKL_B_TYPE_LARGE 0x85

enum {
	LKL_BINDER_TYPE_BINDER	= LKL_B_PACK_CHARS('s', 'b', '*', LKL_B_TYPE_LARGE),
	LKL_BINDER_TYPE_WEAK_BINDER	= LKL_B_PACK_CHARS('w', 'b', '*', LKL_B_TYPE_LARGE),
	LKL_BINDER_TYPE_HANDLE	= LKL_B_PACK_CHARS('s', 'h', '*', LKL_B_TYPE_LARGE),
	LKL_BINDER_TYPE_WEAK_HANDLE	= LKL_B_PACK_CHARS('w', 'h', '*', LKL_B_TYPE_LARGE),
	LKL_BINDER_TYPE_FD		= LKL_B_PACK_CHARS('f', 'd', '*', LKL_B_TYPE_LARGE),
	LKL_BINDER_TYPE_FDA		= LKL_B_PACK_CHARS('f', 'd', 'a', LKL_B_TYPE_LARGE),
	LKL_BINDER_TYPE_PTR		= LKL_B_PACK_CHARS('p', 't', '*', LKL_B_TYPE_LARGE),
};

enum {
	LKL_FLAT_BINDER_FLAG_PRIORITY_MASK = 0xff,
	LKL_FLAT_BINDER_FLAG_ACCEPTS_FDS = 0x100,

	/**
	 * @LKL_FLAT_BINDER_FLAG_TXN_SECURITY_CTX: request security contexts
	 *
	 * Only when set, causes senders to include their security
	 * context
	 */
	LKL_FLAT_BINDER_FLAG_TXN_SECURITY_CTX = 0x1000,
};

#ifdef BINDER_IPC_32BIT
typedef __lkl__u32 lkl_binder_size_t;
typedef __lkl__u32 lkl_binder_uintptr_t;
#else
typedef __lkl__u64 lkl_binder_size_t;
typedef __lkl__u64 lkl_binder_uintptr_t;
#endif

/**
 * struct lkl_binder_object_header - header shared by all binder metadata objects.
 * @type:	type of the object
 */
struct lkl_binder_object_header {
	__lkl__u32        type;
};

/*
 * This is the flattened representation of a Binder object for transfer
 * between processes.  The 'offsets' supplied as part of a binder transaction
 * contains offsets into the data where these structures occur.  The Binder
 * driver takes care of re-writing the structure type and data as it moves
 * between processes.
 */
struct lkl_flat_binder_object {
	struct lkl_binder_object_header	hdr;
	__lkl__u32				flags;

	/* 8 bytes of data. */
	union {
		lkl_binder_uintptr_t	binder;	/* local object */
		__lkl__u32			handle;	/* remote object */
	};

	/* extra data associated with local object */
	lkl_binder_uintptr_t	cookie;
};

/**
 * struct lkl_binder_fd_object - describes a filedescriptor to be fixed up.
 * @hdr:	common header structure
 * @pad_flags:	padding to remain compatible with old userspace code
 * @pad_binder:	padding to remain compatible with old userspace code
 * @fd:		file descriptor
 * @cookie:	opaque data, used by user-space
 */
struct lkl_binder_fd_object {
	struct lkl_binder_object_header	hdr;
	__lkl__u32				pad_flags;
	union {
		lkl_binder_uintptr_t	pad_binder;
		__lkl__u32			fd;
	};

	lkl_binder_uintptr_t		cookie;
};

/* struct lkl_binder_buffer_object - object describing a userspace buffer
 * @hdr:		common header structure
 * @flags:		one or more BINDER_BUFFER_* flags
 * @buffer:		address of the buffer
 * @length:		length of the buffer
 * @parent:		index in offset array pointing to parent buffer
 * @parent_offset:	offset in @parent pointing to this buffer
 *
 * A binder_buffer object represents an object that the
 * binder kernel driver can copy verbatim to the target
 * address space. A buffer itself may be pointed to from
 * within another buffer, meaning that the pointer inside
 * that other buffer needs to be fixed up as well. This
 * can be done by setting the LKL_BINDER_BUFFER_FLAG_HAS_PARENT
 * flag in @flags, by setting @parent buffer to the index
 * in the offset array pointing to the parent binder_buffer_object,
 * and by setting @parent_offset to the offset in the parent buffer
 * at which the pointer to this buffer is located.
 */
struct lkl_binder_buffer_object {
	struct lkl_binder_object_header	hdr;
	__lkl__u32				flags;
	lkl_binder_uintptr_t		buffer;
	lkl_binder_size_t			length;
	lkl_binder_size_t			parent;
	lkl_binder_size_t			parent_offset;
};

enum {
	LKL_BINDER_BUFFER_FLAG_HAS_PARENT = 0x01,
};

/* struct lkl_binder_fd_array_object - object describing an array of fds in a buffer
 * @hdr:		common header structure
 * @pad:		padding to ensure correct alignment
 * @num_fds:		number of file descriptors in the buffer
 * @parent:		index in offset array to buffer holding the fd array
 * @parent_offset:	start offset of fd array in the buffer
 *
 * A binder_fd_array object represents an array of file
 * descriptors embedded in a binder_buffer_object. It is
 * different from a regular binder_buffer_object because it
 * describes a list of file descriptors to fix up, not an opaque
 * blob of memory, and hence the kernel needs to treat it differently.
 *
 * An example of how this would be used is with Android's
 * native_handle_t object, which is a struct with a list of integers
 * and a list of file descriptors. The native_handle_t struct itself
 * will be represented by a struct binder_buffer_objct, whereas the
 * embedded list of file descriptors is represented by a
 * struct lkl_binder_fd_array_object with that binder_buffer_object as
 * a parent.
 */
struct lkl_binder_fd_array_object {
	struct lkl_binder_object_header	hdr;
	__lkl__u32				pad;
	lkl_binder_size_t			num_fds;
	lkl_binder_size_t			parent;
	lkl_binder_size_t			parent_offset;
};

/*
 * On 64-bit platforms where user code may run in 32-bits the driver must
 * translate the buffer (and local binder) addresses appropriately.
 */

struct lkl_binder_write_read {
	lkl_binder_size_t		write_size;	/* bytes to write */
	lkl_binder_size_t		write_consumed;	/* bytes consumed by driver */
	lkl_binder_uintptr_t	write_buffer;
	lkl_binder_size_t		read_size;	/* bytes to read */
	lkl_binder_size_t		read_consumed;	/* bytes consumed by driver */
	lkl_binder_uintptr_t	read_buffer;
};

/* Use with LKL_BINDER_VERSION, driver fills in fields. */
struct lkl_binder_version {
	/* driver protocol version -- increment with incompatible change */
	__lkl__s32       protocol_version;
};

/* This is the current protocol version. */
#ifdef BINDER_IPC_32BIT
#define LKL_BINDER_CURRENT_PROTOCOL_VERSION 7
#else
#define LKL_BINDER_CURRENT_PROTOCOL_VERSION 8
#endif

/*
 * Use with LKL_BINDER_GET_NODE_DEBUG_INFO, driver reads ptr, writes to all fields.
 * Set ptr to NULL for the first call to get the info for the first node, and
 * then repeat the call passing the previously returned value to get the next
 * nodes.  ptr will be 0 when there are no more nodes.
 */
struct lkl_binder_node_debug_info {
	lkl_binder_uintptr_t ptr;
	lkl_binder_uintptr_t cookie;
	__lkl__u32            has_strong_ref;
	__lkl__u32            has_weak_ref;
};

struct lkl_binder_node_info_for_ref {
	__lkl__u32            handle;
	__lkl__u32            strong_count;
	__lkl__u32            weak_count;
	__lkl__u32            reserved1;
	__lkl__u32            reserved2;
	__lkl__u32            reserved3;
};

struct lkl_binder_freeze_info {
	__lkl__u32            pid;
	__lkl__u32            enable;
	__lkl__u32            timeout_ms;
};

struct lkl_binder_frozen_status_info {
	__lkl__u32            pid;

	/* process received sync transactions since last frozen
	 * bit 0: received sync transaction after being frozen
	 * bit 1: new pending sync transaction during freezing
	 */
	__lkl__u32            sync_recv;

	/* process received async transactions since last frozen */
	__lkl__u32            async_recv;
};

struct lkl_binder_frozen_state_info {
	lkl_binder_uintptr_t cookie;
	__lkl__u32            is_frozen;
	__lkl__u32            reserved;
};

/* struct binder_extened_error - extended error information
 * @id:		identifier for the failed operation
 * @command:	command as defined by lkl_binder_driver_return_protocol
 * @param:	parameter holding a negative errno value
 *
 * Used with LKL_BINDER_GET_EXTENDED_ERROR. This extends the error information
 * returned by the driver upon a failed operation. Userspace can pull this
 * data to properly handle specific error scenarios.
 */
struct lkl_binder_extended_error {
	__lkl__u32	id;
	__lkl__u32	command;
	__lkl__s32	param;
};

enum {
	LKL_BINDER_WRITE_READ		= _LKL_IOWR('b', 1, struct lkl_binder_write_read),
	LKL_BINDER_SET_IDLE_TIMEOUT		= _LKL_IOW('b', 3, __lkl__s64),
	LKL_BINDER_SET_MAX_THREADS		= _LKL_IOW('b', 5, __lkl__u32),
	LKL_BINDER_SET_IDLE_PRIORITY	= _LKL_IOW('b', 6, __lkl__s32),
	LKL_BINDER_SET_CONTEXT_MGR		= _LKL_IOW('b', 7, __lkl__s32),
	LKL_BINDER_THREAD_EXIT		= _LKL_IOW('b', 8, __lkl__s32),
	LKL_BINDER_VERSION			= _LKL_IOWR('b', 9, struct lkl_binder_version),
	LKL_BINDER_GET_NODE_DEBUG_INFO	= _LKL_IOWR('b', 11, struct lkl_binder_node_debug_info),
	LKL_BINDER_GET_NODE_INFO_FOR_REF	= _LKL_IOWR('b', 12, struct lkl_binder_node_info_for_ref),
	LKL_BINDER_SET_CONTEXT_MGR_EXT	= _LKL_IOW('b', 13, struct lkl_flat_binder_object),
	LKL_BINDER_FREEZE			= _LKL_IOW('b', 14, struct lkl_binder_freeze_info),
	LKL_BINDER_GET_FROZEN_INFO		= _LKL_IOWR('b', 15, struct lkl_binder_frozen_status_info),
	LKL_BINDER_ENABLE_ONEWAY_SPAM_DETECTION	= _LKL_IOW('b', 16, __lkl__u32),
	LKL_BINDER_GET_EXTENDED_ERROR	= _LKL_IOWR('b', 17, struct lkl_binder_extended_error),
};

/*
 * NOTE: Two special error codes you should check for when calling
 * in to the driver are:
 *
 * LKL_EINTR -- The operation has been interupted.  This should be
 * handled by retrying the ioctl() until a different error code
 * is returned.
 *
 * LKL_ECONNREFUSED -- The driver is no longer accepting operations
 * from your process.  That is, the process is being destroyed.
 * You should handle this by exiting from your process.  Note
 * that once this error code is returned, all further calls to
 * the driver from any thread will return this same code.
 */

enum lkl_transaction_flags {
	LKL_TF_ONE_WAY	= 0x01,	/* this is a one-way call: async, no return */
	LKL_TF_ROOT_OBJECT	= 0x04,	/* contents are the component's root object */
	LKL_TF_STATUS_CODE	= 0x08,	/* contents are a 32-bit status code */
	LKL_TF_ACCEPT_FDS	= 0x10,	/* allow replies with file descriptors */
	LKL_TF_CLEAR_BUF	= 0x20,	/* clear buffer on txn complete */
	LKL_TF_UPDATE_TXN	= 0x40,	/* update the outdated pending async txn */
};

struct lkl_binder_transaction_data {
	/* The first two are only used for bcTRANSACTION and brTRANSACTION,
	 * identifying the target and contents of the transaction.
	 */
	union {
		/* target descriptor of command transaction */
		__lkl__u32	handle;
		/* target descriptor of return transaction */
		lkl_binder_uintptr_t ptr;
	} target;
	lkl_binder_uintptr_t	cookie;	/* target object cookie */
	__lkl__u32		code;		/* transaction command */

	/* General information about the transaction. */
	__lkl__u32	        flags;
	__lkl__kernel_pid_t	sender_pid;
	__lkl__kernel_uid32_t	sender_euid;
	lkl_binder_size_t	data_size;	/* number of bytes of data */
	lkl_binder_size_t	offsets_size;	/* number of bytes of offsets */

	/* If this transaction is inline, the data immediately
	 * follows here; otherwise, it ends with a pointer to
	 * the data buffer.
	 */
	union {
		struct {
			/* transaction data */
			lkl_binder_uintptr_t	buffer;
			/* offsets from buffer to flat_binder_object structs */
			lkl_binder_uintptr_t	offsets;
		} ptr;
		__lkl__u8	buf[8];
	} data;
};

struct lkl_binder_transaction_data_secctx {
	struct lkl_binder_transaction_data transaction_data;
	lkl_binder_uintptr_t secctx;
};

struct lkl_binder_transaction_data_sg {
	struct lkl_binder_transaction_data transaction_data;
	lkl_binder_size_t buffers_size;
};

struct lkl_binder_ptr_cookie {
	lkl_binder_uintptr_t ptr;
	lkl_binder_uintptr_t cookie;
};

struct lkl_binder_handle_cookie {
	__lkl__u32 handle;
	lkl_binder_uintptr_t cookie;
} __attribute__((packed));

struct lkl_binder_pri_desc {
	__lkl__s32 priority;
	__lkl__u32 desc;
};

struct lkl_binder_pri_ptr_cookie {
	__lkl__s32 priority;
	lkl_binder_uintptr_t ptr;
	lkl_binder_uintptr_t cookie;
};

enum lkl_binder_driver_return_protocol {
	LKL_BR_ERROR = _LKL_IOR('r', 0, __lkl__s32),
	/*
	 * int: error code
	 */

	LKL_BR_OK = _LKL_IO('r', 1),
	/* No parameters! */

	LKL_BR_TRANSACTION_SEC_CTX = _LKL_IOR('r', 2,
				      struct lkl_binder_transaction_data_secctx),
	/*
	 * binder_transaction_data_secctx: the received command.
	 */
	LKL_BR_TRANSACTION = _LKL_IOR('r', 2, struct lkl_binder_transaction_data),
	LKL_BR_REPLY = _LKL_IOR('r', 3, struct lkl_binder_transaction_data),
	/*
	 * binder_transaction_data: the received command.
	 */

	LKL_BR_ACQUIRE_RESULT = _LKL_IOR('r', 4, __lkl__s32),
	/*
	 * not currently supported
	 * int: 0 if the last bcATTEMPT_ACQUIRE was not successful.
	 * Else the remote object has acquired a primary reference.
	 */

	LKL_BR_DEAD_REPLY = _LKL_IO('r', 5),
	/*
	 * The target of the last transaction (either a bcTRANSACTION or
	 * a bcATTEMPT_ACQUIRE) is no longer with us.  No parameters.
	 */

	LKL_BR_TRANSACTION_COMPLETE = _LKL_IO('r', 6),
	/*
	 * No parameters... always refers to the last transaction requested
	 * (including replies).  Note that this will be sent even for
	 * asynchronous transactions.
	 */

	LKL_BR_INCREFS = _LKL_IOR('r', 7, struct lkl_binder_ptr_cookie),
	LKL_BR_ACQUIRE = _LKL_IOR('r', 8, struct lkl_binder_ptr_cookie),
	LKL_BR_RELEASE = _LKL_IOR('r', 9, struct lkl_binder_ptr_cookie),
	LKL_BR_DECREFS = _LKL_IOR('r', 10, struct lkl_binder_ptr_cookie),
	/*
	 * void *:	ptr to binder
	 * void *: cookie for binder
	 */

	LKL_BR_ATTEMPT_ACQUIRE = _LKL_IOR('r', 11, struct lkl_binder_pri_ptr_cookie),
	/*
	 * not currently supported
	 * int:	priority
	 * void *: ptr to binder
	 * void *: cookie for binder
	 */

	LKL_BR_NOOP = _LKL_IO('r', 12),
	/*
	 * No parameters.  Do nothing and examine the next command.  It exists
	 * primarily so that we can replace it with a LKL_BR_SPAWN_LOOPER command.
	 */

	LKL_BR_SPAWN_LOOPER = _LKL_IO('r', 13),
	/*
	 * No parameters.  The driver has determined that a process has no
	 * threads waiting to service incoming transactions.  When a process
	 * receives this command, it must spawn a new service thread and
	 * register it via bcENTER_LOOPER.
	 */

	LKL_BR_FINISHED = _LKL_IO('r', 14),
	/*
	 * not currently supported
	 * stop threadpool thread
	 */

	LKL_BR_DEAD_BINDER = _LKL_IOR('r', 15, lkl_binder_uintptr_t),
	/*
	 * void *: cookie
	 */
	LKL_BR_CLEAR_DEATH_NOTIFICATION_DONE = _LKL_IOR('r', 16, lkl_binder_uintptr_t),
	/*
	 * void *: cookie
	 */

	LKL_BR_FAILED_REPLY = _LKL_IO('r', 17),
	/*
	 * The last transaction (either a bcTRANSACTION or
	 * a bcATTEMPT_ACQUIRE) failed (e.g. out of memory).  No parameters.
	 */

	LKL_BR_FROZEN_REPLY = _LKL_IO('r', 18),
	/*
	 * The target of the last sync transaction (either a bcTRANSACTION or
	 * a bcATTEMPT_ACQUIRE) is frozen.  No parameters.
	 */

	LKL_BR_ONEWAY_SPAM_SUSPECT = _LKL_IO('r', 19),
	/*
	 * Current process sent too many oneway calls to target, and the last
	 * asynchronous transaction makes the allocated async buffer size exceed
	 * detection threshold.  No parameters.
	 */

	LKL_BR_TRANSACTION_PENDING_FROZEN = _LKL_IO('r', 20),
	/*
	 * The target of the last async transaction is frozen.  No parameters.
	 */

	LKL_BR_FROZEN_BINDER = _LKL_IOR('r', 21, struct lkl_binder_frozen_state_info),
	/*
	 * The cookie and a boolean (is_frozen) that indicates whether the process
	 * transitioned into a frozen or an unfrozen state.
	 */

	LKL_BR_CLEAR_FREEZE_NOTIFICATION_DONE = _LKL_IOR('r', 22, lkl_binder_uintptr_t),
	/*
	 * void *: cookie
	 */
};

enum lkl_binder_driver_command_protocol {
	LKL_BC_TRANSACTION = _LKL_IOW('c', 0, struct lkl_binder_transaction_data),
	LKL_BC_REPLY = _LKL_IOW('c', 1, struct lkl_binder_transaction_data),
	/*
	 * binder_transaction_data: the sent command.
	 */

	LKL_BC_ACQUIRE_RESULT = _LKL_IOW('c', 2, __lkl__s32),
	/*
	 * not currently supported
	 * int:  0 if the last LKL_BR_ATTEMPT_ACQUIRE was not successful.
	 * Else you have acquired a primary reference on the object.
	 */

	LKL_BC_FREE_BUFFER = _LKL_IOW('c', 3, lkl_binder_uintptr_t),
	/*
	 * void *: ptr to transaction data received on a read
	 */

	LKL_BC_INCREFS = _LKL_IOW('c', 4, __lkl__u32),
	LKL_BC_ACQUIRE = _LKL_IOW('c', 5, __lkl__u32),
	LKL_BC_RELEASE = _LKL_IOW('c', 6, __lkl__u32),
	LKL_BC_DECREFS = _LKL_IOW('c', 7, __lkl__u32),
	/*
	 * int:	descriptor
	 */

	LKL_BC_INCREFS_DONE = _LKL_IOW('c', 8, struct lkl_binder_ptr_cookie),
	LKL_BC_ACQUIRE_DONE = _LKL_IOW('c', 9, struct lkl_binder_ptr_cookie),
	/*
	 * void *: ptr to binder
	 * void *: cookie for binder
	 */

	LKL_BC_ATTEMPT_ACQUIRE = _LKL_IOW('c', 10, struct lkl_binder_pri_desc),
	/*
	 * not currently supported
	 * int: priority
	 * int: descriptor
	 */

	LKL_BC_REGISTER_LOOPER = _LKL_IO('c', 11),
	/*
	 * No parameters.
	 * Register a spawned looper thread with the device.
	 */

	LKL_BC_ENTER_LOOPER = _LKL_IO('c', 12),
	LKL_BC_EXIT_LOOPER = _LKL_IO('c', 13),
	/*
	 * No parameters.
	 * These two commands are sent as an application-level thread
	 * enters and exits the binder loop, respectively.  They are
	 * used so the binder can have an accurate count of the number
	 * of looping threads it has available.
	 */

	LKL_BC_REQUEST_DEATH_NOTIFICATION = _LKL_IOW('c', 14,
						struct lkl_binder_handle_cookie),
	/*
	 * int: handle
	 * void *: cookie
	 */

	LKL_BC_CLEAR_DEATH_NOTIFICATION = _LKL_IOW('c', 15,
						struct lkl_binder_handle_cookie),
	/*
	 * int: handle
	 * void *: cookie
	 */

	LKL_BC_DEAD_BINDER_DONE = _LKL_IOW('c', 16, lkl_binder_uintptr_t),
	/*
	 * void *: cookie
	 */

	LKL_BC_TRANSACTION_SG = _LKL_IOW('c', 17, struct lkl_binder_transaction_data_sg),
	LKL_BC_REPLY_SG = _LKL_IOW('c', 18, struct lkl_binder_transaction_data_sg),
	/*
	 * binder_transaction_data_sg: the sent command.
	 */

	LKL_BC_REQUEST_FREEZE_NOTIFICATION =
			_LKL_IOW('c', 19, struct lkl_binder_handle_cookie),
	/*
	 * int: handle
	 * void *: cookie
	 */

	LKL_BC_CLEAR_FREEZE_NOTIFICATION = _LKL_IOW('c', 20,
					    struct lkl_binder_handle_cookie),
	/*
	 * int: handle
	 * void *: cookie
	 */

	LKL_BC_FREEZE_NOTIFICATION_DONE = _LKL_IOW('c', 21, lkl_binder_uintptr_t),
	/*
	 * void *: cookie
	 */
};

#endif /* _LKL_LINUX_BINDER_H */

