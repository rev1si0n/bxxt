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
#include <stdbool.h>

#include "util.h"
#include "cpio.h"
#include "boot.h"
#include "bxxt.h"

#include "sha1.h"
#include "dtcc.h"

static int force_no_compat_check = false;
static int skip_unknown_data = false;

static bxxt_boot_t* bxxt_boot_new() {
        bxxt_boot_t* b;
        b = bxxt_malloc(sizeof(bxxt_boot_t));
        b->image = NULL;
        return b;
}

static char* bxxt_boot_img_get_patchlevel_string(bxxt_boot_t* b) {
        uint32_t i = b->hdr.os_version;
        static char patchlevel[32];

        sprintf(patchlevel, "%04d-%02d-%02d",
                                        2000 + (i >> 4 & 0b1111111),
                                        i >> 0 & 0b0000111, 0);
        return patchlevel;
}

static char* bxxt_boot_img_get_osver_string(bxxt_boot_t* b) {
        uint32_t i = b->hdr.os_version;
        static char osver[32];
        unsigned d, e, f;

        d = i >> (32 - 7*1) & 0b1111111;
        e = i >> (32 - 7*2) & 0b1111111;
        f = i >> (32 - 7*3) & 0b1111111;

        sprintf(osver, "%d.%d.%d", d, e, f);
        return osver;
}

static int bxxt_boot_print_meta_info_file(bxxt_boot_t* b, FILE* fd) {
#define p_meta(fmt, value) fprintf(fd, "bxxt." fmt, value)
        p_meta("kernel_addr=%x\n",        b->hdr.kernel_addr);
        p_meta("ramdisk_addr=%x\n",       b->hdr.ramdisk_addr);
        p_meta("second_addr=%x\n",        b->hdr.second_addr);
        p_meta("tags_addr=%x\n",          b->hdr.tags_addr);
        p_meta("dtb_addr=%" PRIx64 "\n",  b->hdr.dtb_addr);

        p_meta("recovery_dtbo_offset=%" PRIx64 "\n",  b->hdr.recovery_dtbo_offset);

        p_meta("name=%s\n",               b->hdr.name);

        p_meta("cmdline=%s\n",            b->hdr.cmdline);
        p_meta("extra_cmdline=%s\n",      b->hdr.extra_cmdline);

        p_meta("kernel_compression=%x\n", bxxt_boot_detect_kernel_compression(b));
        p_meta("header_version=%x\n",     b->hdr.header_version);
        p_meta("os_version=%x\n",         b->hdr.os_version);
        p_meta("header_size=%x\n",        b->hdr.header_size);
        p_meta("page_size=%x\n",          b->hdr.page_size);
#if 0
        p_meta("signature=%s\n",          bxxt_boot_img_get_signature_string(b));
#endif
        p_meta("kernel_size=%x\n",        b->kernel_size);
        p_meta("ramdisk_size=%x\n",       b->ramdisk_size);
        p_meta("second_size=%x\n",        b->second_size);
        p_meta("recovery_dtbo_size=%x\n", b->recovery_dtbo_size);
        p_meta("dtb_size=%x\n",           b->dtb_size);
#undef p_meta
        return BXXT_SUCCESS;
}

int bxxt_boot_scan_metadata(char* file, const char* name,
        char* fmt, void* out) {
        int r = BXXT_FAILED;
        char buffer[4096] = {0};
        bxxt_file_t *config;

        config = bxxt_file_new_mm(file, O_RDONLY, 0644,
                                MAP_PRIVATE, PROT_READ);

        if (bxxt_file_open(config) != 0)
                goto error;

        char* p;
        sprintf(buffer, "%s=", name);
        p = strstr(config->ptr, buffer);
        if (!p)
                goto notexist;
        sprintf(buffer, "%s=%s", name, fmt);
        r = sscanf(p, buffer, out);

        notexist:
        if (r != 1)
                log1(-2, "meta-field '%s' not set", name)
        error:
        bxxt_file_close(config);
        return r;
}

int bxxt_boot_hdr_from_metadata(struct boot_img_hdr *hdr, char* file) {
        int r;
        memset(hdr, 0x0, sizeof(*hdr));

#define config_xint(n, dst)     bxxt_boot_scan_metadata(file, "bxxt." n, "%x", (dst))
#define config_string(n, dst)   bxxt_boot_scan_metadata(file, "bxxt." n, "%[^\n]", (dst))

#define RETIF(cond) \
{\
        if ((cond)) { \
                return BXXT_FAILED; \
        }\
}
        r = config_string("cmdline", &hdr->cmdline);
        RETIF(strlen(hdr->cmdline) >= BOOT_ARGS_SIZE)

        r = config_string("extra_cmdline", &hdr->extra_cmdline);
        RETIF(strlen(hdr->extra_cmdline) >= BOOT_EXTRA_ARGS_SIZE)

        r = config_string("name", &hdr->name);
        RETIF(strlen(hdr->name) >= BOOT_NAME_SIZE)

        r = config_xint("header_size", &hdr->header_size);
        RETIF(r != 1)

        r = config_xint("kernel_addr", &hdr->kernel_addr);
        RETIF(r != 1)

        r = config_xint("ramdisk_addr", &hdr->ramdisk_addr);
        RETIF(r != 1)

        r = config_xint("second_addr", &hdr->second_addr);
        RETIF(r != 1)

        r = config_xint("tags_addr", &hdr->tags_addr);
        RETIF(r != 1)

        r = config_xint("page_size", &hdr->page_size);
        RETIF(r != 1)

        r = config_xint("header_version", &hdr->header_version);
        RETIF(r != 1)

        r = config_xint("header_size", &hdr->header_size);
        RETIF(r != 1)

        r = config_xint("os_version", &hdr->os_version);
        RETIF(r != 1)

        r = config_xint("recovery_dtbo_offset",
                                &hdr->recovery_dtbo_offset);
        RETIF(r != 1)

        r = config_xint("dtb_addr", &hdr->dtb_addr);
        RETIF(r != 1)

	memcpy((void*)hdr, BOOT_MAGIC, sizeof(BOOT_MAGIC));
        return BXXT_SUCCESS;
#undef config_xint
#undef config_string
#undef RETIF
}

static int bxxt_boot_img_version_check(bxxt_boot_t* b) {
        if (*(uint64_t*)&b->hdr != *(uint64_t*)BOOT_MAGIC) {
                log1(-1, "invalid android boot image")
                return BXXT_FAILED;
        }
        if (b->hdr.header_version <= 2)
                return BXXT_SUCCESS; //max support v2
        log1(-1, "image version %x not supported", b->hdr.header_version)
        return BXXT_FAILED;
}

static char* bxxt_boot_img_get_signature_string(bxxt_boot_t* b) {
        static char sha1sum[sizeof(uint32_t)*32] = {0};
        for (int i = 0; i < sizeof(b->hdr.id); i++)
                sprintf(sha1sum + (i*2), "%02X", ((uint8_t*)b->hdr.id)[i]);
        return sha1sum;
}

static int bxxt_boot_img_compat_check(bxxt_boot_t* b) {
        if (force_no_compat_check)
                return BXXT_SUCCESS;

        void* sha1sum = bxxt_boot_img_signature(b);
        if (*(uint64_t*)sha1sum == *(uint64_t*)&(b->hdr).id)
                return BXXT_SUCCESS;

        log1(1, "oops, signature check failed, more detail"
                        " please search `SIGNATURE` in README")
        // lineageos update_sha1(args.dt) # DT NOT EXIST IN AOSP
        return BXXT_FAILED; //signature check failed
}

static int bxxt_boot_img_base_check(bxxt_boot_t* b) {
#define length_check(field, cond) {\
        if (b->hdr.field cond) {\
                log1(-1, #field " should not " #cond) \
                return BXXT_FAILED; \
        }\
}
        length_check(page_size, == 0);
        length_check(kernel_size, == 0);

        length_check(ramdisk_size, > 0x4000000);
        length_check(kernel_size, > 0x4000000);
        length_check(page_size, > 0xffff);

        return BXXT_SUCCESS;
#undef length_check
}

static int bxxt_boot_detect_kernel_compression(bxxt_boot_t* b) {
        switch (*(uint16_t*)((void*)b->mm_kernel_ptr)) {
        case 0x8b1f:
        case 0x9e1f:
                return KERNEL_COMPRESSION_GZIP;
        case 0x005d:
                return KERNEL_COMPRESSION_LZMA;
        default:
                return KERNEL_COMPRESSION_NONE;
        }
}

static int bxxt_boot_write_to(void* ptr, size_t size, char* file) {
        int r;
        bxxt_file_t *df;

        df = bxxt_file_new(file, O_TRUNC|O_WRONLY|O_CREAT, 0644);

        if (bxxt_file_open(df) != 0) {
        log1(1, "failed open file %s to write", file);
                r = BXXT_FAILED;
                goto error;
        }
        r = write(df->fd, ptr, size);

        if (r == size)
                r = BXXT_SUCCESS;
        error:
        bxxt_file_close(df);
        return r;
}

static bxxt_buffer_t* bxxt_boot_read_from(char* file) {
        bxxt_file_t *sf;
        bxxt_buffer_t *buffer;

        buffer = bxxt_buffer_create(0);
        sf = bxxt_file_new_mm(file, O_RDONLY, 0644,
                                MAP_PRIVATE, PROT_READ);
        if (bxxt_file_open(sf) != 0)
                goto error;

        bxxt_buffer_concat_from_data(buffer,
                                sf->ptr, sf->size);
        error:
        bxxt_file_close(sf);
        return buffer;
}

static int bxxt_boot_decompress_to_file(void* ptr, size_t size,
        int compression, char* file) {
        struct archive *a;
        struct archive_entry *e;
        bxxt_file_t *df;
        int r;

        df = bxxt_file_new(file, O_WRONLY|O_CREAT, 0644);

        if (bxxt_file_open(df) != 0) {
        log1(1, "failed open file %s to write(dec)", file);
                r = BXXT_FAILED;
                goto error;
        }

        a = archive_read_new();
        archive_read_support_format_raw(a);

        switch (compression) {
        case KERNEL_COMPRESSION_GZIP:
                archive_read_support_filter_gzip(a);
                break;
        case KERNEL_COMPRESSION_LZMA:
                archive_read_support_filter_lzma(a);
                break;
        default: // case KERNEL_COMPRESSION_NONE:
        archive_read_support_filter_none(a);
                break;
        }

        archive_read_open_memory(a, ptr, size);

        archive_read_next_header(a, &e);
        r = archive_read_data_into_fd(a, df->fd);

        archive_read_close(a);
        archive_read_free(a);

        error:
        bxxt_file_close(df);
        return r;
}

static bxxt_buffer_t* bxxt_boot_compress_from_file(char* file, int compression) {
        struct archive *a;
        struct archive_entry *e;

        bxxt_buffer_t *buffer = NULL;
        bxxt_file_t *sf;
        size_t used;

        sf = bxxt_file_new_mm(file, O_RDONLY, 0644,
                                MAP_PRIVATE, PROT_READ);
        if (bxxt_file_open(sf) != 0)
                goto error;

        buffer = bxxt_buffer_create(sf->size);

        a = archive_write_new();
        archive_write_set_format_raw(a);

        switch (compression) {
        case KERNEL_COMPRESSION_GZIP:
                archive_write_add_filter_gzip(a);
                break;
        case KERNEL_COMPRESSION_LZMA:
                archive_write_add_filter_lzma(a);
                break;
        default: // case KERNEL_COMPRESSION_NONE:
                archive_write_add_filter_none(a);
                break;
        }

        archive_write_set_options(a, "gzip:compression-level=9");
        archive_write_set_options(a, "gzip:!timestamp");

        archive_write_open_memory(a, buffer->ptr, buffer->size, &used);

        e = archive_entry_new();
        //archive_entry_set_pathname(e, "");
        archive_entry_set_filetype(e, AE_IFREG);
        archive_entry_set_size(e, sf->size);

        archive_write_header(a, e);
        archive_entry_free(e);

        archive_write_data(a, sf->ptr, sf->size);

        archive_write_close(a);
        archive_write_free(a);

        bxxt_buffer_resize(buffer, (int)used - (int)buffer->size);
        log1(8, "compress %s, orig: %zx, compressed: %zx",
                        file, sf->size, buffer->size);
        error:
        bxxt_file_close(sf);
        return buffer;
}

static int bxxt_boot_parse_dtb_sections(bxxt_dtb_t dtb[], uint32_t* count,
                                void* mm, size_t total) {
        *count = 0;
        uint32_t magic = cpu_to_fdt32(FDT_MAGIC);
        void *base = mm;

        uint32_t size, version;
        void *knldtb_ptr;

        while ((size = (mm + total - base)) > 0
                && (knldtb_ptr = memmem(base, size, &magic, sizeof(magic)))) {
        size = fdt32_to_cpu(((struct fdt_header*)knldtb_ptr)->totalsize);
        version = fdt32_to_cpu(((struct fdt_header*)knldtb_ptr)->version);
        log1(8, "found device tree #%02x ver %x, size %x",
                                *count, version, size);
        dtb[*count].ptr = knldtb_ptr;
        dtb[*count].size = size;

        base = knldtb_ptr + size;
        (*count)++;
        }

        return *count;
}

static int bxxt_boot_init1(bxxt_boot_t* b, void* ptr, size_t size,
                        int check) {
        memcpy(&b->hdr, ptr, sizeof(b->hdr));
        if (bxxt_boot_img_version_check(b) == BXXT_FAILED)
                goto error;
        b->mm                   = ptr;

        b->page_size            = b->hdr.page_size;
        b->version              = b->hdr.header_version;

        log1(2, "image version is %02x, max support %02x",
                                b->version, BOOT_HDR_VERSION_MAX);

        b->kernel_size          = b->hdr.kernel_size;
        b->ramdisk_size         = b->hdr.ramdisk_size;
        b->second_size          = b->hdr.second_size;
        b->recovery_dtbo_size   = b->hdr.recovery_dtbo_size;
        b->dtb_size             = b->hdr.dtb_size;
#define B_ps(ps, bs) (((bs + ps - 1)/ps)*ps)

#define B_HD_ps (b->page_size)
#define B_KE_ps (B_ps(b->page_size, b->kernel_size))
#define B_RA_ps (B_ps(b->page_size, b->ramdisk_size))
#define B_SE_ps (B_ps(b->page_size, b->second_size))
#define B_RE_ps (B_ps(b->page_size, b->recovery_dtbo_size))
#define B_DT_ps (B_ps(b->page_size, b->dtb_size))

#define B_PAD_o (B_HD_ps + B_KE_ps + B_RA_ps + B_SE_ps + B_RE_ps + B_DT_ps)
#define B_DT_o (B_HD_ps + B_KE_ps + B_RA_ps + B_SE_ps + B_RE_ps)
#define B_RE_o (B_HD_ps + B_KE_ps + B_RA_ps + B_SE_ps)
#define B_SE_o (B_HD_ps + B_KE_ps + B_RA_ps)
#define B_RA_o (B_HD_ps + B_KE_ps)
#define B_KE_o (B_HD_ps)

        b->mm_kernel_ptr        = ptr + B_KE_o;
        int kernel_compression  = bxxt_boot_detect_kernel_compression(b);
        b->kernel_compression   = kernel_compression;

        log1(4, "kernel compression %02x",
                                kernel_compression);

        b->mm_ramdisk_ptr       = ptr + B_RA_o;

        b->mm_second_ptr        = ptr + B_SE_o;
        b->mm_recovery_dtbo_ptr = ptr + B_RE_o;
        b->mm_dtb_ptr           = ptr + B_DT_o;

        b->pad_size             = size - B_PAD_o;
        b->mm_pad_ptr           = ptr + B_PAD_o;

        int32_t ts = b->pad_size -1;
        while (ts && ((uint8_t*)b->mm_pad_ptr)[ts] == 0x0) ts -=1;
        if (b->pad_size != 0)
                b->pad_size = ts;

        log1(8,
                "trim (without padding zero bytes) unknown data "\
                "to size %x", b->pad_size);
        if (b->pad_size > 0)
                log1(-2, "NOTICE: parse complete, image size %zx "
                                "but still %x bytes data left",
                                        size, b->pad_size)
        if (b->pad_size > 0)
                log1(-2, "this may produce an image that exceeds the size of your \n"
                         "         actual boot partition.\n"
                         "         if you sure the extra data is useless,\n"
                         "         add extra option `-e skip-unknown-data`\n"
                         "         when you re-pack the image")

        if (bxxt_boot_img_base_check(b) == BXXT_FAILED)
                goto error;

        if (check && bxxt_boot_img_compat_check(b) == BXXT_FAILED)
                goto error;

        log1(2, "os version %s", bxxt_boot_img_get_osver_string(b));
        log1(2, "detecting device tree");

        bxxt_boot_parse_dtb_sections(&b->kernel_dtb, &b->kernel_dtb_count,
                                      b->mm_kernel_ptr, b->kernel_size);
        log1(4, "%02x kernel device tree found",
                                b->kernel_dtb_count);

        bxxt_boot_parse_dtb_sections(&b->dtb, &b->dtb_count,
                                      b->mm_dtb_ptr, b->dtb_size);
        log1(4, "%02x device tree found",
                                b->dtb_count);
        return BXXT_SUCCESS;
        error:
        return BXXT_FAILED;
}

static int bxxt_boot_open_file(bxxt_boot_t* b, char* file) {
        int r;
        log1(4, "openning %s", file);
        b->image = bxxt_file_new_mm(file, O_RDONLY, 0644, MAP_PRIVATE,
                                        PROT_READ);

        if (bxxt_file_open(b->image) != 0)
                goto error;

        r = bxxt_boot_init1(b, b->image->ptr, b->image->size,
                                force_no_compat_check);
        if (r != BXXT_SUCCESS)
                goto error;

        return BXXT_SUCCESS;

        error:
        log1(-1, "open failed");

        bxxt_file_close(b->image);
        b->image = NULL;
        return BXXT_FAILED;
}

static uint8_t* bxxt_boot_img_signature(bxxt_boot_t* b) {
        SHA1_CTX ctx;
        static uint8_t sha[32];
        memset(sha, 0x0, sizeof(sha));

        SHA1Init(&ctx);
#define UINT32SIZE sizeof(uint32_t)

        SHA1Update(&ctx, b->mm_kernel_ptr, b->kernel_size);
        SHA1Update(&ctx, &b->kernel_size, UINT32SIZE);

        SHA1Update(&ctx, b->mm_ramdisk_ptr, b->ramdisk_size);
        SHA1Update(&ctx, &b->ramdisk_size, UINT32SIZE);

        if (b->second_size > 0)
                SHA1Update(&ctx, b->mm_second_ptr, b->second_size);
        SHA1Update(&ctx, &b->second_size, UINT32SIZE);

        if (b->version > 0)
        {
        if (b->recovery_dtbo_size > 0)
                SHA1Update(&ctx, b->mm_recovery_dtbo_ptr, b->recovery_dtbo_size);
        SHA1Update(&ctx, &b->recovery_dtbo_size, UINT32SIZE);
        }

        if (b->version > 1)
        {
        if (b->dtb_size > 0)
                SHA1Update(&ctx, b->mm_dtb_ptr, b->dtb_size);
        SHA1Update(&ctx, &b->dtb_size, UINT32SIZE);
        }
	SHA1Final(sha, &ctx);

#undef UINT32SIZE
        return sha;
}

void bxxt_boot_free(bxxt_boot_t* b) {
        if (!b)
                return;
        bxxt_file_close(b->image);
        bxxt_free(b);
}

static int bxxt_boot_decompile_dt2fs(bxxt_dtb_t dtb[], uint32_t count,
        char* abspath, char* prefix) {
        int ctr;
        char path[PATH_MAX];

        for (ctr = 0; ctr < count; ctr++) {
        sprintf(path, "%s/%s%02x", abspath, prefix, ctr);
        log1(8, "decompile device tree %s", path);
        bxxt_dtcc_dtb_to_source(dtb[ctr].ptr,
                                dtb[ctr].size, path);
        }

        return count - ctr;
}

int bxxt_boot_unpack_all(char* name, char* abspath) {
        int r = BXXT_FAILED;
        char path[PATH_MAX];

        bxxt_boot_t* b = NULL;

        sprintf(path, "%s/METADATA", abspath);
        FILE* meta = fopen(path, "wb");
        if (!meta) {
                log1(-1, "output path %s not exist or writable", abspath);
        goto error;
        }


        b = bxxt_boot_new();
        r = bxxt_boot_open_file(b, name);
        if (!meta || r != BXXT_SUCCESS)
                goto error;

        log1(4, "unpacking to %s", abspath);

        sprintf(path, "%s/kernel", abspath);
        bxxt_boot_decompress_to_file(b->mm_kernel_ptr, b->kernel_size,
                                     b->kernel_compression, path);

        sprintf(path, "%s/recovery_dtbo", abspath);
        if (b->recovery_dtbo_size)
                bxxt_boot_write_to(b->mm_recovery_dtbo_ptr,
                                b->recovery_dtbo_size, path);

        sprintf(path, "%s/second", abspath);
        if (b->second_size)
                bxxt_boot_write_to(b->mm_second_ptr, b->second_size, path);

        sprintf(path, "%s/extra.data", abspath);
        if (b->pad_size)
                bxxt_boot_write_to(b->mm_pad_ptr, b->pad_size, path);

        log1(4, "decompiling device tree");
        bxxt_boot_decompile_dt2fs(b->kernel_dtb, b->kernel_dtb_count,
                                abspath, "kernel.dts-");

        bxxt_boot_decompile_dt2fs(b->dtb, b->dtb_count,
                                abspath, "dt.dts-");

        log1(4, "extracting ramdisk");

        sprintf(path, "%s/ramdisk", abspath);
        bxxt_buffer_t* fs = bxxt_buffer_create_from_data(b->mm_ramdisk_ptr,
                                b->ramdisk_size);
        mkdir(path, 0755);
        if (b->ramdisk_size > 0)
                bxxt_cpio_archive_in(fs, path);
        bxxt_buffer_free(fs);

        log1(4, "writing METADATA");
        bxxt_boot_print_meta_info_file(b, meta);

        r = BXXT_SUCCESS;
        log1(1, "finished");

        error:
        if (meta)
                fclose(meta);
        bxxt_boot_free(b);
        return r;
}

static bxxt_buffer_t *
bxxt_boot_compile_fs2blob(char* abspath, char* prefix) {
        int ctr;
        char path[PATH_MAX];
        struct stat st;

        bxxt_buffer_t *dt, *blob;
        blob = bxxt_buffer_create(0);

        for (ctr = 0; ctr < 0xff; ctr++) {
        sprintf(path, "%s/%s%02x", abspath, prefix, ctr);
        if (stat(path, &st) != 0 || !S_ISREG(st.st_mode))
                break;
        log1(4, "compiling %s", path);
        dt = bxxt_dtcc_dtb_from_source(path);
        bxxt_buffer_concat_from_data(blob, dt->ptr, dt->size);
        bxxt_buffer_free(dt);
        }
        return blob;
}

int bxxt_boot_pack_all(char* abspath, char* name) {
        struct boot_img_hdr hdr;
        char path[PATH_MAX];
        struct stat st;

        bxxt_boot_t* boot;
        bxxt_buffer_t *img, *tmp, *dt;
        int r, page_size, size, compression;

        compression = KERNEL_COMPRESSION_GZIP;
        boot = img = NULL;

        sprintf(path, "%s/METADATA", abspath);

        log1(1, "restoring METADATA");
        r = bxxt_boot_hdr_from_metadata(&hdr, path);
        if (r != BXXT_SUCCESS)
                goto error;

        boot = bxxt_boot_new();
        r = bxxt_boot_scan_metadata(path,
                                "bxxt.kernel_compression",
                                "%x", &compression);

        log1(1, "generating %s", name);
        img = bxxt_buffer_create_from_data(&hdr, sizeof(hdr));

        page_size = ((struct boot_img_hdr*)img->ptr)->page_size;
        log1(1, "image pagesize is %x", page_size);

        bxxt_buffer_align(img, page_size);

#define pad_data(img, buffer, page_size, name) \
{ \
        log1(8, "appending " name ", size %zx (image %zx)", \
                                        buffer->size, img->size); \
        bxxt_buffer_concat_from_data(img, buffer->ptr, buffer->size); \
        bxxt_buffer_align(img, page_size); \
        log1(8, "image aligned to %zx, page_size %x", \
                                img->size, page_size); \
        bxxt_buffer_free(buffer); \
}
        sprintf(path, "%s/kernel", abspath);

        tmp = bxxt_boot_compress_from_file(path, compression);
        if (!tmp) {
                r = BXXT_FAILED;
                goto error;
        }

        log1(1, "compiling device tree (kernel)");
        dt = bxxt_boot_compile_fs2blob(abspath, "kernel.dts-");

        log1(8, "kernel size: %zx, dt size: %zx", tmp->size, dt->size);
        bxxt_buffer_concat_from_data(tmp, dt->ptr, dt->size);
        bxxt_buffer_free(dt);

        ((struct boot_img_hdr*)img->ptr)->kernel_size = tmp->size;
        pad_data(img, tmp, page_size, "kernel");

        sprintf(path, "%s/ramdisk", abspath);

        if (stat(path, &st) != 0 || !S_ISDIR(st.st_mode)) {
                r = BXXT_FAILED;
                log1(-1, "ramdisk root %s not exist", path);
                goto error;
        }

        log1(1, "packaging ramdisk, root path is %s", path);
        r = bxxt_cpio_archive_out(path, &tmp);
        if (r != BXXT_SUCCESS)
                goto error;

        // add ramdisk
        ((struct boot_img_hdr*)img->ptr)->ramdisk_size = tmp->size;
        pad_data(img, tmp, page_size, "ramdisk");

        // add second
        sprintf(path, "%s/second", abspath);

        if (stat(path, &st) == 0 && st.st_size > 0) {
        tmp = bxxt_boot_read_from(path);
        log1(8, "adding second, size %zx", tmp->size);

        ((struct boot_img_hdr*)img->ptr)->second_size = tmp->size;
        pad_data(img, tmp, page_size, "second");
        }

        // add recovery_dtbo
        sprintf(path, "%s/recovery_dtbo", abspath);

        if (stat(path, &st) == 0 && st.st_size > 0) {
        tmp = bxxt_boot_read_from(path);
        log1(8, "adding recovery_dtbo, size %zx", tmp->size);

        ((struct boot_img_hdr*)img->ptr)->recovery_dtbo_size = tmp->size;
        pad_data(img, tmp, page_size, "recovery_dtbo");
        }

        log1(1, "compiling device tree (dtb)");

        tmp = bxxt_boot_compile_fs2blob(abspath, "dt.dts-");
        ((struct boot_img_hdr*)img->ptr)->dtb_size = tmp->size;
        pad_data(img, tmp, page_size, "dtb");

        // add unknown padding data
        sprintf(path, "%s/extra.data", abspath);

        if (!skip_unknown_data
                && stat(path, &st) == 0 && st.st_size > 0) {
        tmp = bxxt_boot_read_from(path);
        log1(8, "adding unknown padding data, size %zx", tmp->size);

        pad_data(img, tmp, page_size, "unknow");
        }

        log1(2, "checking image");
        bxxt_boot_init1(boot, img->ptr, img->size, false);

        uint8_t* sha1 = bxxt_boot_img_signature(boot);
        memcpy(&boot->hdr.id, sha1, sizeof(boot->hdr.id));
        memcpy(&((struct boot_img_hdr*)boot->mm)->id, sha1,
                                sizeof(boot->hdr.id));

        //bxxt_boot_print_meta_info_file(boot, stderr);

        r = bxxt_boot_write_to(img->ptr, img->size, name);
        if (r != BXXT_SUCCESS)
                goto error;

        r = BXXT_SUCCESS;
        log1(2, "image %s generated, size %zx", name, img->size);
        log1(1, "finished");

        error:
        if (r != BXXT_SUCCESS)
                log1(-1, "failed (%d)", r)
        bxxt_buffer_free(img);
        bxxt_boot_free(boot);
        return r;
#undef pad_data
}

int boot_main(int argc, char **argv) {
        struct stat st;
        char i_path[PATH_MAX], o_path[PATH_MAX];

        for (int opt = 0; (opt = getopt(argc, argv, "i:o:e:n")) != -1;) {
        switch (opt) {
        case 'e':
                  skip_unknown_data = !strcmp(optarg, "skip-unknown-data");
                  break;
        case 'n': force_no_compat_check = true;
                  break;
        case 'i': strcpy(i_path, optarg);
                realpath(optarg, i_path); // bad bad but i want pretty log
                                          // without any ending slashes
                break;
        case 'o': strcpy(o_path, optarg);
                realpath(optarg, o_path);
                break;
        }
        }

        if (stat(i_path, &st) == 0 && S_ISDIR(st.st_mode))
                return bxxt_boot_pack_all(i_path, o_path);
        return bxxt_boot_unpack_all(i_path, o_path);
}
