/*
 * Copyright (c) 2020 Feng Shuo <steve.shuo.feng@gmail.com>
 * This file is part of VirtFS.
 *
 * This file is licensed to you under your choice of the GNU Lesser
 * General Public License, version 3 or any later version (LGPLv3 or
 * later), or the GNU General Public License, version 2 (GPLv2), in
 * all cases as published by the Free Software Foundation.
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <nfsc/libnfs.h>
#include <config.h>
#include <virtfs.h>
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
struct virtfs_p
{
        int flags;
        struct nfs_context *nfs;
        struct nfs_url *url;
};

int virtfs_new(const char *url, virtfs_t **fs)
{
        struct virtfs_p *fsp = NULL;
        int ret = -1;

        fsp = malloc(sizeof(struct virtfs_p));
        if (!fsp)
                return alloc_failed();

        bzero(fsp, sizeof(struct virtfs_p));
        fsp->nfs = nfs_init_context();
        if (fsp->nfs == NULL) {
                ERR("failed to init libnfs context\n");
                ret = -ENOMEM;
                goto err;
        }

        fsp->url = nfs_parse_url_dir(fsp->nfs, url);
        if (fsp->url == NULL) {
                ERR("failed to parse NFS URL\n");
                ret = -EINVAL;
                goto err2;
        }

        *fs = (virtfs_t *)fsp;
        return 0;

err2:
        nfs_destroy_context(fsp->nfs);
err:
        free(fsp);
        return ret;
}

int virtfs_init(virtfs_t *fs)
{
        struct virtfs_p *fsp;
        int ret = -EINVAL;

        if (fs == NULL)
                goto err;

        fsp = (struct virtfs_p *)fs;
        if (fsp->flags & VIRTFS_NFS_FLAG_MOUNT != 0)
                goto err;

        ret = nfs_mount(fsp->nfs, fsp->url->server, fsp->url->path);
        if (ret == 0)
                fsp->flags |= VIRTFS_NFS_FLAG_MOUNT;

err:
        return ret;
}

int virtfs_fini(virtfs_t *fs)
{
        struct virtfs_p *fsp;
        int ret = -EINVAL;

        fsp = (struct virtfs_p *)fs;
        if (fs == NULL)
                goto err;

        ret = 0;

#ifdef HAVE_NFS_UMOUNT
        if (fsp->flags & VIRTFS_NFS_FLAG_MOUNT != 0)
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
