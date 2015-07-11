/* 
 * sary - a suffix array library
 *
 * $Id: sary.c,v 1.1.1.1 2004/06/11 18:57:28 satoru-t Exp $
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <ctype.h>
#include <locale.h>
#include <sary.h>
#include "getopt.h"

typedef int	 	(*StrncmpFunc)	(const gchar *s1, 
                                         const gchar *s2, 
                                         size_t n);
typedef gboolean 	(*SearchFunc)	(SarySearcher *searcher, 
                                         const gchar *pattern);
typedef void 		(*PrintFunc)	(const gchar *text,
                                         int len,
                                         const gchar *pattern);
typedef void 		(*GrepFunc)	(SarySearcher *searcher, 
                                         const gchar *pattern);
typedef gchar* 		(*NextFunc)	(SarySearcher *searcher, SaryInt *len);
typedef void		(*SortFunc)	(SarySearcher *searcher);

static void	init_locale		(void);
static void	configure 		(const gchar *mode);
static void	print_highlight_internal(const gchar *text, 
                                         int len, 
                                         const gchar *pattern, 
                                         StrncmpFunc cmpfunc);
static void	print_highlight		(const gchar *text, 
                                         int len,
                                         const gchar *pattern);
static void	print_highlight_icase	(const gchar *text,
                                         int len, 
                                         const gchar *pattern);
static void	print_normal		(const gchar *text, 
                                         int len,
                                         const gchar *pattern);

static void	grep			(const gchar *file_name, 
					 const gchar *array_name, 
					 const gchar *pattern);
static void	grep_count		(SarySearcher *searcher, 
                                         const gchar *pattern);
static void	grep_normal		(SarySearcher *searcher, 
                                         const gchar *pattern);
static gchar*	get_next_line		(SarySearcher *searcher, SaryInt *len);
static gchar*	get_next_context	(SarySearcher *searcher, SaryInt *len);
static gchar*	get_next_region		(SarySearcher *searcher, SaryInt *len);
static void	show_help		(void);
static void	show_mini_help		(void);
static void	show_version		(void);
static void	parse_options		(int argc, char **argv);
static SaryInt	ck_atoi			(gchar const *str, gint *out);
static void	sort_lexicographical	(SarySearcher *searcher);

static struct grep {
    gchar*	mode;
    GrepFunc	grep_func;
    NextFunc	next_func;
    gchar*	separator;
    gchar*	separator2;
} grep_tab[] = {
    { "count",  	grep_count,	NULL,			NULL,	NULL },
    { "line",		grep_normal,	get_next_line,		NULL,	NULL },
    { "context",	grep_normal,	get_next_context,	"--\n",	"" },
    { "tagged",		grep_normal,	get_next_region,	"--\n", "\n" },
    { NULL, 		NULL,		NULL,			NULL,	NULL },
};


static PrintFunc	do_print   = print_normal;
static GrepFunc		do_grep    = NULL;
static NextFunc 	get_next   = NULL;
static SearchFunc	search     = sary_searcher_search;
static SortFunc		sort	   = sary_searcher_sort_occurrences;

static gchar*	grep_mode   = "line";
static gchar    *separator  = NULL;
static gchar    *separator2 = NULL;

static SaryInt 	backward  = 0;
static SaryInt 	forward   = 0;
static gchar*	start_tag = NULL;
static gchar*	end_tag   = NULL;
static gchar*	array_name  = NULL;

int 
main (int argc, char **argv)
{
    gchar *file_name, *pattern;

    init_locale();
    parse_options(argc, argv);

    if (optind + 2 != argc) {
	show_mini_help();
    }

    pattern   = argv[optind];
    file_name = argv[optind + 1];

    if (array_name == NULL) {
	array_name = g_strconcat(file_name, ".ary", NULL);
    }

    configure(grep_mode);
    grep(file_name, array_name, pattern);

    g_free(array_name);

    return 0;
}

static void
init_locale (void)
{
    if (setlocale(LC_CTYPE, "") == NULL) {
	g_warning("Unable to set locale: %s\n", g_strerror(errno));
    }
}

static void
configure (const gchar *mode)
{
    struct grep *cursor;

    for (cursor = grep_tab; cursor->mode != NULL; cursor++) {
	if (g_strcasecmp(cursor->mode, mode) == 0) {
	    do_grep    = cursor->grep_func;
	    get_next   = cursor->next_func;
	    separator  = cursor->separator;
	    separator2 = cursor->separator2;
	    return;
	}
    }
    g_assert_not_reached();
}


static void
grep (const gchar *file_name, 
      const gchar *array_name, 
      const gchar *pattern)
{
    SarySearcher *searcher;

    searcher = sary_searcher_new2(file_name, array_name);
    if (searcher == NULL) {
	g_printerr("searcher: %s, %s: %s\n", file_name, array_name,
		   g_strerror(errno));
	exit(1);
    }

    do_grep(searcher, pattern);

    sary_searcher_destroy(searcher);
}


static void
grep_count (SarySearcher *searcher, const gchar *pattern)
{
    if (search(searcher, pattern)) {
	g_print("%d\n", sary_searcher_count_occurrences(searcher));
    } else {
	g_print("0\n");
    }
}

static void
print_highlight_internal (const gchar *text, int len, const gchar *pattern, 
                          StrncmpFunc cmpfunc)
{
    int i, patlen;
    patlen = strlen(pattern);
    for (i = 0; i < len;) {
        if (cmpfunc(text + i, pattern, patlen) == 0) {
            printf("\x1b[7m");
            fwrite(text + i, sizeof(gchar), patlen, stdout);
            printf("\x1b[0m");
            i += patlen;
        } else {
            fwrite(text + i, sizeof(gchar), 1, stdout);
            i++;
        }
    }
}

static void
print_highlight (const gchar *text, int len, const gchar *pattern)
{
    print_highlight_internal(text, len, pattern, strncmp);
}

static void
print_highlight_icase (const gchar *text, int len, const gchar *pattern)
{
    print_highlight_internal(text, len, pattern, g_strncasecmp);
}

static void
print_normal (const gchar *text, int len, const gchar *pattern)
{
    /*
     * Use fwrite for handling binary files.
     */
    fwrite(text, sizeof(gchar), len, stdout) ;
}

static void
grep_normal (SarySearcher *searcher, const gchar *pattern)
{
    if (search(searcher, pattern)) {
	SaryInt len, i = 0;
	gchar *sep = NULL, *sep2 = NULL;
	gchar *text;

	sort(searcher);
	while ((text = get_next(searcher, &len))) {
	    if (sep2) g_print("%s", sep2);
	    if (sep)  g_print("%s", sep);

            do_print(text, len, pattern);

	    sep  = separator;
	    sep2 = separator2;
	    i++;
	}
	if (i > 1) {
	    if (sep2) g_print("%s", sep2);
	}
    }
}    

static gchar *
get_next_line (SarySearcher *searcher, SaryInt *len)
{
    return sary_searcher_get_next_line2(searcher, len);
}

static gchar *
get_next_context (SarySearcher *searcher, SaryInt *len)
{
    return sary_searcher_get_next_context_lines2(searcher, 
                                                 backward, forward, len);
}

static gchar *
get_next_region (SarySearcher *searcher, SaryInt *len)
{
    return sary_searcher_get_next_tagged_region2(searcher, 
                                                 start_tag, strlen(start_tag),
                                                 end_tag, strlen(end_tag), 
                                                 len);
}

static void
sort_lexicographical (SarySearcher *searcher)
{
    /* do nothing */
}


static const char *short_options = "a:ce:hils:vA:B:C::p";
static struct option long_options[] = {
    { "array",			required_argument,	NULL, 'a' },
    { "count",			no_argument,		NULL, 'c' },
    { "end",			required_argument,	NULL, 'e' },
    { "help",			no_argument,		NULL, 'h' },
    { "ignore-case",		no_argument,		NULL, 'i' },
    { "lexicographical",	no_argument,		NULL, 'l' },
    { "start",			required_argument,	NULL, 's' },
    { "version",		no_argument,		NULL, 'v' },
    { "after-context",		required_argument,	NULL, 'A' },
    { "before-context",		no_argument,		NULL, 'B' },
    { "context",		optional_argument,	NULL, 'C' },
    { "highlight",		no_argument,		NULL, 'p' },
    { NULL, 0, NULL, 0 }
};

static void
show_help (void)
{
    g_print("\
Usage: sary [OPTION]... PATTERN FILE\n\
  -c, --count               only print the number of occurrences\n\
  -i, --ignore-case         ignore case distinctions\n\
  -l, --lexicographical     sort in lexicographical order\n\
  -A, --after-context=NUM   print NUM lines of trailing context\n\
  -B, --before-context=NUM  print NUM lines of leading context\n\
  -C, --context=[NUM]       print NUM (default 2) lines of output context\n\
  -s, --start=TAG           print tagged region. set start tag to TAG \n\
  -e, --end=TAG             print tagged region. set end tag to TAG\n\
  -p, --highlight           highlight the pattern in search results\n\
  -h, --help                display this help and exit\n\
");
    exit(EXIT_SUCCESS);
}

static void 
show_mini_help(void)
{
    g_print("Usage: sary [OPTION]... PATTERN FILE\n");
    g_print("Try `sary --help' for more information.\n");
    exit(EXIT_SUCCESS);
}

static void 
show_version(void)
{
    g_print("sary %s\n", VERSION);
    g_print("%s\n", COPYRIGHT);
    g_print("\
This is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU Lesser General Public License as\n\
published by the Free Software Foundation; either version 2.1,\n\
or (at your option) any later version.\n\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty\n\
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU Lesser General Public License for more details.\n\
");
    exit(EXIT_SUCCESS);
}

static void
parse_options (int argc, char **argv)
{
    int highlight_p = 0, icase_p = 0;

    while (1) {
        int ch = getopt_long(argc, argv, short_options, long_options, NULL);
        if (ch == EOF) {
            break;
	}
	switch (ch) {
	case 'a':
	    array_name = g_strdup(optarg);
	    break;
	case 'c':
	    grep_mode = "count";
	    break;
	case 'i':
            icase_p = 1;
	    search = sary_searcher_icase_search;
	    break;
	case 'l':
	    sort = sort_lexicographical;
	    break;
	case 'A':
	    grep_mode = "context";
	    if (ck_atoi(optarg, &forward)) {
		g_printerr("sary: invalid context length argument\n");
		exit(EXIT_FAILURE);
	    }
	    break;
	case 'B':
	    grep_mode = "context";
	    if (ck_atoi(optarg, &backward)) {
		g_printerr("sary: invalid context length argument\n");
		exit(EXIT_FAILURE);
	    }
	    break;
	case 'C':
	    grep_mode = "context";
	    if (optarg) {
		if (ck_atoi(optarg, &backward)) {
		    g_printerr("sary: invalid context length argument\n");
		    exit(EXIT_FAILURE);
		}
		forward  = backward;
	    } else {
		backward = 2;
		forward  = 2;
	    }
	    break;
	case 's':
	    grep_mode = "tagged";
	    start_tag = optarg;
	    break;
	case 'e':
	    grep_mode = "tagged";
	    end_tag   = optarg;
	    break;
	case 'p':
            highlight_p = 1;
	    break;
	case 'h':
	    show_help();
	    break;
	case 'v':
	    show_version();
	    break;
	}
    }

    if (highlight_p) {
        if (icase_p) {
            do_print = print_highlight_icase;
        } else {
            do_print = print_highlight;
        }            
    }

    if (start_tag == NULL && end_tag != NULL) {
	g_print("sary: start_tag must be specified with -s option.\n");
	exit(EXIT_FAILURE);
    }
    if (start_tag != NULL && end_tag == NULL)  {
	g_print("sary: end_tag must be specified with -e option.\n");
	exit(EXIT_FAILURE);
    }
}

/* 
 * Imported from GNU grep-2.3 and modified.
 *
 * Convert STR to a positive integer, storing the result in
 * *OUT.  If STR is not a valid integer, return -1
 * (otherwise 0).
 */
static SaryInt
ck_atoi (gchar const *str, gint *out)
{
    gchar const *p;
    for (p = str; *p; p++) {
	if (!isdigit(*p)) {
	    return -1;
	}
    }
    *out = atoi (optarg);
    return 0;
}

