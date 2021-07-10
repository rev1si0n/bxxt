# Copyright 2019 rev1si0n <ihaven0emmail@gmail.com>

LOCAL_PATH:= $(call my-dir)
libz_src_files := \
            adler32.c \
            compress.c \
            crc32.c \
            deflate.c \
            gzclose.c \
            gzlib.c \
            gzread.c \
            gzwrite.c \
            infback.c \
            inflate.c \
            inftrees.c \
            inffast.c \
            trees.c \
            uncompr.c \
            zutil.c


libdtc_src_files := \
            checks.c \
            data.c \
            flattree.c \
            fstree.c \
            livetree.c \
            srcpos.c \
            treesource.c \
            util.c \
            dtc-lexer.c \
            dtc-parser.c


libarchive_src_files := \
            archive_write_disk_windows.c \
            archive_write_set_format_7zip.c \
            archive_read_support_format_cpio.c \
            archive_write_add_filter_program.c \
            archive_read_open_filename.c \
            archive_write_add_filter_lrzip.c \
            archive_entry_copy_stat.c \
            archive_read_support_filter_lzop.c \
            archive_blake2sp_ref.c \
            archive_read_support_filter_all.c \
            archive_read_support_filter_xz.c \
            archive_disk_acl_linux.c \
            archive_read.c \
            archive_write_set_format_shar.c \
            archive_cryptor.c \
            archive_write_add_filter_grzip.c \
            archive_write_disk_set_standard_lookup.c \
            archive_read_add_passphrase.c \
            archive_read_support_format_zip.c \
            archive_write_set_format_mtree.c \
            archive_util.c \
            archive_check_magic.c \
            archive_entry_copy_bhfi.c \
            archive_read_support_format_by_code.c \
            archive_write_set_format_warc.c \
            archive_random.c \
            archive_read_data_into_fd.c \
            archive_write_set_format_cpio.c \
            archive_write_set_format_cpio_newc.c \
            archive_write_add_filter_lzop.c \
            archive_write_set_format_ar.c \
            archive_read_disk_set_standard_lookup.c \
            archive_read_extract.c \
            archive_write_add_filter_compress.c \
            archive_read_open_file.c \
            archive_version_details.c \
            archive_options.c \
            archive_read_support_filter_compress.c \
            archive_read_append_filter.c \
            archive_match.c \
            archive_read_disk_posix.c \
            archive_entry.c \
            archive_blake2s_ref.c \
            archive_string_sprintf.c \
            archive_write_set_format_pax.c \
            filter_fork_windows.c \
            archive_write_open_memory.c \
            archive_write_set_format_gnutar.c \
            archive_read_support_filter_gzip.c \
            archive_entry_xattr.c \
            archive_read_support_filter_bzip2.c \
            archive_read_support_filter_none.c \
            archive_read_support_filter_lz4.c \
            archive_read_extract2.c \
            archive_read_disk_windows.c \
            archive_write_set_format_by_name.c \
            archive_write_add_filter_lz4.c \
            archive_write_set_format_v7tar.c \
            archive_virtual.c \
            archive_acl.c \
            archive_read_disk_entry_from_file.c \
            archive_write_set_format_xar.c \
            archive_read_support_format_mtree.c \
            archive_entry_sparse.c \
            archive_write_set_format_filter_by_ext.c \
            archive_windows.c \
            archive_disk_acl_darwin.c \
            archive_read_support_filter_rpm.c \
            archive_read_support_filter_lrzip.c \
            archive_cmdline.c \
            archive_write_set_format.c \
            filter_fork_posix.c \
            archive_write_disk_posix.c \
            archive_write_set_format_raw.c \
            archive_getdate.c \
            archive_hmac.c \
            archive_write_add_filter_by_name.c \
            archive_pack_dev.c \
            archive_write_add_filter_b64encode.c \
            archive_disk_acl_freebsd.c \
            archive_write_set_format_ustar.c \
            archive_entry_stat.c \
            archive_read_support_format_warc.c \
            archive_read_support_format_rar5.c \
            archive_read_support_format_iso9660.c \
            archive_ppmd8.c \
            archive_ppmd7.c \
            archive_write_add_filter.c \
            archive_write_open_filename.c \
            archive_read_open_fd.c \
            archive_entry_link_resolver.c \
            archive_write_add_filter_zstd.c \
            archive_read_support_format_raw.c \
            archive_read_support_format_rar.c \
            archive_read_support_format_7zip.c \
            archive_write_add_filter_uuencode.c \
            archive_write_add_filter_xz.c \
            archive_read_support_format_ar.c \
            archive_read_support_filter_zstd.c \
            archive_digest.c \
            archive_pathmatch.c \
            archive_read_set_options.c \
            archive_read_set_format.c \
            archive_disk_acl_sunos.c \
            archive_read_support_filter_grzip.c \
            archive_string.c \
            archive_entry_strmode.c \
            archive_write_set_passphrase.c \
            archive_write_open_fd.c \
            archive_write_set_format_iso9660.c \
            archive_write_add_filter_bzip2.c \
            archive_read_support_format_lha.c \
            archive_read_support_format_cab.c \
            archive_rb.c \
            archive_read_support_format_all.c \
            archive_read_support_format_empty.c \
            archive_write_set_format_zip.c \
            archive_write_add_filter_none.c \
            archive_write_add_filter_gzip.c \
            archive_read_support_filter_uu.c \
            archive_read_open_memory.c \
            archive_read_support_filter_program.c \
            archive_read_support_format_tar.c \
            archive_write.c \
            archive_read_support_format_xar.c \
            archive_write_set_options.c \
            archive_write_open_file.c \
            xxhash.c


libsepol_src_files := \
            assertion.c \
            avrule_block.c \
            avtab.c \
            boolean_record.c \
            booleans.c \
            conditional.c \
            constraint.c \
            context.c \
            context_record.c \
            debug.c \
            ebitmap.c \
            expand.c \
            handle.c \
            hashtab.c \
            hierarchy.c \
            iface_record.c \
            interfaces.c \
            kernel_to_cil.c \
            kernel_to_common.c \
            kernel_to_conf.c \
            link.c \
            mls.c \
            module.c \
            module_to_cil.c \
            node_record.c \
            nodes.c \
            optimize.c \
            polcaps.c \
            policydb.c \
            policydb_convert.c \
            policydb_public.c \
            port_record.c \
            ports.c \
            roles.c \
            services.c \
            sidtab.c \
            symtab.c \
            user_record.c \
            users.c \
            util.c \
            write.c


bxxt_src_files := \
            boot.c \
            cpio.c \
            util.c \
            sepol.c \
            bxxt.c \
            patch.c \
            sha1.c \
            dtcc.c \
            prop.c


include $(CLEAR_VARS)
LOCAL_MODULE        := archive_static
LOCAL_SRC_FILES     := $(addprefix libarchive/,$(libarchive_src_files))
LOCAL_C_INCLUDES    := $(LOCAL_PATH)/libarchive/include/android $(LOCAL_PATH)/libarchive/include
LOCAL_CFLAGS        := -DHAVE_CONFIG_H -Wno-deprecated-declarations
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE            := z_static
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/zlib
LOCAL_SRC_FILES         := $(addprefix zlib/,$(libz_src_files))
LOCAL_CFLAGS            := -DUSE_MMAP
LOCAL_EXPORT_C_INCLUDES	:= $(LOCAL_C_INCLUDES)
include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE        := sepol_static
LOCAL_C_INCLUDES    := $(LOCAL_PATH)/libsepol $(LOCAL_PATH)/libsepol/include \
                       $(LOCAL_PATH)/libsepol/include/sepol
LOCAL_SRC_FILES     := $(addprefix libsepol/,$(libsepol_src_files))
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE        := dtc_static
LOCAL_SRC_FILES     := $(addprefix libdtc/,$(libdtc_src_files))
LOCAL_C_INCLUDES    := $(LOCAL_PATH)/libdtc
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE        := bxxt
LOCAL_C_INCLUDES    := $(LOCAL_PATH)
LOCAL_SRC_FILES     := $(bxxt_src_files)
# if you want disable all bxxt outputs just set BXXT_LOGLEVEL to -999
LOCAL_CFLAGS        := -DBXXT_LOGLEVEL=255 -Wno-incompatible-pointer-types -Wno-pointer-sign \
                       -Wno-implicit-function-declaration
LOCAL_STATIC_LIBRARIES  := archive_static sepol_static dtc_static z_static
LOCAL_LDFLAGS       := -static
include $(BUILD_EXECUTABLE)
