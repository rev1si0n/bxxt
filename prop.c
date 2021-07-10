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
#include "prop.h"


int bxxt_scan_maps_by_offset_callback(void* offset,
                                      struct bxxt_mapinfo* bm) {
        return !(*(uint64_t*)offset >= bm->start \
                        && *(uint64_t*)offset <= bm->end);
}

int bxxt_scan_maps_by_callback(pid_t pid, void* args,
                int (*callback)(void*, struct bxxt_mapinfo*),
                                struct bxxt_mapinfo *bm) {
        int r = 0;
        static char name[PATH_MAX], *p_name;
        sprintf(name, "/proc/%d/maps", pid?pid: getpid());
        memset(bm, 0x0, sizeof (struct bxxt_mapinfo));

        FILE* m = fopen(name, "rb");
        if (m == NULL) {
                log1(-1, "failed open maps %s", name);
        return 0;
        }

        while (fscanf(m, "%" PRIx64 "-%" PRIx64 " %[^\n]s", &bm->start,
                                        &bm->end, name) == 3) {
        p_name = name + strlen(name);
        while (p_name > name && *(p_name-1) != ' ') p_name--;
        strcpy(bm->file, p_name);
        if (callback(args, bm) == 0)
                goto found;
        }
        r = 1;

        found:
        fclose(m);
        return r;
}

int bxxt_prop_set(char* name, char* value) {
        uint64_t offset, addr;
        struct bxxt_mapinfo bm;

        struct _prinfo *p = (struct _prinfo*)__system_property_find(name);
        addr = (uint64_t)p;

        int r = bxxt_scan_maps_by_callback(0, &addr,
                        bxxt_scan_maps_by_offset_callback, &bm);
        if (r != 0) {
                log1(-1, "%s not found", name);
        return 1;
        }
        offset = addr - bm.start;

        if (strlen(value) > PROP_VALUE_MAX) {
                log1(-1, "prop value %s too long", value)
        return 1;
        }

        int fd = open(bm.file, O_RDWR | O_EXCL, 0444);
        if (fd == -1) {
                log1(-1, "failed open dev %s", bm.file);
        return 1;
        }

        lseek64(fd, offset +((void*)(p->value) - (void*)p),
                                SEEK_SET);
        r = write(fd, value, strlen(value) + 1);
        close(fd);

        return !(r > 0);
}

pid_t bxxt_get_pid_by_cmdline(char* name) {
        DIR *d;
        struct dirent *de;
        char path[PATH_MAX], tmp[PATH_MAX];
        FILE *cmdline;

        pid_t pid = -1;

        d = opendir("/proc");
        if (!d) {
                log1(-1, "failed walk /proc");
        return -1;
        }

        while ((de = readdir(d))) {
        sprintf(path, "/proc/%s/cmdline", de->d_name);

        if (!(cmdline = fopen(path, "r")))
            continue;

        memset(tmp, 0x0, sizeof(tmp));
        fread(tmp, 1, sizeof(tmp), cmdline);

        fclose(cmdline);

        if (strcmp(name, tmp) == 0) {
                pid = atol(de->d_name);
                break;
        }
        }

        closedir(d);
        return pid;
}

int bxxt_do_setdebuggable(char* value) {
        if (bxxt_prop_set("ro.debuggable", value) != 0)
                return 1;

        pid_t pid = bxxt_get_pid_by_cmdline("system_server");
        if (pid <= 0) {
                log1(-1, "system_server not found")
        return 1;
        }

        // kill system server to let system `reboot`
        // that reboot is not a cold reboot, the kernel
        // is still there
        kill(pid, SIGTERM);
        return 0;
}


int setdebuggable_main(int argc, char **argv) {
        return bxxt_do_setdebuggable(argv[1]);
}

int setprop_main(int argc, char **argv) {
        return bxxt_prop_set(argv[1], argv[2]);
}
