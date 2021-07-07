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
#ifndef _BXXT_SEPOL_H_
#define _BXXT_SEPOL_H_

#include "bxxt.h"

#ifdef DEBUG
#include <debug.h>
#include <sepol/debug.h>
#endif
#include <sepol/policydb/link.h>
#include <sepol/policydb/expand.h>
#include <sepol/policydb/policydb.h>
#include <sepol/policydb/services.h>
#include <sepol/policydb/avrule_block.h>
#include <sepol/policydb/conditional.h>

#ifndef _SEPOL_INTERNAL_DEBUG_H_

#define STATUS_SUCCESS 0
#define STATUS_NODATA 1
#define STATUS_ERR -1

#endif

#include "util.h"

typedef struct bxxt_sepoldb {
        // i don't know how to free memory allocated by create domain
        // pdb destory seems not release them,
        // so we allocate it ourself with a block memory
        bxxt_buffer_t *mm_pool;

        policydb_t pdb;
} bxxt_sepoldb_t;

static int bxxt_sepol_save_policy_file(bxxt_sepoldb_t* bpb, char* file);
static int bxxt_sepol_load_policy_file(bxxt_sepoldb_t* bpb, char* file);

static int bxxt_sepol_reload_kernel_policy(bxxt_sepoldb_t* bpb);
static void bxxt_sepol_reload_kernel_policy_from_file(char* file);

static int bxxt_sepol_allow_ext(bxxt_sepoldb_t* bpb, char* src, char* tgt,
                                 char* cls, char* perm, bool disallow);
static int bxxt_sepol_create_domain(bxxt_sepoldb_t* bpb, char* domain);

static int bxxt_sepol_execute_stmt(bxxt_sepoldb_t* bpb, char* stmt);
static int bxxt_sepol_set_permissive(bxxt_sepoldb_t* bpb, char* domain, bool permissive);

static bxxt_sepoldb_t* bxxt_sepol_new();
static void bxxt_sepol_free(bxxt_sepoldb_t* bpb);

int sepol_main(int argc, char **argv);
int sepol_rtexec(char* stmt);
#endif
