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
#include <errno.h>
#include "util.h"


int bxxt_file_open(bxxt_file_t* file) {
        file->fd = open(file->name, file->o_flags, file->mode);
        if (file->fd == -1 || fstat(file->fd, &file->stat) != 0)
                goto err;

        file->size = file->stat.st_size;
        if (file->mm == true) {
        file->ptr = mmap(NULL, file->size, file->mm_prot, file->mm_flags,
                         file->fd, 0);
        }
        if (!file->mm || file->ptr != MAP_FAILED)
                return 0;
        err:
        log1(-1, "failed open file %s (%s)", file->name,
                                strerror(errno));
        bxxt_file_close_no_free(file);
        return -1;
}

void bxxt_file_close_no_free(bxxt_file_t* file) {
        if (file->ptr && file->ptr != MAP_FAILED)
                munmap(file->ptr, file->size);
        if (file->fd && file->fd != -1)
                close(file->fd);
}

void bxxt_file_close(bxxt_file_t* file) {
        if (!file)
                return;
        bxxt_file_close_no_free(file);
        bxxt_free(file);
}

bxxt_file_t* bxxt_file_new(char* file, uint32_t o_flags,
                                uint32_t mode) {
        bxxt_file_t *F;
        F = bxxt_malloc(sizeof(*F));

        F->o_flags = o_flags;
        F->mode = mode;
        strcpy(F->name, file);
        return F;
}

bxxt_file_t* bxxt_file_new_mm(char* file, uint32_t o_flags, uint32_t mode,
                                uint32_t mm_flags, uint32_t mm_prot) {
        bxxt_file_t *F;
        F = bxxt_file_new(file, o_flags, mode);

        F->mm = true;
        F->mm_flags = mm_flags;
        F->mm_prot = mm_prot;
        return F;
}

void* bxxt_malloc(size_t size) {
        void *ptr = (void*)malloc(size);
        memset(ptr, 0x0, size);
        return ptr;
}

void bxxt_free(void* ptr) {
        free(ptr);
}

bxxt_buffer_t* bxxt_buffer_create(size_t size) {
        bxxt_buffer_t *b = bxxt_malloc(sizeof(bxxt_buffer_t));

        b->size = size;
        b->ptr = bxxt_malloc(size);
        return b;
}

bxxt_buffer_t* bxxt_buffer_create_from_data(const void* data,
                                        size_t size) {
        bxxt_buffer_t *b = bxxt_buffer_create(size);

        memcpy(b->ptr, data, size);
        return b;
}

size_t bxxt_buffer_concat_from_data(bxxt_buffer_t* buff,
                                        const void* data, size_t size) {
        size_t old_size = buff->size;

        bxxt_buffer_resize(buff, size);
        memcpy(buff->ptr + old_size, data, size);
        return size;
}

void bxxt_buffer_resize(bxxt_buffer_t* buff,
                                        int adj_size) {
        buff->ptr = realloc(buff->ptr, buff->size + adj_size);
        memset(buff->ptr + buff->size, 0x0, adj_size > 0?adj_size:0);
        buff->size += adj_size;
}

size_t bxxt_buffer_align(bxxt_buffer_t* buff,
                                        size_t page_size) {
        size_t extra_size = page_size - (buff->size % page_size);
        extra_size = extra_size % page_size;

        bxxt_buffer_resize(buff, extra_size);
        return extra_size;
}

void bxxt_buffer_free(bxxt_buffer_t* b) {
        if (!b)
                return;
        void* ptr = b->ptr;
        memset(b, 0x0, sizeof(bxxt_buffer_t));

        bxxt_free(b);
        bxxt_free(ptr);
}
