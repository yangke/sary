/* 
 * sary - a suffix array library
 *
 * $Id: cache.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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
#include <string.h>
#include <glib.h>
#include <sary.h>

static void	destroy_element	(gpointer key, 
				 gpointer value, 
				 gpointer use_data);
static guint	pattern_hash	(gconstpointer key);
static gint	pattern_equal	(gconstpointer v, gconstpointer v2);


SaryCache *
sary_cache_new (void)
{
    return g_hash_table_new(pattern_hash, pattern_equal);
}

void
sary_cache_destroy (SaryCache *cache)
{
    if (cache != NULL) {
	g_hash_table_foreach(cache, destroy_element, NULL);
	g_hash_table_destroy(cache);
    }
}

SaryResult *
sary_cache_get (SaryCache *cache, const gchar *pattern, SaryInt len)
{
    SaryPattern key;

    key.str = (gchar *)pattern;
    key.len = len;

    return (SaryResult *)g_hash_table_lookup(cache, &key);
}

void
sary_cache_add (SaryCache *cache, 
		const gchar *pattern,
		SaryInt len,
		SaryInt *first,
		SaryInt *last)
{
    SaryResult *item  = g_new(SaryResult, 1);
    SaryPattern *key = g_new(SaryPattern, 1);

    key->str = pattern;
    key->len = len;

    item->first    = first;
    item->last     = last;

    g_hash_table_insert(cache, key, item);
}

/* 
 * Similar to GLib's g_str_hash but it handles `len'.
 */
static guint
pattern_hash (gconstpointer key)
{
    SaryPattern *pattern = (SaryPattern *)key;
    const gchar *p    = pattern->str;
    SaryInt len = pattern->len;
    guint h = 0;
  
    for (; len > 0; len--, p++) {
	h = (h << 5) - h + *p;
    }
  
    return h;
}

static gint
pattern_equal (gconstpointer v, gconstpointer v2)
{
    SaryPattern *p1 = (SaryPattern *)v;
    SaryPattern *p2 = (SaryPattern *)v2;

    if (p1->len == p2->len) {
	return memcmp(p1->str, p2->str, p1->len) == 0;
    } else {
	return 0; /* not equal */
    }
}
static void
destroy_element (gpointer element, 
		 gpointer value, 
		 gpointer use_data)
{
    SaryPattern *elt = (SaryPattern *)element;
    g_free(elt);
}
