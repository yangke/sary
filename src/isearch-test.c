/* 
 * sary - a suffix array library
 *
 * $Id: isearch-test.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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
#include <sary.h>

static void 	isearch			(const gchar *file_name,
					 const gchar *pattern);
static void 	show_usage		(void);

int 
main (int argc, char **argv)
{
    gchar *file_name, *pattern;

    if (argc != 3) {
	show_usage();
	exit(EXIT_FAILURE);
    }

    pattern   = argv[1];
    file_name = argv[2];
    isearch(file_name, pattern);

    return 0;
}

static void
isearch (const gchar *file_name, const gchar *pattern)
{
    SarySearcher *searcher;
    SaryInt i;
    SaryInt len = strlen(pattern);

    searcher = sary_searcher_new(file_name);
    if (searcher == NULL) {
	g_printerr("isearch-test: %s(.ary): %s\n", 
		   file_name, g_strerror(errno));
	exit(EXIT_FAILURE);
    }
    for (i = 0; i < len ; i++) {
	gchar *line;

	if (sary_searcher_isearch(searcher, pattern, i + 1) == FALSE) {
	    break;
	}
	while ((line = sary_searcher_get_next_line(searcher))) {
	    g_print("%s", line);
	    g_free(line);
	}
    }
    sary_searcher_destroy(searcher);
}

static void
show_usage (void)
{
    g_print("Usage: isearch-test <pattern> <file>\n");
}

