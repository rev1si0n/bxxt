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
#ifndef _BXXT_UTIL_H_
#define _BXXT_UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <stdbool.h>

#include <inttypes.h>
#include <byteswap.h>

#include <unistd.h>
#include <errno.h>

#include <fcntl.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ctype.h>
#include <stdarg.h>

typedef void (*buffer_free_callback)(void*);
typedef void (*buffer_resize_callback)(void*, int);


struct bxxt_buffer {
        buffer_free_callback    free;
        buffer_resize_callback  resize;

        size_t                  size;
        void*                   ptr;
};

struct bxxt_file {
        struct stat             stat;
        int                     fd;

        char                    name[PATH_MAX];
        uint32_t                o_flags;

        mode_t                  mode;
        size_t                  size;

        uint8_t                 mm;
        uint32_t                mm_flags;
        uint32_t                mm_prot;
        void*                   ptr;
};

#define log1(level, fmt, ...) \
{\
        if (level <= BXXT_LOGLEVEL) { \
        fprintf(stderr, "bxxt \033[1;3%cm[%c]\033[0m " fmt "\n", (level < 0)?'1': '2', \
                        (level < 0)?'!': ((level <= 2)?'-': ((level <= 4)?'+': '*')), \
                                ##__VA_ARGS__); \
        }\
}

typedef struct bxxt_buffer bxxt_buffer_t;
typedef struct bxxt_file bxxt_file_t;


uint32_t bxxt_convert_to_little32(uint32_t n);
uint64_t bxxt_convert_to_little64(uint64_t n);

bxxt_file_t* bxxt_file_new(char* file, uint32_t o_flags, uint32_t mode);
bxxt_file_t* bxxt_file_new_mm(char* file, uint32_t o_flags, uint32_t mode,
                             uint32_t mm_flags, uint32_t mm_mode);

int bxxt_file_open(bxxt_file_t* file);
void bxxt_file_close_no_free(bxxt_file_t* file);
void bxxt_file_close(bxxt_file_t* file);

bxxt_buffer_t* bxxt_buffer_create(size_t size);
bxxt_buffer_t* bxxt_buffer_create_from_data(const void* data, size_t size);
void bxxt_buffer_resize(bxxt_buffer_t* buffer, int adj_size);
size_t bxxt_buffer_concat_from_data(bxxt_buffer_t* buffer, const void* data, size_t size);
size_t bxxt_buffer_align(bxxt_buffer_t* buffer, size_t page_size);
void bxxt_buffer_free(bxxt_buffer_t* buffer);

void* bxxt_malloc(size_t size);
void bxxt_free(void* ptr);
#endif