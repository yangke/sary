/* 
 * sary - a suffix array library
 *
 * $Id: builder.c,v 1.2 2005/01/28 08:50:24 knok Exp $
 *
 * Copyright (C) 2000  Satoru Takabayashi <satoru@namazu.org>
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sary.h>

typedef void	(*SortFunc)	(SaryBuilder *builder,
				 SaryMmap *array, 
				 SaryText *text,
				 SaryInt len);

struct _SaryBuilder{
    SaryText		*text;
    gchar		*array_name;
    SaryIpointFunc	ipoint_func;
    SaryInt		block_size;
    SaryInt		nthreads;
    SaryProgressFunc	progress_func;
    gpointer		progress_func_data;
};

static SaryInt	index		(SaryBuilder	*builder, 
				 SaryProgress	*progress,
				 SaryWriter	*writer);
static void	progress_quiet	(SaryProgress	*progress);

SaryBuilder *
sary_builder_new (const gchar *file_name)
{
    SaryBuilder *builder;
    gchar *array_name;

    g_assert(file_name != NULL);
    array_name = g_strconcat(file_name, ".ary", NULL);

    builder = sary_builder_new2(file_name, array_name);
    g_free(array_name);
    return builder;
}


SaryBuilder *
sary_builder_new2 (const gchar *file_name, const gchar *array_name)
{
    SaryBuilder *builder;

    g_assert(file_name != NULL && array_name != NULL);

    builder = g_new(SaryBuilder, 1);
    builder->text = sary_text_new(file_name);
    if (builder->text == NULL) {
	return NULL;
    }

    builder->array_name	   = g_strdup(array_name);
    builder->ipoint_func   = sary_ipoint_bytestream;
    builder->block_size    = 1024 * 1024 / sizeof(SaryInt); /* 1 MB */
    builder->nthreads      = 1;
    builder->progress_func = progress_quiet;

    return builder;
}

void
sary_builder_destroy (SaryBuilder *builder)
{
    sary_text_destroy(builder->text);
    g_free(builder->array_name);
    g_free(builder);
}

void
sary_builder_set_ipoint_func (SaryBuilder *builder,
			      SaryIpointFunc ipoint_func)
{
    g_assert(ipoint_func != NULL);

    builder->ipoint_func = ipoint_func;
}

SaryInt
sary_builder_index (SaryBuilder *builder)
{
    SaryInt count;
    SaryInt file_size;
    SaryProgress *progress;
    SaryWriter *writer;

    writer = sary_writer_new(builder->array_name);
    if (writer == NULL) {
	return -1;
    }

    file_size  = sary_text_get_size(builder->text);

    progress = sary_progress_new("index", file_size);
    sary_progress_connect(progress,
			  builder->progress_func, 
			  builder->progress_func_data);

    count = index(builder, progress, writer);

    sary_progress_destroy(progress);
    sary_writer_destroy(writer);

    return count;
}

gboolean
sary_builder_sort (SaryBuilder *builder)
{
    SarySorter *sorter;
    gboolean result;

    sorter = sary_sorter_new(builder->text, builder->array_name);
    if (sorter == NULL)
      return FALSE;
    sary_sorter_connect_progress(sorter,
				 builder->progress_func,
				 builder->progress_func_data);
    result = sary_sorter_sort(sorter);
    sary_sorter_destroy(sorter);

    return result;
}

gboolean
sary_builder_block_sort (SaryBuilder *builder)
{
    gchar *tmp_name;
    SarySorter *sorter;
    gboolean result;

    /*
     * Rename the array file temporarily.  
     *
     * Note: tmpnam(3) and friends cannot be used because
     * they may create a pathname on a different filesystem.
     */
    tmp_name = g_strconcat(builder->array_name, ".tmp", NULL);
    if (rename(builder->array_name, tmp_name) == -1) {
	return FALSE;
    }

    sorter = sary_sorter_new(builder->text, tmp_name);
    sary_sorter_connect_progress(sorter,
				 builder->progress_func,
				 builder->progress_func_data);
    sary_sorter_set_nthreads(sorter, builder->nthreads);

    /*
     * Construct the temporary array file by block sorting
     * and the real array file is created by merging later.
     */
    result = sary_sorter_sort_blocks(sorter, builder->block_size);
    if (result == TRUE) {
	sary_sorter_merge_blocks(sorter, builder->array_name);
    }
    sary_sorter_destroy(sorter);

    /*
     * Remove the temporary array file.
     */
    unlink(tmp_name);
    g_free(tmp_name);

    return result;
}

void
sary_builder_set_block_size (SaryBuilder *builder, SaryInt block_size)
{
    g_assert(block_size > 0);
    builder->block_size = block_size / sizeof(SaryInt);
}

void
sary_builder_set_nthreads (SaryBuilder *builder, SaryInt nthreads)
{
    g_assert(nthreads > 0);
    builder->nthreads = nthreads;
}

void
sary_builder_connect_progress (SaryBuilder *builder,
			       SaryProgressFunc progress_func,
			       gpointer progress_func_data)
{
    g_assert(progress_func != NULL);

    builder->progress_func = progress_func;
    builder->progress_func_data = progress_func_data;
}

static SaryInt
index (SaryBuilder *builder, SaryProgress *progress, SaryWriter *writer)
{
    gchar *bof, *cursor;
    SaryInt count;

    bof   = sary_text_get_bof(builder->text);
    count = 0;
    while ((cursor = builder->ipoint_func(builder->text))) {
	SaryInt pos = cursor - bof;

	if (sary_writer_write(writer, GINT_TO_BE(pos)) == FALSE) {
	    return -1;
	}

	sary_progress_set_count(progress, pos);
	count++;
    }
    if (sary_writer_flush(writer) == FALSE) {
	return -1;
    }
    return count;
}

static void
progress_quiet (SaryProgress *progress)
{
    /* do nothing */
}

