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

#include <virtfs.h>
#include <virtfs_log.h>

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
