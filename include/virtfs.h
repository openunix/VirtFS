/*
 * Copyright (c) 2016 Feng Shuo <steve.shuo.feng@gmail.com>
 * This file is part of VirtFS.
 *
 * This file is licensed to you under your choice of the GNU Lesser
 * General Public License, version 3 or any later version (LGPLv3 or
 * later), or the GNU General Public License, version 2 (GPLv2), in
 * all cases as published by the Free Software Foundation.
 */

#ifndef _VIRTFS_H
#define _VIRTFS_H

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/stat.h>

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include <unistd.h>

/* Portability non glibc c++ build systems */
#ifndef __THROW
# if defined __cplusplus
#  define __THROW       throw ()
# else
#  define __THROW
# endif
#endif

__BEGIN_DECLS

typedef struct virtfs virtfs_t;

int virtfs_new(const char *spec, virtfs_t **vfs) __THROW;
int virtfs_init(virtfs_t *vfs) __THROW;
int virtfs_add_option(virtfs_t *vfs, const char *opt) __THROW;
int virtfs_fini(virtfs_t *vfs) __THROW;
void virtfs_dump_info(virtfs_t *vfs, int verbose) __THROW;
void virtfs_clean() __THROW;

int virtfs_stat(virtfs_t *fs, const char *path, struct stat *buf) __THROW;
int virtfs_lstat(virtfs_t *fs, const char *path, struct stat *buf) __THROW;

typedef void *virtfs_fd_t;
#define vfd_t virtfs_fd_t

vfd_t virtfs_openuri(const char *uri, int flags, ...) __THROW;
int virtfs_close(vfd_t vfd) __THROW;
int virtfs_fd_to_posix(vfd_t vfd) __THROW;
vfd_t virtfs_fd_from_posix(int fd) __THROW;
int virtfs_ftruncate(vfd_t vfd, off_t length) __THROW;
int virtfs_fstat(vfd_t vfd, struct stat *buf) __THROW;
int virtfs_read(vfd_t vfd, void *buf, size_t count) __THROW;
int virtfs_write(vfd_t vfd, const void *buf, size_t count) __THROW;
int virtfs_lseek(vfd_t vfd, off_t offset, int whence) __THROW;

__END_DECLS

#endif /* !_VIRTFS_H */

