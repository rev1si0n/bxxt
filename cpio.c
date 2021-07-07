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
#include "cpio.h"

#include <archive.h>
#include <archive_entry.h>

struct dir_entries {
        char    **entries;
        size_t  count;
};

static int compare(const void* a, const void* b) {
        return strcmp(*(const char**)a, *(const char**)b);
}

static void bxxt_cpio_walk_entries(char *path, void *de) {
        struct dir_entries* wde = de;
        wde->count++;
        wde->entries = realloc(wde->entries,
                        wde->count*sizeof(char*));
        wde->entries[wde->count -1] = strdup(path);
}

static void bxxt_tree_walk(const char *name,
        void (*fn)(char *path, void* arg), void *arg) {
        DIR *D;
        struct dirent *de;
        char path[PATH_MAX];
        if (!(D = opendir(name)))
                return;

        while ((de = readdir(D)) != NULL) {
        snprintf(path, sizeof(path), "%s/%s", name, de->d_name);
        if (de->d_type == DT_DIR) {
                if (strcmp(de->d_name, ".") && strcmp(de->d_name, "..")) {
                        bxxt_tree_walk(path, fn, arg);
                        fn(path, arg);
                }
        } else
                fn(path, arg);
        }
        closedir(D);
}

static bxxt_cpio_t* bxxt_cpio_archive_new() {
	int flags;
	flags =  ARCHIVE_EXTRACT_TIME;
	flags |= ARCHIVE_EXTRACT_PERM;
	// flags |= ARCHIVE_EXTRACT_OWNER; owner in android cpio always 0

        bxxt_cpio_t* cp;
        cp = (bxxt_cpio_t*)bxxt_malloc(sizeof(bxxt_cpio_t));

	cp->r = archive_read_new();
	archive_read_support_format_cpio(cp->r);
	archive_read_support_compression_gzip(cp->r);

        cp->d_r = NULL; // we never read boot.img cpio from disk

	cp->w = archive_write_new();
	archive_write_add_filter_gzip(cp->w);
	archive_write_set_options(cp->w, "gzip:compression-level=6");
	archive_write_set_options(cp->w, "gzip:!timestamp");

	archive_write_set_format_cpio_newc(cp->w);

	cp->d_w = archive_write_disk_new();
	archive_write_disk_set_options(cp->d_w, flags);
	archive_write_disk_set_standard_lookup(cp->d_w);

        cp->inode = 300000; // android boot inode
        cp->flags = flags;
        return cp;
}

static void bxxt_cpio_archive_free(bxxt_cpio_t* cp) {
	archive_write_close(cp->w);
	archive_write_free(cp->w);

	archive_write_close(cp->d_w);
	archive_write_free(cp->d_w);

	archive_read_close(cp->r);
	archive_read_free(cp->r);

        bxxt_free(cp);
}

static int bxxt_cpio_archive_read_entry(bxxt_cpio_t* cp, struct archive_entry *e) {
        char fpath[PATH_MAX];

	la_int64_t offset;
	size_t size;
	const void *buff;

        sprintf(fpath, "%s/%s", cp->path, archive_entry_pathname(e));
        log1(8, "archive > mode=%03o %s", archive_entry_mode(e) & 0x1ff,
                                        archive_entry_pathname(e))

	archive_entry_set_pathname(e, fpath);
	archive_write_header(cp->d_w, e);

        int r;

	while (1) {
	r = archive_read_data_block(cp->r, &buff, &size, &offset);
	if (r == ARCHIVE_EOF || r < ARCHIVE_OK)
		break;
	r = archive_write_data_block(cp->d_w, buff, size, offset);
	if (r < ARCHIVE_OK)
		break;
	}
	archive_write_finish_entry(cp->d_w);
        return ARCHIVE_OK;
}

static int bxxt_cpio_archive_write(bxxt_cpio_t* cp, char* d_name,
                                char* a_name) {
	char link[PATH_MAX];
	struct stat st;

	char buff[4096];
	int fd, len;

	if (lstat(d_name, &st) != 0)
                return ARCHIVE_FAILED;

	struct archive_entry *e;
	e = archive_entry_new();

	archive_entry_set_pathname(e, a_name);
	archive_entry_set_size(e, st.st_size);

	archive_entry_set_perm(e, st.st_mode);

	archive_entry_set_uid(e, 0);
	archive_entry_set_gid(e, 0);

        memset(link, 0x0, sizeof(link));

	switch (st.st_mode & S_IFMT) {
case S_IFREG:
        archive_entry_set_filetype(e, AE_IFREG);
        break;
case S_IFDIR:
        archive_entry_set_filetype(e, AE_IFDIR);
        break;
case S_IFLNK:
        len = readlink(d_name, link, sizeof(link));
        archive_entry_set_filetype(e, AE_IFLNK);
        archive_entry_set_symlink(e, link);
        break;
default:
        log1(-1, "unknown type file %s", d_name)
        break; //should die
	}

        archive_entry_set_mtime(e, 0, 0);
        log1(8, "archive < mode=%03o %s",
                                st.st_mode & 0x1ff, a_name)

	archive_entry_set_nlink(e, 1);

	archive_entry_set_rdev(e, 0);
	archive_entry_set_dev(e, 0);

	archive_entry_set_ino(e, cp->inode++);
	archive_write_header(cp->w, e);

	if (archive_entry_size(e) > 0) {
	fd = open(d_name, O_RDONLY);
	while ((len = read(fd, buff, sizeof(buff))) > 0)
		archive_write_data(cp->w, buff, len);
	close(fd);
	}

	archive_entry_free(e);
        return ARCHIVE_OK;
}

int bxxt_cpio_archive_in(bxxt_buffer_t* bb, const char* path) {
        char rpath[PATH_MAX];
        if (realpath(path, rpath) == NULL)
                return ARCHIVE_FAILED;
        bxxt_cpio_t *cp = bxxt_cpio_archive_new();
        strcpy(cp->path, rpath);

        int r;

        struct archive_entry *e;
	archive_read_open_memory(cp->r, bb->ptr, bb->size);

        log1(5, "archive > working on %s", cp->path);

	for (;(r = archive_read_next_header(cp->r, &e)) != ARCHIVE_EOF;) {
        bxxt_cpio_archive_read_entry(cp, e);
        }

        bxxt_cpio_archive_free(cp);
        return ARCHIVE_OK;
}

static void bxxt_cpio_archive_guess_buffer_size(char*path, void *_cp) {
        struct stat st;
        bxxt_cpio_t *cp = _cp;
        memset(&st, 0x0, sizeof(struct stat));

        lstat(path, &st);

        cp->archive_size_gussed += st.st_size;
        cp->archive_size_gussed += PATH_MAX;
}

static void bxxt_cpio_archive_add_file(char *path, void *_cp) {
        bxxt_cpio_t *cp = _cp;

        char *cp_path = path + strlen(cp->path) + 1; // name stored in archive
        bxxt_cpio_archive_write(cp, path, cp_path);
}

int bxxt_cpio_archive_out(const char* path, bxxt_buffer_t** _bb) {
        char rpath[PATH_MAX];
        if (realpath(path, rpath) == NULL)
                return ARCHIVE_FAILED;
        bxxt_cpio_t *cp = bxxt_cpio_archive_new();
        strcpy(cp->path, rpath);

        log1(8, "archive < packing %s", cp->path);

        // check path
        bxxt_tree_walk(rpath, bxxt_cpio_archive_guess_buffer_size, (void*)cp);
        bxxt_buffer_t* bb = bxxt_buffer_create(cp->archive_size_gussed);

        log1(8, "archive < create buffer, %x gussed", cp->archive_size_gussed)

        size_t used = 0;
        archive_write_open_memory(cp->w, bb->ptr, bb->size, &used);

        struct dir_entries wde;
        wde.entries = malloc(0);
        wde.count = 0;

        bxxt_tree_walk(rpath, bxxt_cpio_walk_entries, (void*)&wde);
        qsort(wde.entries, wde.count, sizeof(char*), compare);

        for (int i = 0; i < wde.count; i++) {
                bxxt_cpio_archive_add_file(wde.entries[i], (void*)cp);
        free(wde.entries[i]);
        }

        free(wde.entries);
        bxxt_cpio_archive_free(cp);

        // resize output buffer to the real size of cpio data
	bxxt_buffer_resize(bb, used - bb->size);
        log1(8, "archive < size is %zx bytes", bb->size)

        *_bb = bb;
        return ARCHIVE_OK;
}
