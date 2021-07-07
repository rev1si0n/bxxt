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
#ifndef _BXXT_BOOT_H_
#define _BXXT_BOOT_H_

// https://source.android.com/devices/bootloader/boot-image-header?hl=zh-cn#boot-image-header-version-2

// v3, v4
// https://android.googlesource.com/platform/system/tools/mkbootimg/+/refs/heads/master/include/bootimg/bootimg.h

#include <stdint.h>
#include <stdio.h>

#define   BOOT_HDR_VERSION_MAX  2

#define   BOOT_MAGIC            "ANDROID!"
#define   BOOT_MAGIC_SIZE       0x0008
#define   BOOT_NAME_SIZE        0x0010
#define   BOOT_ARGS_SIZE        0x0200
#define   BOOT_EXTRA_ARGS_SIZE  0x0400

#define   BOOT_MAX_DTB          0xff

struct boot_img_hdr {
        uint8_t magic[BOOT_MAGIC_SIZE];
        uint32_t kernel_size;  /* size in bytes */
        uint32_t kernel_addr;  /* physical load addr */

        uint32_t ramdisk_size; /* size in bytes */
        uint32_t ramdisk_addr; /* physical load addr */

        uint32_t second_size;  /* size in bytes */
        uint32_t second_addr;  /* physical load addr */

        uint32_t tags_addr;    /* physical addr for kernel tags */
        uint32_t page_size;    /* flash page size we assume */
        uint32_t header_version;
        uint32_t os_version;
        uint8_t name[BOOT_NAME_SIZE]; /* asciiz product name */
        uint8_t cmdline[BOOT_ARGS_SIZE];
        uint32_t id[8]; /* timestamp / checksum / sha1 / etc */
        uint8_t extra_cmdline[BOOT_EXTRA_ARGS_SIZE];
        uint32_t recovery_dtbo_size;   /* size of recovery dtbo image */
        uint64_t recovery_dtbo_offset; /* offset in boot image */
        uint32_t header_size;   /* size of boot image header in bytes */
        uint32_t dtb_size;   /* size of dtb image */
        uint64_t dtb_addr; /* physical load address */
} __attribute__((packed));

struct bxxt_boot_kernel_dtb {
        size_t size;
        void* ptr;
};

typedef struct bxxt_boot_kernel_dtb bxxt_dtb_t;
typedef struct bxxt_boot_kernel_dtb bxxt_boot_kernel_dtb_t;
typedef struct bxxt_boot {
        struct boot_img_hdr hdr;
        char path[PATH_MAX]; // path for extract
        bxxt_file_t *image;

        // kernel padding dtb
        bxxt_dtb_t kernel_dtb[BOOT_MAX_DTB];
        uint32_t kernel_dtb_count;

        bxxt_dtb_t dtb[BOOT_MAX_DTB];
        uint32_t dtb_count;

        void* mm;
        uint32_t version;

        uint32_t page_size;
        int kernel_compression;

        uint32_t kernel_size;
        void* mm_kernel_ptr;

        uint32_t ramdisk_size;
        void* mm_ramdisk_ptr;

        uint32_t second_size;
        void* mm_second_ptr;

        uint32_t recovery_dtbo_size;
        void* mm_recovery_dtbo_ptr;

        uint32_t dtb_size;
        void* mm_dtb_ptr;

        uint32_t pad_size;
        void* mm_pad_ptr;
} bxxt_boot_t;

enum {
        KERNEL_COMPRESSION_NONE = 0,
        KERNEL_COMPRESSION_GZIP,
        KERNEL_COMPRESSION_BZIP2,
        KERNEL_COMPRESSION_LZMA,
        KERNEL_COMPRESSION_XZ,
        KERNEL_COMPRESSION_LZ4
};

#define BXXT_IMAGE_INVALID -1

static void bxxt_boot_free(bxxt_boot_t* b);

static int bxxt_boot_open_file(bxxt_boot_t* b, char* file);
static int bxxt_boot_detect_kernel_compression(bxxt_boot_t* b);
static bxxt_boot_t* bxxt_boot_new();
static int bxxt_boot_print_meta_info_file(bxxt_boot_t* b, FILE* fd);
static char * bxxt_boot_img_get_signature_string(bxxt_boot_t* b);
static uint8_t* bxxt_boot_img_signature(bxxt_boot_t* b);


static int bxxt_boot_img_compat_check(bxxt_boot_t* b);
static int bxxt_boot_img_version_check(bxxt_boot_t* b);

#endif
