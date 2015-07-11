/* 
 * sary - a suffix array library
 *
 * $Id: merger.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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
#include <string.h>
#include <sary.h>

enum {
    CACHE_SIZE = 16
};

typedef struct {
    SaryInt	*first;
    SaryInt	*cursor;
    SaryInt	*last;
    gchar	cache[CACHE_SIZE];
    SaryInt	cache_len;
} Block;

typedef struct {
    SaryText	*text;
    Block	**qblocks;
    SaryInt	len;
} Queue;

struct _SaryMerger {
    gchar	*array_name;
    Block	*blocks;
    SaryInt	nblocks;
    Queue	*queue;
};

static gboolean		merge			(Queue *queue, 
						 SaryProgress *progress, 
						 SaryWriter *writer);
static inline gboolean	is_block_exhausted	(Block *block);
static void		update_block_cache	(Block *block, 
						 SaryText *text);
static inline gint 	suffixcmp		(const gchar *s1, 
						 const gchar *s2, 
						 const gchar *eof);
static inline gint 	queuecmp		(SaryText *text, 
						 Block *b1, 
						 Block *b2);
static void 		queue_insert		(Queue *queue, 
						 Block *block);
static inline Block*	queue_minimum		(Queue *queue);
static inline void	queue_downsize		(Queue *queue);
static inline void	queue_rearrange		(Queue *queue);
static inline void	swap			(Block *blocks[], 
						 SaryInt i,
						 SaryInt j);

SaryMerger*
sary_merger_new	(SaryText *text, 
		 const gchar *array_name, 
		 SaryInt nblocks)
{
    SaryMerger *merger;

    g_assert(text != NULL);

    merger = g_new(SaryMerger, 1);
    merger->array_name = g_strdup(array_name);
    merger->blocks     = g_new(Block, nblocks);
    merger->nblocks    = 0;

    merger->queue          = g_new(Queue, 1);
    /*
     * Allocate nblock + 1 for heaps. The 0-th element will
     * never be used.
     */
    merger->queue->qblocks = g_new(Block*, nblocks + 1);
    merger->queue->len  = 0;
    merger->queue->text    = text;

    return merger;
}

void
sary_merger_destroy (SaryMerger *merger)
{
    g_free(merger->blocks);
    g_free(merger->queue->qblocks);
    g_free(merger->queue);
    g_free(merger->array_name);
    g_free(merger);
}

void
sary_merger_add_block (SaryMerger *merger, SaryInt *head, SaryInt len)
{
    Block block, *added_block;

    g_assert(head != NULL && len >= 0);

    block.first  = head;
    block.cursor = head;
    block.last   = head + len - 1;

    merger->blocks[merger->nblocks] = block;
    added_block = merger->blocks + merger->nblocks;
    queue_insert(merger->queue, added_block);

    merger->nblocks++;
}

gboolean
sary_merger_merge(SaryMerger *merger, 
		  SaryProgressFunc progress_func,
		  gpointer progress_func_data,
		  SaryInt nipoints) /* number of index points */
{
    gboolean result;
    SaryProgress *progress;
    SaryWriter *writer;

    progress = sary_progress_new("merge", nipoints);
    sary_progress_connect(progress, progress_func, progress_func_data);

    writer = sary_writer_new(merger->array_name);
    if (writer == NULL) {
	return FALSE;
    }

    g_assert(merger->nblocks == merger->queue->len);
    result = merge(merger->queue, progress, writer);

    sary_progress_destroy(progress);
    sary_writer_destroy(writer);

    return result;
}

static gboolean
merge (Queue *queue, SaryProgress *progress, SaryWriter *writer)
{
    SaryInt count = 0;

    while (queue->len > 0) {
	Block *block = queue_minimum(queue);

	if (sary_writer_write(writer, *block->cursor) == FALSE) {
	    return FALSE;
	}

	block->cursor++;
	if (is_block_exhausted(block)) {
	    queue_downsize(queue);
	} else {
	    update_block_cache(block, queue->text);
	}
	queue_rearrange(queue);

	sary_progress_set_count(progress, count);
	count++;
    }
    if (sary_writer_flush(writer) == FALSE) {
	return FALSE;
    }
    return TRUE;
}

static inline gboolean
is_block_exhausted(Block *block)
{
    return block->cursor > block->last;
}

static inline gint 
suffixcmp (const gchar *s1, const gchar *s2, const gchar *eof)
{
    gint cmp;

    SaryInt len1 = eof - s1;
    SaryInt len2 = eof - s2;

    cmp = memcmp(s1, s2, MIN(len1, len2));
    if (cmp == 0) {
	return len1 - len2;  /* compare by length */
    } else {
	return cmp;
    }
}

static inline gint 
queuecmp (SaryText *text, Block *b1, Block *b2)
{
    /*
     * Consult cache first.
     */
    gint len = MIN(b1->cache_len, b2->cache_len);
    gint cmp = memcmp(b1->cache, b2->cache, len);

    if (cmp == 0) {
	gchar *eof     = sary_text_get_eof(text);
	gchar *suffix1 = sary_i_text(text, b1->cursor) + len;
	gchar *suffix2 = sary_i_text(text, b2->cursor) + len;

	cmp = suffixcmp(suffix1, suffix2, eof);
    }
    return cmp;
}

/*
 * To reduce disk accesses, cache the head of a suffix for
 * each block.
 */
static void
update_block_cache (Block *block, SaryText *text)
{
    gchar *suffix = sary_i_text(text, block->cursor);
    SaryInt len   = sary_text_get_eof(text) - suffix;

    block->cache_len = MIN(len, CACHE_SIZE);
    g_memmove(block->cache, suffix, block->cache_len);
}


/*
 * Priority queue represented with heaps.
 *
 * Reference: 
 * Jon Bentley: Programming Pearls 2nd ed. pp. 147-159.
 * <http://www.programmingpearls.com/>
 */

static void 
queue_insert (Queue *queue, Block *block)
{
    SaryInt i;
    Block **qblocks = queue->qblocks;

    queue->len++;
    qblocks[queue->len] = block;

    update_block_cache(block, queue->text);

    for (i = queue->len; i > 1 && 
	     queuecmp(queue->text, qblocks[i / 2],  qblocks[i]) > 0; i /= 2) 
    {
	swap(qblocks, i / 2, i);
    }

}

static inline Block *
queue_minimum (Queue *queue)
{
    return queue->qblocks[1];
}

static inline void
queue_downsize (Queue *queue)
{
    queue->qblocks[1] = queue->qblocks[queue->len];
    queue->len--;
}

/*
 * Rearrange heap structure. Relocate the 1st element to the
 * appropriate position.
 */
static inline void
queue_rearrange (Queue *queue)
{
    SaryInt i, c;
    Block **qblocks = queue->qblocks;

    for (i = 1; i * 2 <= queue->len; i = c) {
	c = 2 * i;
	if (c + 1 <= queue->len && 
	    queuecmp(queue->text, qblocks[c + 1], qblocks[c]) < 0) 
	{
	    c++;
	}
	if (queuecmp(queue->text, qblocks[i], qblocks[c]) <= 0) {
	    break;
	}
	swap(qblocks, c, i);
    }
}

static inline void 
swap (Block **qblocks, SaryInt i, SaryInt j)
{
    Block *tmp;

    tmp  = qblocks[i];
    qblocks[i] = qblocks[j];
    qblocks[j] = tmp;
}

