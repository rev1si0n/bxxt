/*
 * (C) Copyright rev1si0n <ihaven0emmail@gmail.com>.  2019.
 *
 * This file is part of bxxt.
 *
 * bxxt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * bxxt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with bxxt.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _BXXT_CPIO_H_
#define _BXXT_CPIO_H_

#include <archive.h>
#include <archive_entry.h>

#include "util.h"
#include "bxxt.h"

#define CPIO_DIR_NOT_EXIST -2
#define CPIO_MAX_SIZE 64*1024*1024 //64MB


typedef struct bxxt_cpio_archive {
        char path[PATH_MAX];
        uint32_t archive_size_gussed;

	struct archive *r;
	struct archive *w;

	struct archive *d_r; // disk reader
	struct archive *d_w; // disk writer

        uint32_t inode;
	int flags;
} bxxt_cpio_t;


static bxxt_cpio_t* bxxt_cpio_archive_new();
static void bxxt_cpio_archive_free(bxxt_cpio_t* cp);
static int bxxt_cpio_archive_read_entry(bxxt_cpio_t* cp, struct archive_entry *e);

static int bxxt_cpio_archive_write(bxxt_cpio_t* cp, char* d_name,
                                char* a_name);
int bxxt_cpio_archive_in(bxxt_buffer_t* bb, const char* path);
static void bxxt_cpio_archive_guess_buffer_size(char*path, void *_cp);
static void bxxt_cpio_archive_add_file(char *path, void *_cp);
int bxxt_cpio_archive_out(const char* path, bxxt_buffer_t** _bb);
static void bxxt_tree_walk(const char *name, void (*fn)(char *path, void* arg), void *arg);
#endif