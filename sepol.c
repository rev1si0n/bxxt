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
#include <getopt.h>

#include "sepol.h"
#include "util.h"


static bxxt_sepoldb_t* bxxt_sepol_new() {
        bxxt_sepoldb_t *bpb;

        bpb = (bxxt_sepoldb_t*)bxxt_malloc(sizeof(bxxt_sepoldb_t));
        bpb->mm_pool = bxxt_buffer_create(0);
        policydb_init(&bpb->pdb);
        return bpb;
}

static void bxxt_sepol_free(bxxt_sepoldb_t* bpb) {
        policydb_destroy(&bpb->pdb);
        bxxt_buffer_free(bpb->mm_pool);
        bxxt_free(bpb);
}

static int bxxt_sepol_load_policy_file(bxxt_sepoldb_t *bpb, char* file) {
        bxxt_file_t *sepol;
        sepol = bxxt_file_new_mm(file, O_RDONLY, 0644,
                        MAP_PRIVATE, PROT_READ|PROT_WRITE);
        int r;

        r = bxxt_file_open(sepol);
        if (r != 0) {
                r = BXXT_FAILED;
                goto error;
        }

        policydb_t* pdb = &bpb->pdb;
        r = policydb_from_image(NULL, sepol->ptr, sepol->size, pdb);

        error:
        bxxt_file_close(sepol);

        if (r != STATUS_SUCCESS)
                return BXXT_FAILED;
        return BXXT_SUCCESS;
}

static int bxxt_sepol_save_policy_file(bxxt_sepoldb_t* bpb, char* file) {
        bxxt_file_t *sepol;

        size_t datalen;
        void* data;

        int r;
        policydb_t* pdb = &bpb->pdb;

        r = policydb_to_image(NULL, pdb, &data, &datalen);
        if (r != STATUS_SUCCESS)
                return BXXT_FAILED;

        sepol = bxxt_file_new(file, O_RDWR|O_CREAT, 0644);
        if (bxxt_file_open(sepol) != 0) {
                r = BXXT_FAILED;
                goto error;
        }

        r = datalen;

        write(sepol->fd, data, datalen);
        free(data);

        error:
        bxxt_file_close(sepol);
        return r;
}

static int bxxt_sepol_reload_kernel_policy(bxxt_sepoldb_t* bpb) {
        int r = BXXT_SUCCESS;

        void* data = NULL;
        size_t datalen = 0;

        bxxt_file_t *sepol;
        policydb_t* pdb = &bpb->pdb;
        policydb_to_image(NULL, pdb, &data, &datalen);

        sepol = bxxt_file_new("/sys/fs/selinux/load", O_WRONLY, 0600);
        if (bxxt_file_open(sepol) != 0)
                goto error;

        if (write(sepol->fd, data, datalen) == datalen)
                goto success;

        error:
        r = BXXT_FAILED;

        success:
        bxxt_file_close(sepol);
        free(data);
        return r;
}

static void bxxt_sepol_reload_kernel_policy_from_file(char* file) {
        bxxt_sepoldb_t* bpb = bxxt_sepol_new();

        bxxt_sepol_load_policy_file(bpb, file);
        bxxt_sepol_reload_kernel_policy(bpb);
        bxxt_sepol_free(bpb);
}

static int bxxt_sepol_allow_ext(bxxt_sepoldb_t* bpb, char* src, char* tgt,
                                 char* cls, char* perm, bool disallow) {
        type_datum_t *s, *t;
        class_datum_t *c;

        perm_datum_t *p;

        avtab_datum_t *a;
        avtab_key_t k;

        int r;

        uint32_t mask = ~0x0;
        policydb_t* pdb = &bpb->pdb;

        if (!(c = hashtab_search(pdb->p_classes.table, cls)))
                return BXXT_FAILED;

        if (!(s = hashtab_search(pdb->p_types.table, src)))
                return BXXT_FAILED;

        if (!(t = hashtab_search(pdb->p_types.table, tgt)))
                return BXXT_FAILED;

        if ((uint8_t)'*' != *(uint8_t*)perm) {
        p = hashtab_search(c->permissions.table, perm);
        if (p == NULL && c->comdatum != NULL) {
                p = hashtab_search(c->comdatum->permissions.table, perm);
        }
        if (!p)
                return BXXT_FAILED;
        mask = (1U << (p->s.value - 1));
        }

        memset(&k, 0x0, sizeof(avtab_key_t));

        k.source_type           = s->s.value;
        k.target_type           = t->s.value;
        k.target_class          = c->s.value;
        k.specified             = AVTAB_ALLOWED;

        a = avtab_search(&pdb->te_avtab, &k);
        if (a == NULL) {
        a = malloc(sizeof(avtab_datum_t));
        r = avtab_insert(&pdb->te_avtab, &k, a);
        free(a);
        if (r != 0)
                return BXXT_FAILED;
        a = avtab_search(&pdb->te_avtab, &k);
        }

        if (disallow)
                a->data &= ~mask;
        else
                a->data |=  mask;

        return BXXT_SUCCESS;
}

static int bxxt_sepol_create_domain(bxxt_sepoldb_t* bpb, char* domain) {
        policydb_t* pdb = &bpb->pdb;

        symtab_datum_t *sd = hashtab_search(pdb->p_types.table, domain);
        if (sd)
                return sd->value;

        type_datum_t *attr;

        log1(2, "domain '%s' not exist, create now", domain)
        type_datum_t *td = (type_datum_t*)malloc(sizeof(type_datum_t));
        type_datum_init(td);

        td->primary             = 1;
        td->flavor              = TYPE_TYPE;

        uint32_t value = 0;
        char *type = strdup(domain);
        int r;

        r = symtab_insert(pdb, SYM_TYPES, type, td, SCOPE_DECL, 1, &value);
        if (r)
                return BXXT_FAILED;

        td->s.value     = value;

        r = ebitmap_set_bit(&pdb->global->branch_list->declared.scope[SYM_TYPES], value - 1, 1);
        if (r)
                return BXXT_FAILED;

        pdb->type_attr_map = realloc(pdb->type_attr_map, sizeof(ebitmap_t)*pdb->p_types.nprim);
        pdb->attr_type_map = realloc(pdb->attr_type_map, sizeof(ebitmap_t)*pdb->p_types.nprim);

        ebitmap_init(&pdb->type_attr_map[value -1]);
        ebitmap_init(&pdb->attr_type_map[value -1]);
        ebitmap_set_bit(&pdb->type_attr_map[value -1], value -1, 1);

        for (uint32_t i = 0; i < pdb->p_roles.nprim; ++i) {
        ebitmap_set_bit(&pdb->role_val_to_struct[i]->types.negset, value -1, 0);
        ebitmap_set_bit(&pdb->role_val_to_struct[i]->types.types, value -1, 1);
        }

        sd = hashtab_search(pdb->p_types.table, domain);
        if (!sd)
                return BXXT_FAILED;

        extern int policydb_index_decls(sepol_handle_t*, policydb_t*);

        policydb_index_decls(NULL, pdb);
        policydb_index_classes(pdb);

        policydb_index_others(NULL, pdb, 0);

        attr = hashtab_search(pdb->p_types.table, domain);
        if (!attr)
                return BXXT_FAILED;

        if (attr->flavor != TYPE_ATTRIB)
                return value;
        if(ebitmap_set_bit(&pdb->type_attr_map[value -1], attr->s.value -1, 1))
                return value;
        if(ebitmap_set_bit(&pdb->attr_type_map[attr->s.value -1], value -1, 1))
                return value;
        return value;
}

static int bxxt_sepol_set_permissive(bxxt_sepoldb_t* bpb, char* domain, bool permissive) {
        policydb_t* pdb = &bpb->pdb;

        symtab_datum_t *sd = hashtab_search(pdb->p_types.table, domain);
        return ebitmap_set_bit(&pdb->permissive_map, sd?sd->value: bxxt_sepol_create_domain(bpb, domain),
                               (int)permissive);
}

static int bxxt_sepol_execute_stmt(bxxt_sepoldb_t* bpb, char* stmt) {
        int r = BXXT_SUCCESS;
        log1(8, "execute sepol stmt: %s", stmt)
        char src[256] = {0}, tgt[256] = {0}, cls[256] = {0}, perm[256] = {0};
        if (sscanf(stmt, "allow %[1-9,a-z,_,-] %[1-9,a-z,_,-]:%[1-9,a-z,_,-] %[1-9,a-z,_,-,*];",
                                                                    src, tgt, cls, perm) == 4)
                r = bxxt_sepol_allow_ext(bpb, src, tgt, cls, perm, false);
        else if (sscanf(stmt, "disallow %[1-9,a-z,_,-] %[1-9,a-z,_,-]:%[1-9,a-z,_,-] %[1-9,a-z,_,-,*];",
                                                                    src, tgt, cls, perm) == 4)
                r = bxxt_sepol_allow_ext(bpb, src, tgt, cls, perm, true);
        else if (sscanf(stmt, "permissive %[1-9,a-z,_,-];", src) == 1)
                r = bxxt_sepol_set_permissive(bpb, src, true);
        else if (sscanf(stmt, "enforce %[1-9,a-z,_,-];", src) == 1)
                r = bxxt_sepol_set_permissive(bpb, src, false);
        else if (sscanf(stmt, "create %[1-9,a-z,_,-];", src) == 1)
                //bxxt_sepol_create_domain returns an id not status
                r = bxxt_sepol_create_domain(bpb, src) < 0;
        else
                return -1;
        return r;
}

int sepol_main(int argc, char **argv) {
        bxxt_sepoldb_t* bpb = bxxt_sepol_new();

        bool reload = false;
        char input[PATH_MAX] = {0}, output[PATH_MAX] = {0};
        char stmt[4096] = {0};

        strcpy(input, "/sys/fs/selinux/policy");
        for (int opt = 0; (opt = getopt(argc, argv, "s:i:o:l")) != -1;) {
        switch (opt) {
        case 'i': strcpy(input, optarg);
                  break;
        case 'o': strcpy(output, optarg);
                  break;
        case 's': strcpy(stmt, optarg);
                  break;
        case 'l': reload = true;
                  break;
        }
        }

        bxxt_sepol_load_policy_file(bpb, input);
        if (bxxt_sepol_execute_stmt(bpb, stmt) != 0)
                log1(-1, "failed execute %s", stmt);
        if (strlen(output))
                bxxt_sepol_save_policy_file(bpb, output);
        if (reload)
                bxxt_sepol_reload_kernel_policy(bpb);
        bxxt_sepol_free(bpb);
        return 0;
}

int sepol_rtexec(char* stmt) {
        int r = BXXT_FAILED;
        bxxt_sepoldb_t* bpb = bxxt_sepol_new();

        bxxt_sepol_load_policy_file(bpb, "/sys/fs/selinux/policy");
        if (bxxt_sepol_execute_stmt(bpb, stmt) != 0)
                goto error;

        bxxt_sepol_reload_kernel_policy(bpb);
        r = BXXT_SUCCESS;

        error:
        bxxt_sepol_free(bpb);
        return r;
}