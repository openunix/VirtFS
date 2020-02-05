/**
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

#ifndef GLFS_CLI_H
#define GLFS_CLI_H

#include <virtfs.h>
#include <stdbool.h>

struct fd_list {
        struct fd_list *next;
        virtfs_fd_t fd;
        char *path;
};

struct cli_context {
        virtfs_t fs;
        struct fd_list *flist;
        char *url;
        struct options *options;
        char *conn_str;
        int argc;
        bool in_shell;
        char **argv;
};

struct options {
        struct xlator_option *xlator_options;
        bool debug;
};

#define COPYRIGHT \
"Based on glusterfs-coreutils:\n" \
"  Copyright (C) 2015 Facebook, Inc."
#define LICENSE \
"  License GPLv3: GNU GPL version 3 <http://www.gnu.org/licenses/gpl-3.0.en.html>.\n" \
"  This is free software: you are free to change and redistribute it.\n" \
"  There is NO WARRANTY, to the extent permitted by law."

#define PRINT_VERSION do \
{ \
        printf ("%s (%s) %s\n\n%s\n%s\n  %s\n", \
                program_invocation_name, \
                PACKAGE_NAME,    \
                PACKAGE_VERSION,  \
                COPYRIGHT,  \
                LICENSE,  \
                AUTHORS);  \
}while(0)

#endif
