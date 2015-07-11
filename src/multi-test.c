/* 
 * sary - a suffix array library
 *
 * $Id: multi-test.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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

static void 	fgrep			(const gchar *pattern_file_name,
                                         const gchar *file_name);
static void 	show_usage		(void);

int 
main (int argc, char **argv)
{
    gchar *pattern_file_name, *file_name;

    if (argc != 3) {
	show_usage();
	exit(EXIT_FAILURE);
    }

    pattern_file_name = argv[1];
    file_name = argv[2];
    fgrep(pattern_file_name, file_name);

    return 0;
}

typedef struct {
    gchar **patterns;
    gint  npatterns;
} Patterns;

static Patterns *
get_patterns (const gchar *pattern_file_name)
{
    gint i;
    Patterns *patterns = malloc(sizeof(Patterns));
    char line[BUFSIZ];
    FILE *fp = fopen (pattern_file_name, "r");

    if (fp == NULL) {
	g_printerr("multi-test: %s: %s\n", pattern_file_name,
                   g_strerror(errno));
        exit(EXIT_FAILURE);
    }

    patterns->npatterns = 0;
    patterns->patterns  = g_new(gchar *, 100); /* max 100 patterns */
    i = 0;
    while (fgets(line, BUFSIZ, fp)) {
        line[strlen(line)-1] = '\0';
        patterns->patterns[i] = g_strdup(line);
        i++;
    }
    patterns->npatterns = i;
    return patterns;
}


/*
 * This is a test for sary_searcher_multi_search().
 */
static void
fgrep (const gchar *pattern_file_name, const gchar *file_name)
{
    SarySearcher *searcher;
    Patterns *patterns = get_patterns(pattern_file_name);

    searcher = sary_searcher_new(file_name);
    if (searcher == NULL) {
	g_printerr("multi-test: %s(.ary): %s\n", file_name, g_strerror(errno));
	exit(EXIT_FAILURE);
    }

    if (sary_searcher_multi_search(searcher, 
                                   patterns->patterns, 
                                   patterns->npatterns) == TRUE) {
        gchar *line;
	sary_searcher_sort_occurrences(searcher);
	while ((line = sary_searcher_get_next_line(searcher))) {
            /*
             * Use printf instead of g_print to avoid "[Invalid UTF-8]"
             */
	    printf("%s", line);
	}
    }
    sary_searcher_destroy(searcher);
}

static void
show_usage (void)
{
    g_print("Usage: multi-test <pattern-file> <file>\n");
}

