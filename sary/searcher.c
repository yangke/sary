/* 
 * sary - a suffix array library
 *
 * $Id: searcher.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <ctype.h>
#include <sary.h>

/* 
 * SarySearcher stands for Suffix Array Searcher. 
 */

typedef	gboolean	(*SearchFunc) (SarySearcher *searcher, 
				       const gchar *pattern, 
				       SaryInt len, 
				       SaryInt offset,
				       SaryInt range);
struct _SarySearcher {
    SaryInt     len;    /* number of index points */
    SaryText    *text;
    SaryMmap    *array;
    SaryInt  	*first;
    SaryInt     *last;
    SaryInt     *cursor;
    SaryInt	*allocated_data;
    gboolean    is_sorted;
    gboolean    is_allocated;
    SaryPattern	pattern;
    SaryCache	*cache;
    SearchFunc  search;
};

typedef gchar* (*SeekFunc)(const gchar *cursor, 
			   const gchar *sentinel,
			   gconstpointer data);
typedef struct {
    SeekFunc		seek_backward;
    SeekFunc		seek_forward;
    gconstpointer	backward_data;
    gconstpointer	forward_data;
} Seeker;

typedef struct {
    const gchar *str;
    SaryInt len;
} Tag;

typedef struct {
    gchar **patterns;
    gint  npatterns;
} Patterns;

static gchar *		peek_next_occurrence	(SarySearcher *searcher);
static void		init_searcher_states	(SarySearcher *searcher, 
						 gboolean first_time);
static gboolean		search 			(SarySearcher *searcher, 
						 const gchar *pattern, 
						 SaryInt len, 
						 SaryInt offset,
						 SaryInt range);
static inline gint	bsearchcmp		(gconstpointer searcher_ptr, 
					 	 gconstpointer obj_ptr);
static inline gint	qsortcmp		(gconstpointer ptr1, 
						 gconstpointer ptr2);
static inline gint	qsortscmp		(gconstpointer ptr1,
                                                 gconstpointer ptr2);
static Patterns*	patterns_new		(gchar **patterns,
                                                 gint npatterns);
static void		patterns_destroy	(Patterns *pat);
static void		patterns_sort		(Patterns *pat);
static gboolean		cache_search		(SarySearcher *searcher, 
						 const gchar *pattern, 
						 SaryInt len, 
						 SaryInt offset,
						 SaryInt range);
static GArray*		icase_search		(SarySearcher *searcher, 
						 gchar *pattern, 
						 SaryInt len,
						 SaryInt step, 
						 GArray *result);
static gint		expand_letter		(gint *cand, gint c);
static void		assign_range		(SarySearcher *searcher, 
						 SaryInt *occurences, 
						 SaryInt len);
static gchar*		get_next_region		(SarySearcher *searcher, 
						 Seeker *seeker,
						 SaryInt *len);
static gchar*		join_subsequent_region	(SarySearcher *searcher, 
						 Seeker *seeker,
						 gchar *tail);
static gchar*		get_region		(const gchar *head, 
						 const gchar *eof, 
						 SaryInt len);
static gchar*		seek_lines_backward	(const gchar *cursor, 
						 const gchar *bof,
						 gconstpointer n_ptr);
static gchar*		seek_lines_forward 	(const gchar *cursor, 
						 const gchar *eof,
						 gconstpointer n_ptr);
static gchar*		seek_tag_backward	(const gchar *cursor, 
						 const gchar *eof,
						 gconstpointer tag_ptr);
static gchar*		seek_tag_forward	(const gchar *cursor, 
						 const gchar *eof,
						 gconstpointer tag_ptr);
static gboolean		has_prev_as_prefix	(const gchar *prev,
                                                 const gchar *cur);

SarySearcher *
sary_searcher_new (const gchar *file_name)
{
    SarySearcher *searcher;
    gchar *array_name = g_strconcat(file_name, ".ary", NULL);

    searcher = sary_searcher_new2(file_name, array_name);
    g_free(array_name);
    return searcher;
}

SarySearcher *
sary_searcher_new2 (const gchar *file_name, const gchar *array_name)
{
    SarySearcher *searcher = g_new(SarySearcher, 1);

    searcher->text = sary_text_new(file_name);
    if (searcher->text == NULL) {
	return NULL;
    }

    searcher->array = sary_mmap(array_name, "r");
    if (searcher->array == NULL) {
	return NULL;
    }

    searcher->len    = searcher->array->len / sizeof(SaryInt);
    searcher->search = search;
    searcher->cache  = NULL;

    init_searcher_states(searcher, TRUE);

    return searcher;
}

void
sary_searcher_destroy (SarySearcher *searcher)
{
    sary_text_destroy(searcher->text);
    sary_cache_destroy(searcher->cache);
    sary_munmap(searcher->array);

    g_free(searcher->allocated_data);
    g_free(searcher);
}

gboolean
sary_searcher_search (SarySearcher *searcher, const gchar *pattern)
{
    return sary_searcher_search2(searcher, pattern, strlen(pattern));
}

gboolean
sary_searcher_search2 (SarySearcher *searcher, 
                       const gchar *pattern,
                       SaryInt len)
{
    g_assert(searcher != NULL);
    init_searcher_states(searcher, FALSE);

    /*
     * Search the full range of the suffix array.
     */
    return searcher->search(searcher, pattern, len, 0, searcher->len);
}

gboolean
sary_searcher_multi_search (SarySearcher *searcher,
                            gchar **patterns, 
                            gint npatterns)
{
    gint i;
    GArray *occurences = g_array_new(FALSE, FALSE, sizeof(SaryInt));
    Patterns *pat = patterns_new(patterns, npatterns);
    gboolean first_time = TRUE, result;

    g_assert(searcher != NULL);
    init_searcher_states(searcher, FALSE);

    patterns_sort(pat);
    for (i = 0; i < pat->npatterns; i++) {
        /*
         * If the previous pattern is "a" and the current
         * one is "ab", we can skip the current one because
         * the previous results include all results for the
         * current one.
         */
        if (first_time || !has_prev_as_prefix(pat->patterns[i - 1], 
                                              pat->patterns[i])) 
        {
            if (sary_searcher_search(searcher, pat->patterns[i])) {
                SaryInt len = sary_searcher_count_occurrences(searcher);
		g_array_append_vals(occurences, searcher->first, len);
            }
            first_time = FALSE;
        }
    }
    patterns_destroy(pat);

    if (occurences->len == 0) { /* no pattern found */
        result = FALSE;
    } else {
        searcher->is_allocated = TRUE;
        searcher->allocated_data = (SaryInt *)occurences->data;
        assign_range(searcher, searcher->allocated_data, occurences->len);
        result = TRUE;
    }
    g_array_free(occurences, FALSE); /* don't free the data */
    return result;
}

gboolean
sary_searcher_isearch (SarySearcher *searcher, 
                       const gchar *pattern,
                       SaryInt len)
{
    SaryInt offset, range;
    gboolean result;

    g_assert(searcher->pattern.skip <= len && 
	     searcher->is_sorted == FALSE);

    if (searcher->pattern.skip == 0) { /* the first time */
	init_searcher_states(searcher, FALSE);
	offset = 0;
	range  = searcher->len;
    } else {
	offset = (gconstpointer)searcher->first - searcher->array->map;
	range  = sary_searcher_count_occurrences(searcher);
    }

    /*
     * Search the range of the previous search results.
     * Don't use sary_searcher_sort_occurrences together.
     */
    result = searcher->search(searcher, pattern, len, offset, range);
    searcher->pattern.skip = len;
    return result;
}

gboolean
sary_searcher_icase_search (SarySearcher *searcher, const gchar *pattern)
{
    return sary_searcher_icase_search2(searcher, pattern, strlen(pattern));
}

gboolean
sary_searcher_icase_search2 (SarySearcher *searcher, 
                             const gchar *pattern, 
                             SaryInt len)
{
    gboolean result;
    GArray *occurences;
    gchar *tmppat;

    g_assert(len >= 0);
    init_searcher_states(searcher, FALSE);

    if (len == 0) { /* match all occurrences */
	return sary_searcher_isearch(searcher, pattern, len);
    }

    tmppat = g_new(gchar, len);  /* for modifications in icase_search. */
    g_memmove(tmppat, pattern, len);

    occurences = g_array_new(FALSE, FALSE, sizeof(SaryInt));
    occurences = icase_search(searcher, tmppat, len, 0, occurences);

    if (occurences->len == 0) { /* not found */
	result = FALSE;
    } else {
	searcher->is_allocated   = TRUE;
	searcher->allocated_data = (SaryInt *)occurences->data;
	assign_range(searcher, searcher->allocated_data, occurences->len);
	result = TRUE;
    }

    g_free(tmppat);
    g_array_free(occurences, FALSE); /* don't free the data */

    return result;
}

void
sary_searcher_isearch_reset (SarySearcher *searcher)
{
    searcher->pattern.skip = 0;
}

SaryText *
sary_searcher_get_text (SarySearcher *searcher)
{
    return searcher->text;
}

SaryMmap *
sary_searcher_get_array (SarySearcher *searcher)
{
    return searcher->array;
}

gchar *
sary_searcher_get_next_line (SarySearcher *searcher)
{
    /*
     * This function is only a special case of
     * sary_searcher_get_next_context_lines().  If
     * sary_searcher_sort_occurrences() is used and the patttern
     * occurs more than once in the same line, duplicated
     * lines will never returned because of the work of
     * join_subsequent_lines().
     */
    return sary_searcher_get_next_context_lines(searcher, 0, 0);
}

gchar *
sary_searcher_get_next_line2 (SarySearcher *searcher, SaryInt *len)
{
    return sary_searcher_get_next_context_lines2(searcher, 0, 0, len);
}

/*
 * Act like GNU grep -A -B -C. Subsequent lines are joined
 * not to print duplicated lines if occurrences are sorted. 
 */

gchar *
sary_searcher_get_next_context_lines (SarySearcher *searcher, 
			       SaryInt backward, 
			       SaryInt forward)
{
    gchar *head, *eof;
    SaryInt len;

    eof  = sary_text_get_eof(searcher->text);
    head = sary_searcher_get_next_context_lines2(searcher, backward, 
                                                 forward, &len);

    return get_region(head, eof, len);
}

/*
 * Act like GNU grep -A -B -C. Subsequent lines are joined
 * not to print duplicated lines if occurrences are sorted. 
 */

gchar *
sary_searcher_get_next_context_lines2 (SarySearcher *searcher, 
				SaryInt backward, 
				SaryInt forward,
				SaryInt *len)
{
    Seeker seeker;
    g_assert(backward >= 0 && forward >=0);

    seeker.seek_backward = seek_lines_backward;
    seeker.seek_forward  = seek_lines_forward;
    seeker.backward_data = &backward;
    seeker.forward_data  = &forward;

    return get_next_region(searcher, &seeker, len);
}

gchar *
sary_searcher_get_next_tagged_region (SarySearcher *searcher,
			       const gchar *start_tag,
			       const gchar *end_tag)
{
    gchar *head, *eof;
    SaryInt len;
    SaryInt start_tag_len, end_tag_len;

    start_tag_len = strlen(start_tag);
    end_tag_len   = strlen(end_tag);

    eof  = sary_text_get_eof(searcher->text);
    head = sary_searcher_get_next_tagged_region2(searcher, 
                                                 start_tag, start_tag_len,
                                                 end_tag, end_tag_len, &len);
    return get_region(head, eof, len);
}

gchar *
sary_searcher_get_next_tagged_region2 (SarySearcher *searcher,
                                       const gchar *start_tag,
                                       SaryInt start_tag_len,
                                       const gchar *end_tag,
                                       SaryInt end_tag_len,
                                       SaryInt *len)
{
    Seeker seeker;
    Tag start, end;

    g_assert(start_tag != NULL && end_tag != NULL);
    g_assert(start_tag_len >= 0 && end_tag_len >= 0);

    start.str = start_tag;
    start.len = start_tag_len;
    end.str   = end_tag;
    end.len   = end_tag_len;

    seeker.seek_backward = seek_tag_backward;
    seeker.seek_forward  = seek_tag_forward;
    seeker.backward_data = &start;
    seeker.forward_data  = &end;

    return get_next_region(searcher, &seeker, len);
}

/*
 * Return the text object whose cursor points to the
 * position of the occurrence in the text. This function is
 * useful for low-level text processing with sary_text_*
 * functions.
 */

SaryText *
sary_searcher_get_next_occurrence (SarySearcher *searcher)
{
    gchar *occurrence;

    if (searcher->cursor > searcher->last) {
	return NULL;
    }

    occurrence = sary_i_text(searcher->text, searcher->cursor);
    sary_text_set_cursor(searcher->text, occurrence);
    searcher->cursor++;

    return searcher->text;
}

SaryInt
sary_searcher_get_next_position (SarySearcher *searcher)
{
    SaryInt position;

    if (searcher->cursor > searcher->last) {
        return -1;
    }

    position =  GINT_FROM_BE(*(searcher->cursor));
    searcher->cursor++;
    return position;
}

SaryInt
sary_searcher_count_occurrences (SarySearcher *searcher)
{
    return searcher->last - searcher->first + 1;
}

void
sary_searcher_sort_occurrences (SarySearcher *searcher)
{
    SaryInt len;

    len = sary_searcher_count_occurrences(searcher);

    if (searcher->is_allocated == FALSE) {
	searcher->allocated_data = g_new(SaryInt, len);
	g_memmove(searcher->allocated_data, 
		  searcher->first, len * sizeof(SaryInt));
	searcher->is_allocated = TRUE;
    }

    qsort(searcher->allocated_data, len, sizeof(SaryInt), qsortcmp);
    assign_range(searcher, searcher->allocated_data, len);
    searcher->is_sorted = TRUE;
}

void
sary_searcher_enable_cache (SarySearcher *searcher)
{
    searcher->cache  = sary_cache_new();
    searcher->search = cache_search;
}

static gchar *
peek_next_occurrence (SarySearcher *searcher)
{
    gchar *occurrence;

    if (searcher->cursor > searcher->last) {
	return NULL;
    }

    occurrence = sary_i_text(searcher->text, searcher->cursor);
    return occurrence;
}

static void
init_searcher_states (SarySearcher *searcher, gboolean first_time)
{
    if (!first_time) {
	g_free(searcher->allocated_data);
    }
    searcher->allocated_data = NULL;
    searcher->is_allocated   = FALSE;
    searcher->is_sorted      = FALSE;
    searcher->first     = NULL;
    searcher->last      = NULL;
    searcher->cursor    = NULL;
    searcher->pattern.skip = 0;
}

static gboolean
search (SarySearcher *searcher, 
	const gchar *pattern, 
	SaryInt len, 
	SaryInt offset,
	SaryInt range)
{
    SaryInt *first, *last;
    SaryInt next_low, next_high;

    g_assert(len >= 0);

    if (searcher->array->map == NULL) {  /* 0-length (empty) file */
	return FALSE;
    }

    searcher->pattern.str = (gchar *)pattern;
    searcher->pattern.len = len;

    first = (SaryInt *)sary_bsearch_first(searcher, 
					  searcher->array->map + offset,
					  range, sizeof(SaryInt), 
					  &next_low, &next_high,
					  bsearchcmp);
    if (first == NULL) {
	return FALSE;
    }

    last  = (SaryInt *)sary_bsearch_last(searcher, 
					 searcher->array->map + offset, 
					 range, sizeof(SaryInt),
					 next_low, next_high,
					 bsearchcmp);
    g_assert(last != NULL);

    searcher->first   = first;
    searcher->last    = last;
    searcher->cursor  = first;

    return TRUE;
}

static inline gint 
bsearchcmp (gconstpointer sary_searcher_ptr, gconstpointer obj_ptr)
{
    gint len1, len2, skip;
    SarySearcher *searcher  = (SarySearcher *)sary_searcher_ptr;
    gchar *eof  = sary_text_get_eof(searcher->text);
    gchar *pos = sary_i_text(searcher->text, obj_ptr);

    skip = searcher->pattern.skip;
    len1 = searcher->pattern.len - skip;
    len2 = eof - pos - skip;
    if (len2 < 0) {
	len2 = 0;
    }

    return memcmp(searcher->pattern.str + skip, pos + skip, MIN(len1, len2));
}

static inline gint 
qsortcmp (gconstpointer ptr1, gconstpointer ptr2)
{
    SaryInt occurrence1 = GINT_FROM_BE(*(SaryInt *)ptr1);
    SaryInt occurrence2 = GINT_FROM_BE(*(SaryInt *)ptr2);

    if (occurrence1 < occurrence2) {
	return -1;
    } else if (occurrence1 == occurrence2) {
	return 0;
    } else {
	return 1;
    }
}

static inline gint 
qsortscmp (gconstpointer ptr1, gconstpointer ptr2)
{
    const gchar *str1 = *(const gchar **)ptr1;
    const gchar *str2 = *(const gchar **)ptr2;

    return strcmp(str1, str2);
}

static gboolean
cache_search (SarySearcher *searcher, 
	      const gchar *pattern, 
	      SaryInt len, 
	      SaryInt offset,
	      SaryInt range)
{
    SaryResult *cache;

    if ((cache = sary_cache_get(searcher->cache, pattern, len)) != NULL) {
	searcher->first   = cache->first;
	searcher->last    = cache->last;
	searcher->cursor  = cache->first;
	return TRUE;
    } else {
	gboolean result = search(searcher, pattern, len, offset, range);
	if (result == TRUE) {
	    sary_cache_add(searcher->cache, 
			   sary_i_text(searcher->text, searcher->first), len, 
			   searcher->first, searcher->last);
	}
	return result;
    }
    g_assert_not_reached();
}

static GArray *
icase_search (SarySearcher *searcher, 
	      gchar *pattern,
	      SaryInt len,
	      SaryInt step, 
	      GArray *result)
{
    gint cand[2], ncand; /* candidates and the number of candidates */
    gint i;

    ncand = expand_letter(cand, (guchar)pattern[step]);
    for (i = 0; i < ncand; i++) {
	SaryInt *orig_first = searcher->first;
	SaryInt *orig_last  = searcher->last;

	pattern[step] = cand[i];
	if (sary_searcher_isearch(searcher, pattern, step + 1)) {
	    if (step + 1 < len) {
		result = icase_search(searcher, pattern,
                                      len, step + 1, result);
	    } else if (step + 1 == len) {
		g_array_append_vals(result, searcher->first, 
				    sary_searcher_count_occurrences(searcher));
	    } else {
		g_assert_not_reached();
	    }
	}
	searcher->first = orig_first;
	searcher->last  = orig_last;
	searcher->pattern.skip--;
    }

    return result;
}

static gint
expand_letter (gint *cand, gint c)
{
    if (isalpha(c)) {
	/* 
	 * To preserve lexicographical order, do toupper first.
	 * Assume 'A' < 'a'.
	 */
	cand[0] = toupper(c); 
	cand[1] = tolower(c);
	return 2;
    } else {
	cand[0] = c;
	return 1;
    }
}

static void
assign_range (SarySearcher *searcher, SaryInt *occurences, SaryInt len)
{
    searcher->first  = occurences;
    searcher->cursor = occurences;
    searcher->last   = occurences + len - 1;
}

static gchar *
get_next_region (SarySearcher *searcher, Seeker *seeker, SaryInt *len)
{
    gchar *bof, *eof, *cursor;
    gchar *head, *tail;

    if (searcher->cursor > searcher->last) {
	return NULL;
    }

    bof    = sary_text_get_bof(searcher->text);
    eof    = sary_text_get_eof(searcher->text);
    cursor = sary_i_text(searcher->text, searcher->cursor);

    head   = seeker->seek_backward(cursor, bof, seeker->backward_data);
    tail   = seeker->seek_forward(cursor, eof, seeker->forward_data);

    searcher->cursor++; /* Must be called before join_subsequent_region. */
    if (searcher->is_sorted == TRUE) {
	tail = join_subsequent_region(searcher, seeker, tail);
    }

    *len = tail - head;
    return head;
}

static gchar *
join_subsequent_region (SarySearcher *searcher, Seeker *seeker, gchar *tail)
{
    gchar *bof = sary_text_get_bof(searcher->text);
    gchar *eof = sary_text_get_eof(searcher->text);

    do {
	gchar *next, *next_head;

	next = peek_next_occurrence(searcher);
	if (next == NULL) {
	    break;
	}
	next_head = seeker->seek_backward(next, bof, seeker->backward_data);
	if (next_head < tail) {
	    sary_searcher_get_next_occurrence(searcher);  /* skip */
	    tail = seeker->seek_forward(next, eof, seeker->forward_data);
	} else {
	    break;
	}
    } while (1);


    return tail;
}

static gchar *
get_region (const gchar *head, const gchar *eof, SaryInt len)
{
    if (head == NULL) {
	return NULL;
    } else {
	return sary_str_get_region(head, eof, len);
    }
}

static gchar *
seek_lines_backward (const gchar *cursor, 
		     const gchar *bof,
		     gconstpointer n_ptr)
{
    SaryInt n = *(gint *)n_ptr;
    return sary_str_seek_lines_backward(cursor, bof, n);
}

static gchar *
seek_lines_forward (const gchar *cursor, 
		    const gchar *eof,
		    gconstpointer n_ptr)
{
    SaryInt n = *(gint *)n_ptr;
    return sary_str_seek_lines_forward(cursor, eof, n);
}

static gchar *
seek_tag_backward (const gchar *cursor, 
		   const gchar *bof,
		   gconstpointer tag_ptr)
{
    Tag *tag = (Tag *)tag_ptr;
    return sary_str_seek_pattern_backward2(cursor, bof, tag->str, tag->len);
}

static gchar *
seek_tag_forward (const gchar *cursor, 
		  const gchar *eof,
		  gconstpointer tag_ptr)
{
    Tag *tag = (Tag *)tag_ptr;
    return sary_str_seek_pattern_forward2(cursor, eof, tag->str, tag->len);
}


static Patterns *
patterns_new (gchar **patterns, gint npatterns)
{
    gint i;
    Patterns *pat = malloc(sizeof(Patterns));

    pat->patterns  = g_new(gchar *, npatterns);
    pat->npatterns = npatterns;
    for (i = 0; i < npatterns; i++) {
        pat->patterns[i] = g_strdup(patterns[i]);
    }
    return pat;
}

static void
patterns_sort (Patterns *pat)
{
    qsort(pat->patterns, pat->npatterns, sizeof(gchar *), qsortscmp);
}

static void
patterns_destroy (Patterns *pat)
{
    int i;
    for (i = 0; i < pat->npatterns; i++) {
        g_free(pat->patterns[i]);
    }
    g_free(pat);
}

static gboolean
has_prev_as_prefix (const gchar *prev, const gchar *cur)
{
    if (strncmp(prev, cur, strlen(prev)) == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

