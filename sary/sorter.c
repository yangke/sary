/* 
 * sary - a suffix array library
 *
 * $Id: sorter.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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
#include <sary.h>
#include <errno.h>
#include <pthread.h>

typedef struct {
    SaryInt *first;
    SaryInt len;
} Block;

typedef struct {
    Block*	blocks;
    Block*	first;
    Block*	cursor;
    Block*	last;
} Blocks;

struct _SarySorter {
    SaryMmap*		array;
    SaryText*		text;
    gchar*		array_name;
    SaryInt		nthreads;
    SaryInt		nipoints;
    Blocks*		blocks;
    SaryProgress*	progress;
    SaryProgressFunc	progress_func;
    gpointer		progress_func_data;
    pthread_mutex_t*	mutex;
};

static Blocks*	new_blocks	(SaryInt *array, 
				 SaryInt nipoints, 
				 SaryInt block_size, 
				 SaryInt nblocks);
static void	destroy_blocks	(Blocks *blocks);
static void	sort_block	(SarySorter *sorter);
static Block*	get_next_block	(SarySorter *sorter);

/*
 * `nipoints' stands for number of index points.
 * `nblocks'  stands for number of blocks.
 */
static SaryInt		calc_nblocks	(SaryInt nipoints,
					 SaryInt block_size);


SarySorter *
sary_sorter_new (SaryText *text, const gchar *array_name)
{
    SarySorter *sorter;

    sorter = g_new(SarySorter, 1);
    sorter->array = sary_mmap(array_name, "r+");
    if (sorter->array == NULL) {
	return FALSE;
    }

    sorter->text   = text;
    sorter->nipoints = sorter->array->len / sizeof(SaryInt);
    sorter->nthreads = 1;
    sorter->array_name = g_strdup(array_name);
    sorter->blocks   = NULL;
    sorter->progress = NULL;
    sorter->progress_func      = NULL;
    sorter->progress_func_data = NULL;

    return sorter;
}

void
sary_sorter_destroy (SarySorter *sorter)
{
    sary_munmap(sorter->array);
    destroy_blocks(sorter->blocks);
    g_free(sorter->array_name);
    g_free(sorter);
}

gboolean
sary_sorter_sort (SarySorter *sorter)

{
    sorter->progress = sary_progress_new("sort", sorter->nipoints);
    sary_progress_connect(sorter->progress, 
			  sorter->progress_func, 
			  sorter->progress_func_data);

    sary_multikey_qsort(sorter->progress,
			(SaryInt *)sorter->array->map, 
			sorter->nipoints, 
			0,
			sary_text_get_bof(sorter->text),
			sary_text_get_eof(sorter->text));

    sary_progress_destroy(sorter->progress);

    return TRUE;
}

gboolean
sary_sorter_sort_blocks (SarySorter *sorter,
			 SaryInt block_size)
{
    SaryInt i;
    SaryInt nblocks = calc_nblocks(sorter->nipoints, block_size);
    pthread_t *threads = g_new(pthread_t, sorter->nthreads);

    sorter->blocks  = new_blocks((SaryInt *)sorter->array->map,
				 sorter->nipoints, block_size, nblocks);
    sorter->mutex = g_new(pthread_mutex_t, 1);
    pthread_mutex_init(sorter->mutex, NULL);

    sorter->progress = sary_progress_new("sort", sorter->nipoints);
    sary_progress_connect(sorter->progress, 
			  sorter->progress_func, 
			  sorter->progress_func_data);

    for (i = 0; i < sorter->nthreads; i++) {
	if (pthread_create(&threads[i], NULL, 
			   (void *)sort_block, sorter) != 0) 
	{
	    g_error("pthread_create: %s", g_strerror(errno));
	}
    }

    for (i = 0; i < sorter->nthreads; i++) {
	pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(sorter->mutex);
    sary_progress_destroy(sorter->progress);
    g_free(threads);
    g_free(sorter->mutex);

    return TRUE;
}

void
sary_sorter_merge_blocks (SarySorter *sorter, 
			  const gchar *array_name)
{
    SaryInt i;
    Blocks *blocks  = sorter->blocks;
    SaryInt nblocks = blocks->last - blocks->first + 1;
    SaryMerger *merger= sary_merger_new(sorter->text, 
					array_name,
					nblocks);

    for (i = 0; i < nblocks; i++) {
	sary_merger_add_block(merger, 
			      blocks->blocks[i].first, 
			      blocks->blocks[i].len);
    }

    sary_merger_merge(merger, sorter->progress_func,    
		      sorter->progress_func_data, sorter->nipoints);
    
    sary_merger_destroy(merger);
}

void
sary_sorter_set_nthreads (SarySorter *sorter, SaryInt nthreads)
{
    g_assert(nthreads > 0);
    sorter->nthreads = nthreads;
}

void
sary_sorter_connect_progress (SarySorter *sorter,
			      SaryProgressFunc progress_func,
			      gpointer progress_func_data)
{
    g_assert(progress_func != NULL);

    sorter->progress_func = progress_func;
    sorter->progress_func_data = progress_func_data;
}

static Blocks *
new_blocks (SaryInt *array, 
	    SaryInt nipoints, 
	    SaryInt block_size, 
	    SaryInt nblocks)
{
    SaryInt i, offset, remain;
    Blocks *blocks;

    blocks = g_new(Blocks, 1);
    blocks->blocks = g_new(Block, nblocks);

    offset = 0;
    remain = nipoints;
    for (i = 0; i < nblocks; i++) {
	blocks->blocks[i].first = array + offset;
	blocks->blocks[i].len = MIN(block_size, remain);
	offset += block_size;
	remain -= block_size;
    }

    blocks->first  = blocks->blocks;
    blocks->cursor = blocks->first;
    blocks->last   = blocks->first + nblocks - 1;

    return blocks;
}

static void
destroy_blocks (Blocks *blocks)
{
    if (blocks != NULL) {
	g_free(blocks->blocks);
	g_free(blocks);
    }
}

static void
sort_block (SarySorter *sorter)
{
    Block *block;

    while ((block = get_next_block(sorter)) != NULL) {
	/*
	 * Pass NULL to the first argument to avoid
	 * sary_progress_set_count().  In multi-threaded
	 * sorting, mutex lock is necessary for
	 * sary_progress_set_count() but it't too expensive.
	 */
	sary_multikey_qsort(NULL,
			    block->first,
			    block->len,
			    0,
			    sary_text_get_bof(sorter->text),
			    sary_text_get_eof(sorter->text));
    
	pthread_mutex_lock(sorter->mutex);
	sary_progress_set_count(sorter->progress, 
				sorter->progress->current + block->len);
	pthread_mutex_unlock(sorter->mutex);
    }
}


static Block *
get_next_block (SarySorter *sorter)
{
    Block *block;

    pthread_mutex_lock(sorter->mutex);

    if (sorter->blocks->cursor > sorter->blocks->last) { /* exhausted */
	pthread_mutex_unlock(sorter->mutex);
	return NULL;
    }

    block = sorter->blocks->cursor;
    sorter->blocks->cursor++;

    pthread_mutex_unlock(sorter->mutex);

    return block;
}


static SaryInt
calc_nblocks (SaryInt nipoints,
	      SaryInt block_size)
{
    SaryInt result = nipoints / block_size;

    if (nipoints % block_size != 0) {
	result++;
    }

    return result;
}

