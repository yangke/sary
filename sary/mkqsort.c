/* 
 * sary - a suffix array library
 *
 * $Id: mkqsort.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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
#include <sary.h>

/*
 * Multikey Quicksort:
 * Jon L. Bentley, Robert Sedgewick: "Fast Algorithms for Sorting and
 * Searching Strings," Proceedings of the Eighth Annual ACM-SIAM
 * Symposium on Discrete Algorithms, 1997.
 * <http://www.cs.princeton.edu/~rs/strings/>
 */

static void		insertion_sort	(SaryInt *array, 
					 gint len, 
					 gint depth, 
					 const gchar *bof, 
					 const gchar *eof);

static inline void	swap		(SaryInt *array, 
					 SaryInt a, 
					 SaryInt b);
					
static inline void	vecswap 	(SaryInt i,
					 SaryInt j, 
					 SaryInt n, 
					 SaryInt *array);
					
static inline gint	ref		(const gchar *bof, 
					 SaryInt offset, 
					 SaryInt depth, 
					 const gchar *eof);

static inline void	swap2		(SaryInt *a, SaryInt *b);

void
sary_multikey_qsort (SaryProgress *progress,
		     SaryInt *array,
		     SaryInt len,
		     SaryInt depth,
		     const gchar *bof,
		     const gchar *eof)
{
    SaryInt  a, b, c, d, r, v;

    if (len <= 10) {
	insertion_sort(array, len, depth, bof, eof);
	if (progress != NULL) {
	    sary_progress_set_count(progress, progress->current + len);
	}
	return;
    }

    a = rand() % len;
    swap(array, 0, a);

    v = ref(bof, array[0], depth, eof);
    a = b = 1;
    c = d = len - 1;

    while (1) {
        while (b <= c && (r = ref(bof, array[b], depth, eof) - v) <= 0) {
            if (r == 0) {
		swap(array, a, b); 
		a++;
	    }
            b++;
        }
        while (b <= c && (r = ref(bof, array[c], depth, eof) - v) >= 0) {
            if (r == 0) {
		swap(array, c, d); 
		d--;
	    }
            c--;
        }
        if (b > c) {
	    break;
	}
        swap(array, b, c);
        b++;
        c--;
    }

    r = MIN(a, b - a);
    vecswap(0, b - r, r, array);

    r = MIN(d - c, len - d - 1);
    vecswap(b, len - r, r, array);

    r = b - a;
    sary_multikey_qsort(progress, array, r, depth, bof, eof);

    if (ref(bof, array[r], depth, eof) != EOF) {
        sary_multikey_qsort(progress, array + r, 
			    a + len - d - 1, depth + 1, bof, eof);
    }
    r = d - c;
    sary_multikey_qsort(progress, array + len - r, r, depth, bof, eof);
}

static void
insertion_sort(SaryInt *array, gint len, gint depth, 
	       const gchar *bof, const gchar *eof)
{
    SaryInt *pi, *pj;

    g_assert(len <= 10);

    for (pi = array + 1; --len > 0; pi++) {
        for (pj = pi; pj > array; pj--) {
	    const gchar *s = bof + GINT_FROM_BE(*(pj - 1)) + depth;
	    const gchar *t = bof + GINT_FROM_BE(*pj) + depth;

	    for (; s < eof && t < eof && *s == *t; s++, t++)
		;
	    if (s == eof || (t != eof && (guchar)*s <= (guchar)*t)) {
		break;
	    }
	    swap2(pj, pj - 1);
	}
    }
}



static inline void
swap (SaryInt *array, SaryInt a, SaryInt b)
{
    SaryInt t = array[a];
    array[a]  = array[b]; 
    array[b]  = t;
}

static inline void
vecswap (SaryInt i, SaryInt j, SaryInt n, SaryInt *array)
{
    while (n-- > 0) {
	swap(array, i, j);
	i++;
	j++;
    }
}

/*
 * If pos exceeds eof, return EOF. Avoid referencing pos out
 * of the `mmap'ed area.
 */
static inline gint
ref (const gchar *bof, 
     SaryInt offset, 
     SaryInt depth, 
     const gchar *eof)
{
    const gchar *pos = bof + GINT_FROM_BE(offset) + depth;
    return pos < eof ? (guchar)*pos : EOF;
}


static inline void
swap2 (SaryInt *a, SaryInt *b)
{
    SaryInt t = *a;
    *a = *b;
    *b = t;
}

