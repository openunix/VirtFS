/*
 * Copyright (c) 2020 Feng Shuo <steve.shuo.feng@gmail.com>
 * This file is part of VirtFS.
 *
 * This file is licensed to you under your choice of the GNU Lesser
 * General Public License, version 3 or any later version (LGPLv3 or
 * later), or the GNU General Public License, version 2 (GPLv2), in
 * all cases as published by the Free Software Foundation.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include <nfsc/libnfs.h>
#include <virtfs.h>
#include <virtfs_log.h>
#include "virtfs_i.h"

/*
 * Copied from libfuse lib/fuse_opt.c and lib/fuse_log.c
 */
static void default_log_func(
		__attribute__(( unused )) enum virtfs_log_level level,
		const char *fmt, va_list ap)
{
	vfprintf(stderr, fmt, ap);
}

static virtfs_log_func_t log_func = default_log_func;

void virtfs_set_log_func(virtfs_log_func_t func)
{
	if (!func)
		func = default_log_func;

	log_func = func;
}

void virtfs_log(enum virtfs_log_level level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	log_func(level, fmt, ap);
	va_end(ap);
}

static int alloc_failed(void)
{
	virtfs_log(VIRTFS_LOG_ERR, "fuse: memory allocation failed\n");
	return -1;
}

static int add_opt_common(char **opts, const char *opt, int esc)
{
	unsigned oldlen = *opts ? strlen(*opts) : 0;
	char *d = realloc(*opts, oldlen + 1 + strlen(opt) * 2 + 1);

	if (!d)
		return alloc_failed();

	*opts = d;
	if (oldlen) {
		d += oldlen;
		*d++ = ',';
	}

	for (; *opt; opt++) {
		if (esc && (*opt == ',' || *opt == '\\'))
			*d++ = '\\';
		*d++ = *opt;
	}
	*d = '\0';

	return 0;
}

#define VIRTFS_NFS_FLAG_MOUNT 0x0001
struct virtfs
{
        int flags;
        enum virtfs_log_level log;
        struct nfs_context *nfs;
        struct nfs_url *url;
};

int virtfs_new(const char *url, virtfs_t *fs_out)
{
        struct virtfs *fsp;
        int ret = -1;

        fsp = malloc(sizeof(struct virtfs));
        if (!fsp)
                return alloc_failed();

        bzero(fsp, sizeof(struct virtfs));
        fsp->nfs = nfs_init_context();
        if (fsp->nfs == NULL) {
                ERR("failed to init libnfs context\n");
                ret = -ENOMEM;
                goto err;
        }

        fsp->url = nfs_parse_url_incomplete(fsp->nfs, url);
        if (fsp->url == NULL ||
            fsp->url->server == NULL ||
            fsp->url->path == NULL) {
                ERR("failed to parse NFS URL\n");
                ret = -EINVAL;
                goto err2;
        }

        *fs_out = fsp;
        return 0;

err2:
        nfs_destroy_context(fsp->nfs);
        nfs_destroy_url(fsp->url);
err:
        free(fsp);
        return ret;
}

int virtfs_init(virtfs_t fs)
{
        struct virtfs *fsp;
        int ret = -EINVAL;

        if (fs == NULL)
                goto err;

        fsp = fs;
        if (fsp->flags & VIRTFS_NFS_FLAG_MOUNT != 0)
                goto err;

        ret = nfs_mount(fsp->nfs, fsp->url->server, fsp->url->path);
        if (ret == 0)
                fsp->flags |= VIRTFS_NFS_FLAG_MOUNT;

err:
        return ret;
}

int virtfs_fini(virtfs_t fs)
{
        struct virtfs *fsp;
        int ret = -EINVAL;

        fsp = fs;
        if (fs == NULL)
                goto err;

        ret = 0;

#ifdef HAVE_NFS_UMOUNT
        if ((fsp->flags & VIRTFS_NFS_FLAG_MOUNT) != 0)
                ret = nfs_umount(fsp->nfs);
#endif /* HAVE_NFS_UMOUNT */

        if (fsp->url)
                nfs_destroy_url(fsp->url);
        if (fsp->nfs)
                nfs_destroy_context(fsp->nfs);
err:
        return ret;
}

int virtfs_close(vfd_t vfd)
{
        return -ENOTSUP;
}

#define __COPY_ATTR(x) buf->st_##x = buf64.nfs_##x
#define __COPY_ATTR2(x, y) buf->x = buf64.y
static int _do_nfs_stat(virtfs_t fs, const char *path, struct stat *buf,
        int (*f)(struct nfs_context *, const char *, struct nfs_stat_64 *))
{
        struct nfs_stat_64 buf64;
        struct virtfs *fsp;
        int ret = -EINVAL;

        fsp = fs;
        if (fs == NULL)
                goto err;

        if (path)
                ret = f(fsp->nfs, path, &buf64);
        else if (fsp->url->file)
                ret = f(fsp->nfs, fsp->url->file, &buf64);
        else
                ret = f(fsp->nfs, "/", &buf64);

        __COPY_ATTR(dev);
        __COPY_ATTR(ino);
        __COPY_ATTR(mode);
        __COPY_ATTR(nlink);
        __COPY_ATTR(uid);
        __COPY_ATTR(gid);
        __COPY_ATTR(rdev);
        __COPY_ATTR(size);
        __COPY_ATTR(blksize);
        __COPY_ATTR(blocks);
        __COPY_ATTR(atime);
        __COPY_ATTR(mtime);
        __COPY_ATTR(ctime);
#ifdef HAVE_STRUCT_STAT_ST_ATIM
        __COPY_ATTR2(st_atim.tv_nsec, nfs_atime_nsec);
        __COPY_ATTR2(st_mtim.tv_nsec, nfs_mtime_nsec);
        __COPY_ATTR2(st_ctim.tv_nsec, nfs_ctime_nsec);
#else
        /* Apple? */
#endif

err:
        return ret;
}
#undef __COPY_ATTR
#undef __COPY_ATTR2

int virtfs_stat(virtfs_t fs, const char *path, struct stat *buf)
{
        return _do_nfs_stat(fs, path, buf, nfs_stat64);
}

int virtfs_lstat(virtfs_t fs, const char *path, struct stat *buf)
{
        return _do_nfs_stat(fs, path, buf, nfs_lstat64);
}

void virtfs_dump_info(virtfs_t fs, int verbose)
{
        struct virtfs *fsp;
        int ret = -EINVAL;

        fsp = fs;
        DEBUG("fs: %p, ", fsp);
        if (fs == NULL)
                goto err;

        DEBUG("flags: %x, server: %s, path: %s, file: %s\n", fsp->flags,
              fsp->url->server, fsp->url->path, fsp->url->file);

err:
        return;
}

struct virtfs_dir
{
        struct nfs_context *nfs;
        struct nfsdir *nfsdir;
};

int virtfs_opendir(virtfs_t fs_in, const char *path, virtfs_dir_t *dir_out)
{
        struct virtfs *fsp;
        struct virtfs_dir *dir;
        int ret = -EINVAL;

        fsp = fs_in;
        if (fsp == NULL)
                goto err;

        dir = malloc(sizeof(struct virtfs_dir));
        if (!dir)
                return alloc_failed();
        dir->nfs = fsp->nfs;
        ret = nfs_opendir(dir->nfs, path, &(dir->nfsdir));
        if (ret)
                goto err2;
        *dir_out = dir;

        return 0;
err2:
        free(dir);
err:
        return ret;
}

int virtfs_closedir(virtfs_dir_t dir_in)
{
        int ret = -EINVAL;

        if (dir_in->nfs == NULL || dir_in->nfsdir == NULL)
                goto out;
        nfs_closedir(dir_in->nfs, dir_in->nfsdir);
        ret = 0;
out:
        free(dir_in);
        return ret;
}

struct virtfs_dirent
{
        struct dirent ent;
        struct nfsdirent *nfsent;
};

#define __COPY_ATTR2(x, y) buf->x = ndirp->y
struct dirent *virtfs_readdirplus(virtfs_dir_t dir, struct stat *buf)
{
        struct virtfs_dirent *dirp;
        struct nfsdirent *ndirp;

        ndirp = nfs_readdir(dir->nfs, dir->nfsdir);
        if (!ndirp)
                return NULL;

        dirp = malloc(sizeof(struct virtfs_dirent));
        if (!dirp) {
                free(ndirp);
                alloc_failed();
                return NULL;
        }

        bzero(dirp, sizeof(struct virtfs_dirent));
        dirp->nfsent = ndirp;
        dirp->ent.d_ino = ndirp->inode;
        strncpy(dirp->ent.d_name, ndirp->name, 255);

        __COPY_ATTR2(st_dev, dev);
        __COPY_ATTR2(st_ino, inode);
        __COPY_ATTR2(st_mode, mode);
        __COPY_ATTR2(st_nlink, nlink);
        __COPY_ATTR2(st_uid, uid);
        __COPY_ATTR2(st_gid, gid);
        __COPY_ATTR2(st_rdev, rdev);
        __COPY_ATTR2(st_size, size);
        __COPY_ATTR2(st_blksize, blksize);
        __COPY_ATTR2(st_blocks, blocks);
        __COPY_ATTR2(st_atime, atime.tv_sec);
        __COPY_ATTR2(st_mtime, mtime.tv_sec);
        __COPY_ATTR2(st_ctime, ctime.tv_sec);
#ifdef HAVE_STRUCT_STAT_ST_ATIM
        buf->st_atim.tv_nsec = ndirp->atime.tv_usec * 1000 + ndirp->atime_nsec;
        buf->st_mtim.tv_nsec = ndirp->mtime.tv_usec * 1000 + ndirp->mtime_nsec;
        buf->st_ctim.tv_nsec = ndirp->ctime.tv_usec * 1000 + ndirp->ctime_nsec;
#else
        /* Apple? */
#endif
        dirp->ent.d_type = IFTODT(buf->st_mode);

        return &(dirp->ent);
}

char *
virtfs_append_path (const char *base_path, const char *hanging_path)
{
        size_t new_length;
        size_t base_path_length = strlen (base_path);
        size_t hanging_path_length = strlen (hanging_path);
        const char *fmt;
        char *new_path;

        if (base_path_length > 0 && base_path[base_path_length - 1] == '/') {
                new_length = base_path_length + hanging_path_length + 1;
                fmt = "%s%s";
        } else {
                new_length = base_path_length + hanging_path_length + 2;
                fmt = "%s/%s";
        }

        new_path = malloc (new_length);
        if (new_path == NULL) {
                goto out;
        }

        snprintf (new_path, new_length, fmt, base_path, hanging_path);

out:
        return new_path;
}

char *virtfs_url_get_path(virtfs_t fs)
{
        struct nfs_url *nfs_url = fs->url;

        if (!nfs_url || !nfs_url->path)
                return NULL;
        if (!nfs_url->file)
                return strdup(nfs_url->path);

        return virtfs_append_path(nfs_url->path, nfs_url->file);
}

int virtfs_set_log_level(virtfs_t fs, enum virtfs_log_level level)
{
        fs->log = level;

        return 0;
}
