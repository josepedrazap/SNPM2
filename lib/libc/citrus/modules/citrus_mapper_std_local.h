/*	$NetBSD: citrus_mapper_std_local.h,v 1.3 2006/09/09 14:35:17 tnozaki Exp $	*/

/*-
 * Copyright (c)2003, 2006 Citrus Project,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _CITRUS_MAPPER_STD_LOCAL_H_
#define _CITRUS_MAPPER_STD_LOCAL_H_

typedef u_int32_t (*_citrus_mapper_std_getvalfunc_t)(const void *, u_int32_t);

struct _citrus_mapper_std_linear_zone {
	_citrus_index_t		begin;
	_citrus_index_t		end;
	_citrus_index_t		width;
};
struct _citrus_mapper_std_rowcol {
	struct _citrus_region	rc_table;
	size_t			rc_src_rowcol_len;
	struct _citrus_mapper_std_linear_zone
				*rc_src_rowcol;
	_citrus_index_t		rc_src_rowcol_bits;
	_citrus_index_t		rc_src_rowcol_mask;
	_citrus_index_t		rc_dst_invalid;
	_citrus_index_t		rc_dst_unit_bits;
	int			rc_oob_mode;
	_citrus_index_t		rc_dst_ilseq;
};

struct _citrus_mapper_std;

typedef int (*_citrus_mapper_std_convert_t)(
	struct _citrus_mapper_std *__restrict,
	_index_t *__restrict, _index_t, void *__restrict);
typedef void (*_citrus_mapper_std_uninit_t)(struct _citrus_mapper_std *);

struct _citrus_mapper_std {
	struct _citrus_region		ms_file;
	struct _citrus_db		*ms_db;
	_citrus_mapper_std_convert_t	ms_convert;
	_citrus_mapper_std_uninit_t	ms_uninit;
	union {
		struct _citrus_mapper_std_rowcol	rowcol;
	} u;
#define ms_rowcol	u.rowcol
};

#endif
