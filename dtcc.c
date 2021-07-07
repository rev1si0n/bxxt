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
#include <assert.h>
#include <dtc.h>

extern void* dt_to_mem(struct dt_info *dti, int version, uint32_t* size);
extern struct dt_info* dt_from_mem(void* ptr, size_t size);

#include "bxxt.h"
#include "util.h"
#include "dtcc.h"

int quiet 		= 0xff;
int reservenum 		= 0x00;
int annotate		= 0x00;

int phandle_format 	= PHANDLE_EPAPR;

int minsize 		= 0x00;
int padsize 		= 0x00;
int alignsize 		= 0x00;


int bxxt_dtcc_dtb_to_source(void* ptr, size_t size, char *fname) {
	FILE* out;
	bxxt_buffer_t* dtb;
	struct dt_info* dti;
	out = fopen(fname, "wb");

	if (out == NULL)
		return BXXT_FAILED;

	dtb = bxxt_buffer_create_from_data(ptr, size);

	dti = dt_from_mem(dtb->ptr, dtb->size);
 // see https://source.android.com/devices/architecture/dto/compile
	generate_label_tree(dti, "__symbols__", true);

        dt_to_source(out, dti);
	free(dti);

	bxxt_buffer_free(dtb);
	return BXXT_SUCCESS;
}

bxxt_buffer_t* bxxt_dtcc_dtb_from_source(char *fname) {
	struct dt_info* dti;
	bxxt_buffer_t* data;
	uint32_t size = 0;
	void* p;

	dti = dt_from_source(fname);
 // see https://source.android.com/devices/architecture/dto/compile
	generate_label_tree(dti, "__symbols__", true);

	p = dt_to_mem(dti, DEFAULT_FDT_VERSION, &size);
	data = bxxt_buffer_create_from_data(p, size);

	free(p);
	free(dti);

	return data;
}