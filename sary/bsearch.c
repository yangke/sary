/* 
 * sary - a suffix array library
 *
 * $Id: bsearch.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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
#include <glib.h>
#include <sary.h>

/*
 * This function is almost identical to bsearch(3) but
 * searches the first occurrence of the `key' and takes two
 * additional parameters `next_low' and `next_high'.
 * 
 * This algorithm is extracted from Jon Bentley's
 * Programming Pearls 2nd ed. p.93
 */
gpointer
sary_bsearch_first (gconstpointer key, 
		    gconstpointer base, 
		    gsize len,
		    gsize elt_size, 
		    SaryInt *next_low,
		    SaryInt *next_high,
		    GCompareFunc compare_func)
{
    SaryInt low, high, mid, cmp;
    SaryInt is_first = 1;

    g_assert(key != NULL && base != NULL && compare_func !=NULL);

    low = -1;
    high = len;
    g_assert(low < high);

    while (low+1 != high) {
	mid = (low + high) / 2;
	cmp = compare_func(key, base + mid * elt_size);
	if (cmp > 0) {
	    low = mid;
	} else {
	    /*
	     * Store the boundary information for the
	     * subsequent call of sary_bsearch_last.  It's
	     * rather ugly but doubles performance.
	     */
	    if (cmp == 0 && is_first) {
		*next_low  = low;
		*next_high = high;
		is_first = 0;
	    }
	    high = mid;
	}
    }
    if (high >= len || compare_func(key, base + high * elt_size) != 0) {
	return NULL;
    }
    return (gpointer)(base + high * elt_size);
}


/*
 * This function is almost identical to bsearch(3) but
 * search the LAST occurrence of the `key' and takes two
 * additional parameters `prev_low' and `prev_high'.
 */
gpointer
sary_bsearch_last (gconstpointer key, 
		   gconstpointer base, 
		   gsize len,
		   gsize elt_size, 
		   SaryInt prev_low,
		   SaryInt prev_high,
		   GCompareFunc compare_func)
{
    SaryInt low, high, mid;

    g_assert(key != NULL && base != NULL && compare_func !=NULL);

    low = prev_low;
    high = prev_high;
    g_assert(low < high);

    while (low+1 != high) {
	mid = (low + high) / 2;
	if (compare_func(key, base + mid * elt_size) >= 0) {
	    low = mid;
	} else {
	    high = mid;
	}
    }
    if (low <= -1 || compare_func(key, base + low * elt_size) != 0) {
	return NULL;
    }
    return (gpointer)(base + low * elt_size);
}
