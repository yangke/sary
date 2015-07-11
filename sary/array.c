/* 
 * sary - a suffix array library
 *
 * $Id: array.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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
 * For backward compatibility. Use SaryBuilder instead.
 */

/*
 * Index `file_name' using `next_ipoint' and simply store the
 * generated index points to `file_name'.ary.
 */
SaryInt
sary_array_index (const gchar *file_name, SaryIpointFunc next_ipoint)
{
    SaryInt ipoints;
    gchar *array_name = g_strconcat(file_name, ".ary", NULL);

    ipoints = sary_array_index2(file_name, next_ipoint, array_name);
    g_free(array_name);
    return ipoints;
}

SaryInt
sary_array_index2 (const gchar *file_name, 
		   SaryIpointFunc next_ipoint,
		   const gchar *array_name)
{
    SaryInt ipoints;
    SaryBuilder *builder;

    builder = sary_builder_new2(file_name, array_name);
    sary_builder_set_ipoint_func(builder, next_ipoint);
    ipoints = sary_builder_index(builder);
    sary_builder_destroy(builder);

    return ipoints;
}

/*
 * Sort the index point made for `file_name' and create the
 * suffix array.
 */
gboolean
sary_array_sort (const gchar *file_name)
{
    gboolean status;
    gchar *array_name = g_strconcat(file_name, ".ary", NULL);

    status = sary_array_sort2(file_name, array_name);
    g_free(array_name);
    return status;
}


gboolean
sary_array_sort2 (const gchar *file_name, const gchar *array_name)
{
    gboolean status;
    SaryBuilder *builder;

    builder = sary_builder_new2(file_name, array_name);
    status  = sary_builder_sort(builder);
    sary_builder_destroy(builder);

    return status;
}

