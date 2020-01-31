/**
 * A utility to display the status of a file or directory.
 *
 * Copyright (C) 2015 Facebook Inc.
 *
 *      This program is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "glfs-stat.h"
#include "glfs-stat-util.h"

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <virtfs.h>

#define AUTHORS "Written by Craig Cabrey."

struct state {
        struct gluster_url *gluster_url;
        struct xlator_option *xlator_options;
        char *url;
        bool debug;
        bool dereference;
};

struct state *state;

static struct option const long_options[] =
{
        {"debug", no_argument, NULL, 'd'},
        {"dereference", no_argument, NULL, 'L'},
        {"help", no_argument, NULL, 'x'},
        {"port", no_argument, NULL, 'p'},
        {"version", no_argument, NULL, 'V'},
        {"xlator-option", required_argument, NULL, 'o'},
        {NULL, no_argument, NULL, 0}
};

static void
usage ()
{
        printf ("Usage: %s [OPTION]... URL\n"
                "Display file status from a remote Gluster volume.\n\n"
                "  -L, --dereference            follow links\n"
                "  -h, --help     display this help and exit\n"
                "  -V, --version  output version information and exit\n\n"
                "Examples:\n"
                "  virtfs-stat virtfs://URL/path/to/file\n"
                "         Stat the file /path/to/file on the VirtFS URL\n"
                "  virtfs-cli (localhost/groot)> stat /file\n"
                "         In the context of a shell with a connection established,\n"
                "         stat a file on the root of the VirtFS URL\n",
                program_invocation_name);
}

static int
parse_options (int argc, char *argv[], bool has_connection)
{
//        uint16_t port = GLUSTER_DEFAULT_PORT;
        int ret = -1;
        int opt = 0;
        int option_index = 0;
//        struct xlator_option *option;

        while (true) {
                opt = getopt_long (argc, argv, "dLVx", long_options,
                                &option_index);

                if (opt == -1) {
                        break;
                }

                switch (opt) {
                        case 'd':
                                state->debug = true;
                                break;
                        case 'L':
                                state->dereference = true;
                                break;
#if 0
                        case 'o':
                                option = parse_xlator_option (optarg);
                                if (option == NULL) {
                                        error (0, errno, "%s", optarg);
                                        goto err;
                                }

                                if (append_xlator_option (&state->xlator_options, option) == -1) {
                                        error (0, errno, "append_xlator_option: %s", optarg);
                                        goto err;
                                }

                                break;
                        case 'p':
                                port = strtoport (optarg);
                                if (port == 0) {
                                        goto out;
                                }

                                break;
#endif
                        case 'V':
                                PRINT_VERSION;
                                ret = -2;
                                goto out;
                        case 'x':
                                usage ();
                                ret = -2;
                                goto out;
                        default:
                                goto err;
                }
        }

        if ((argc - option_index) < 2) {
                error (0, 0, "missing operand");
                goto err;
        } else {
                state->url = strdup (argv[argc - 1]);
                if (state->url == NULL) {
                        error (0, errno, "strdup");
                        goto out;
                }
#if 0
                if (has_connection) {
                        state->gluster_url = gluster_url_init ();
                        if (state->gluster_url == NULL) {
                                error (0, errno, "gluster_url_init");
                                goto out;
                        }

                        state->gluster_url->path = strdup (argv[argc - 1]);
                        if (state->gluster_url->path == NULL) {
                                error (0, errno, "strdup");
                                goto out;
                        }

                        ret = 0;
                        goto out;
                }

                ret = gluster_parse_url (argv[argc - 1], &(state->gluster_url));
                if (ret == -1) {
                        error (0, EINVAL, "%s", state->url);
                        goto err;
                }

                state->gluster_url->port = port;
#endif
                ret = 0;
        }

        goto out;

err:
        error (0, 0, "Try --help for more information.");
out:
        return ret;
}

static struct state*
init_state ()
{
        struct state *state = malloc (sizeof (*state));

        if (state == NULL) {
                goto out;
        }

        state->debug = false;
        state->dereference = false;
        state->gluster_url = NULL;
        state->url = NULL;
        state->xlator_options = NULL;

out:
        return state;
}

static void
print_stat (char *path, struct stat stat)
{
        struct passwd *pw_ent;
        struct group *gw_ent;

        pw_ent = getpwuid (stat.st_uid);
        gw_ent = getgrgid (stat.st_gid);

        long unsigned int mode = stat.st_mode & CHMOD_MODE_BITS;
        long unsigned int uid = stat.st_uid;
        long unsigned int gid = stat.st_gid;

        printf ("  File: `%s'\n", path);
        printf ("  Size: %-10ld\tBlocks: %-10lu IO Block: %-6lu %s\n",
                        stat.st_size,
                        stat.st_blocks,
                        stat.st_blksize,
                        file_type (&stat));
        printf ("Device: %lxh/%lud\tInode: %-10lu Links: %lu\n",
                        stat.st_dev,
                        stat.st_dev,
                        stat.st_ino,
                        stat.st_nlink);
        printf ("Access: (%04lo/%10.10s)  Uid: (%5lu/%8s)   Gid: (%5lu/%8s)\n",
                        mode,
                        human_access (&stat),
                        uid,
                        pw_ent ? pw_ent->pw_name : "UNKNOWN",
                        gid,
                        gw_ent ? gw_ent->gr_name : "UNKNOWN");
        printf ("Access: %s\n", human_time (get_stat_atime (&stat)));
        printf ("Modify: %s\n", human_time (get_stat_mtime (&stat)));
        printf ("Change: %s\n", human_time (get_stat_ctime (&stat)));
}

static int
stat_with_fs(virtfs_t *fs, const char *path)
{
        int ret;
        struct stat statbuf;

        if (state->dereference) {
                ret = virtfs_stat (fs, path, &statbuf);
        } else {
                ret = virtfs_lstat (fs, path, &statbuf);
        }

        if (ret < 0) {
                error (0, -ret, "cannot stat `%s'", state->url);
                goto out;
        }

        print_stat (path, statbuf);

out:
        return ret;
}

static int
stat_without_context ()
{
        virtfs_t *fs = NULL;
        int ret;

        ret = virtfs_new(state->url, &fs);
        if (ret) {
                usage();
                goto out;
        }

        ret = virtfs_init(fs);
        if (ret == -1) {
                error (0, errno, "failed to connect to `%s'", state->url);
                goto out;
        }
#if 0
        ret = apply_xlator_options (fs, &state->xlator_options);
        if (ret == -1) {
                error (0, errno, "failed to apply translator options");
                goto out;
        }

        if (state->debug) {
                ret = glfs_set_logging (fs, "/dev/stderr", GF_LOG_DEBUG);

                if (ret == -1) {
                        error (0, errno, "failed to set logging level");
                        goto out;
                }
        }
#endif
        if (state->debug){
                printf("FS initialized correctly:\n");
                virtfs_dump_info(fs, 0);
        }

        ret = stat_with_fs (fs, NULL);

out:
        if (fs) {
                virtfs_fini (fs);
        }

        return ret;
}

int
do_stat (struct cli_context *ctx)
{
        int argc = ctx->argc;
        char **argv = ctx->argv;
        int ret = -1;

        state = init_state ();
        if (state == NULL) {
                error (0, errno, "failed to initialize state");
                goto out;
        }

        state->debug = ctx->options->debug;

        if (ctx->fs) {
                ret = parse_options (argc, argv, true);
                if (ret != 0) {
                        goto out;
                }

                ret = stat_with_fs(ctx->fs, state->url);
        } else {
                if (ctx->in_shell) {
                        error(0, 0, "Use connect first before stat");
                        goto out;
                }

                ret = parse_options (argc, argv, false);
                switch (ret) {
                        case -2:
                                // Fall through
                                ret = 0;
                        case -1:
                                goto out;
                }

                ret = stat_without_context();
        }

out:
        if (state) {
//                gluster_url_free (state->gluster_url);
                free (state->url);
        }

        free (state);

        return ret;
}
