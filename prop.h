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
#ifndef _BXXT_DTCC_H_
#define _BXXT_DTCC_H_
#include <inttypes.h>

#include <fcntl.h>
#include <unistd.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "bxxt.h"
#include "util.h"

struct _prinfo {
        unsigned volatile serial;
        char value[PROP_VALUE_MAX];
        char name[0];
};

struct bxxt_mapinfo {
        char file[PATH_MAX];
        uint64_t start;
        uint64_t end;
};

int bxxt_scan_maps_by_offset_callback(void* offset,
                                      struct bxxt_mapinfo* bm);
int bxxt_scan_maps_by_callback(pid_t pid, void* args,
                int (*callback)(void*, struct bxxt_mapinfo*),
                                struct bxxt_mapinfo *bm);
int bxxt_prop_set(char* name, char* value);
pid_t bxxt_get_pid_by_cmdline(char* name);
int bxxt_do_setdebuggable(char* value);
#endif
