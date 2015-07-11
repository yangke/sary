/* 
 * sary - a suffix array library
 *
 * $Id: search-benchmark.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sary.h>
#include "../src/getopt.h"

typedef gboolean 	(*SarySearcherSearchFunc)  (SarySearcher *searcher, 
                                                    const gchar *pattern, 
                                                    SaryInt len);
typedef void		(*SearchFunc)		(SarySearcher *searcher, 
						 const gchar *pattern, 
						 SarySearcherSearchFunc 
                                                 search_func);

static void	benchmark1		(const gchar *file_name, 
					 const gchar *pattern);
static void	benchmark2		(const gchar *file_name, 
					 const gchar *pattern);
static double	benchmark_iterate 	(SarySearcher *searcher, 
					 const gchar *pattern, 
					 SearchFunc search_func,
					 SarySearcherSearchFunc 
                                         sary_searcher_search_func,
					 gint n);
static void	incremental_search 	(SarySearcher *searcher, 
					 const gchar *pattern, 
					 SarySearcherSearchFunc 
                                         sary_searcher_search_func);
static void	normal_search	 	(SarySearcher *searcher, 
					 const gchar *pattern, 
					 SarySearcherSearchFunc 
                                         sary_searcher_search_func);
static SarySearcher*	new			(const gchar *file_name);
static void 	parse_options 		(int argc, char **argv);
static void 	show_usage		(void);

gint iterations = 1;

int
main (int argc, char **argv)
{
    gchar *file_name;
    gchar *pattern; 

    parse_options(argc, argv);
    if (optind + 2 != argc) {
	show_usage();
	exit(EXIT_FAILURE);
    }

    pattern   = argv[optind];
    file_name = argv[optind + 1];

    benchmark1(file_name, pattern);
    benchmark2(file_name, pattern);

    return 0;
}

static void
benchmark1 (const gchar *file_name, const gchar *pattern)
{
    double elapsed1, elapsed2, elapsed3, elapsed4;
    SarySearcher *searcher1 = new(file_name);
    SarySearcher *searcher2 = new(file_name);

    elapsed1 = benchmark_iterate(searcher1, pattern, normal_search, 
				 sary_searcher_search2, iterations);
    elapsed2 = benchmark_iterate(searcher2, pattern, normal_search, 
				 sary_searcher_icase_search2, iterations);

    sary_searcher_enable_cache(searcher1);
    sary_searcher_enable_cache(searcher2);

    elapsed3 = benchmark_iterate(searcher1, pattern, normal_search, 
				 sary_searcher_search2, iterations);
    elapsed4 = benchmark_iterate(searcher2, pattern, normal_search, 
				 sary_searcher_icase_search2, iterations);

    g_print("= Normal Search\n");
    g_print("  search2:      %5.2f (with cache: %5.2f)\n", 
	    elapsed1 / CLOCKS_PER_SEC, elapsed3 / CLOCKS_PER_SEC);
    g_print("  icase_search: %5.2f (with cache: %5.2f)\n", 
	    elapsed2 / CLOCKS_PER_SEC, elapsed4 / CLOCKS_PER_SEC);

    sary_searcher_destroy(searcher1);
    sary_searcher_destroy(searcher2);
}

static void
benchmark2 (const gchar *file_name, const gchar *pattern)
{
    double elapsed1, elapsed2, elapsed3, elapsed4;
    SarySearcher *searcher1 = new(file_name);
    SarySearcher *searcher2 = new(file_name);

    elapsed1 = benchmark_iterate(searcher1, pattern, incremental_search, 
				 sary_searcher_search2, iterations);
    elapsed2 = benchmark_iterate(searcher2, pattern, incremental_search, 
				 sary_searcher_isearch, iterations);

    sary_searcher_enable_cache(searcher1);
    sary_searcher_enable_cache(searcher2);

    elapsed3 = benchmark_iterate(searcher1, pattern, incremental_search, 
				 sary_searcher_search2, iterations);
    elapsed4 = benchmark_iterate(searcher2, pattern, incremental_search, 
				 sary_searcher_isearch, iterations);

    g_print("= Incremental Search\n");
    g_print("  search2:      %5.2f (with cache: %5.2f)\n", 
	    elapsed1 / CLOCKS_PER_SEC, elapsed3 / CLOCKS_PER_SEC);
    g_print("  isearch:      %5.2f (with cache: %5.2f)\n", 
	    elapsed2 / CLOCKS_PER_SEC, elapsed4 / CLOCKS_PER_SEC);

    sary_searcher_destroy(searcher1);
    sary_searcher_destroy(searcher2);
}

static double
benchmark_iterate (SarySearcher *searcher, 
		   const gchar *pattern, 
		   SearchFunc search_func,
		   SarySearcherSearchFunc sary_searcher_search_func,
		   gint n)
{
    gint i;
    clock_t start;

    start = clock();
    for (i = 0; i < n; i++) {
	search_func(searcher, pattern, sary_searcher_search_func);
    }
    return clock() - start;

}

static void
incremental_search (SarySearcher *searcher, 
		    const gchar *pattern, 
		    SarySearcherSearchFunc search_func)
{
    SaryInt i;
    SaryInt len = strlen(pattern);

    for (i = 1; i <= len; i++) {
	if (search_func(searcher, pattern, i) == FALSE) {
	    break;
	}
    }
    sary_searcher_isearch_reset(searcher);
}

static void
normal_search (SarySearcher *searcher, 
               const gchar *pattern, 
               SarySearcherSearchFunc search_func)
{
    SaryInt len = strlen(pattern);

    search_func(searcher, pattern, len);
}

static SarySearcher *
new (const gchar *file_name)
{
    SarySearcher *searcher = sary_searcher_new(file_name);

    if (searcher == NULL) {
	g_printerr("search-benchmark: %s(.ary): %s\n", 
		   file_name, g_strerror(errno));
	exit(EXIT_FAILURE);
    }
    return searcher;
}

static void
parse_options (int argc, char **argv)
{
    while (1) {
        int ch = getopt(argc, argv, "n:");
        if (ch == EOF) {
	    break;
	}
	switch (ch) {
	case 'n':
	    iterations = atoi(optarg);
            break;
	}
    }
}

static void
show_usage (void)
{
    g_print("Usage: search-benchmark [-n NUM] <pattern> <file>\n");
}

