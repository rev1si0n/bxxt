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
#include "util.h"
#include "boot.h"
#include "cpio.h"

int main(int argc, char **argv) {
        umask(022);
        if (argc > 2 && strcmp(argv[1], "boot") == 0) {
        log1(-256, "bxxt 1.0, Copyright rev1si0n <github.com/rev1si0n/bxxt>, 2019.")
        }

        if (argc > 2 && strcmp(argv[1], "boot") == 0)
                return boot_main(argc - 1, argv + 1);
        else if (argc > 2 && strcmp(argv[1], "sepol") == 0)
                return sepol_main(argc - 1, argv + 1);
        else if (argc == 3 && strcmp(argv[1], "setdebuggable") == 0)
                return setdebuggable_main(argc - 1, argv + 1);
        else if (argc == 4 && strcmp(argv[1], "setprop") == 0)
                return setprop_main(argc - 1, argv + 1);
        else if (argc > 2 && strcmp(argv[1], "patch") == 0)
                return patch_main(argc - 1, argv + 1);
        return 1;
}
