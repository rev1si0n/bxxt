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
#include <byteswap.h>

#include "bxxt.h"
#include "util.h"


int patch(uint64_t offset, uint64_t repl,
        uint64_t size, char* fname) {
        bxxt_file_t *file = NULL;
        char fmt[512] = {0}, msg[512];

        uint64_t n = 0;
        uint8_t *ptr;

        repl = bswap_64(repl);

        if (size > sizeof(uint64_t)) {
        log1(-1, "patch size must <= %x bytes", (int)sizeof(uint64_t));
                goto error;
        }

        file = bxxt_file_new(fname, O_WRONLY, 0644);
        if (bxxt_file_open(file) != 0) {
        log1(-1, "failed open file %s to patch", fname);
                goto error;
        }

        if (lseek(file->fd, offset, SEEK_SET) != offset) {
        log1(-1, "cant't seek to offset %" PRIx64, offset);
                goto error;
        }

        ptr = ((uint8_t*)&repl) + (sizeof(uint64_t) - size);
        memcpy(&n, ptr, size);

        n = bswap_64(n) >> ((sizeof(uint64_t) - size)*8);
        sprintf(fmt, "patch offset %%08" PRIx64 " to %%0%u" PRIx64,
                                                (uint8_t)size*2);
        sprintf(msg, fmt, offset, n);
        log1(1, "%s", msg)

        write(file->fd, ptr, (size_t)size);

        bxxt_file_close(file);
        return BXXT_SUCCESS;

        error:
        log1(-1, "failed");
        bxxt_file_close(file);
        return BXXT_FAILED;
}

int patch_main(int argc, char **argv) {
        uint64_t offset = 0, repl = 0, size = 0;
        if (argc == 3 && sscanf(argv[1], "@%" PRIX64 ":%" PRIX64 "=%" PRIX64,
                                         &offset, &size, &repl) == 3)
                return patch(offset, repl, size, argv[2]);
        return BXXT_FAILED;
}