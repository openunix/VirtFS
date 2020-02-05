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

#include <dirent.h>

/* Portability non glibc c++ build systems */
#ifndef __THROW
# if defined __cplusplus
#  define __THROW       throw ()
# else
#  define __THROW
# endif
#endif

__BEGIN_DECLS

/* The struct is private. Use the typedef pointer instead. */
typedef struct virtfs *virtfs_t;

/* Create a new VirtFS object. */
int virtfs_new(const char *spec, virtfs_t *fs_out) __THROW;

/* Set additional options */
int virtfs_setopt(virtfs_t fs_in, const char *opt) __THROW;

/* Initialize, mount the filesystem */
int virtfs_init(virtfs_t fs_in) __THROW;

/* Finish, umount & free the filesystem */
int virtfs_fini(virtfs_t fs_in) __THROW;

/* Dump debug information */
void virtfs_dump_info(virtfs_t fs_in, int verbose) __THROW;
void virtfs_clean() __THROW;

/* stat() and lstat() */
int virtfs_stat(virtfs_t fs_in, const char *path, struct stat *buf) __THROW;
int virtfs_lstat(virtfs_t fs_in, const char *path, struct stat *buf) __THROW;

typedef struct virtfs_fd *virtfs_fd_t;
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

/* virtfs_dir_t equals to DIR * */
typedef struct virtfs_dir *virtfs_dir_t;
#define vdir_t virtfs_dir_t

/* opendir() and closeddir() */
int virtfs_opendir(virtfs_t fs_in, const char *path, virtfs_dir_t *dir_out) __THROW;
int virtfs_closedir(virtfs_dir_t dir_in) __THROW;

/* readdir() plus stat() */
struct dirent *virtfs_readdirplus(virtfs_dir_t dir, struct stat *st_out) __THROW;

/* utilities to maintain URL and path */
char *virtfs_append_path(const char *base_path, const char *hanging_path) __THROW;
char *virtfs_url_get_path(virtfs_t fs) __THROW;
__END_DECLS

#endif /* !_VIRTFS_H */

