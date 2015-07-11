/* 
 * sary - a suffix array library
 *
 * $Id: mksary.c,v 1.1.1.1 2004/06/11 18:57:28 satoru-t Exp $
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
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <locale.h>
#include <sary.h>
#include "getopt.h"

typedef struct {
    gchar 		*codeset;
    SaryIpointFunc	ipoint_func;
} CodesetFunc;

typedef void		(*ProcessFunc)	(SaryBuilder *builder,
					 const gchar *file_name, 
					 const gchar *array_name);
typedef gboolean	(*SortFunc)	(SaryBuilder *builder);

CodesetFunc codeset_func_tab[] = {
    { "bytestream",  	sary_ipoint_bytestream   },
    { "ascii",  	sary_ipoint_char_ascii   },
    { "iso8859",  	sary_ipoint_char_iso8859 },
    { "iso-8859",  	sary_ipoint_char_iso8859 },
    { "EUC-JP", 	sary_ipoint_char_eucjp   },
    { "eucJP",	 	sary_ipoint_char_eucjp   },
    { "Shift_JIS",	sary_ipoint_char_sjis    },
    { "SJIS",		sary_ipoint_char_sjis    },
    { "UTF-8",		sary_ipoint_char_utf8    },
    { NULL, 		NULL },
};

static SaryIpointFunc	dispatch_codeset_func	(const gchar *codeset);
static SaryBuilder*	new_builder		(const gchar *file_name, 
						 const gchar *array_name);
static void		index			(SaryBuilder *builder,
						 const gchar *file_name,
						 const gchar *array_name);
static void		sort			(SaryBuilder *builder,
						 const gchar *file_name,
						 const gchar *array_name);
static void		index_and_sort		(SaryBuilder *builder,
						 const gchar *file_name,
						 const gchar *array_name);
static void		print_time		(SaryProgress *progress, 
						 time_t t);
static void		print_eta		(SaryProgress *progress);
static void		print_elapsed		(SaryProgress *progress);
static void		progress_bar		(SaryProgress *progress);
static void		progress_quiet		(SaryProgress *progress);
static void		parse_options		(int argc, char **argv);
static void		show_help		(void);
static void		show_mini_help		(void);
static void		show_version		(void);
static SaryInt		ck_atoi			(gchar const *str, 
						 gint *out);

static SaryIpointFunc	ipoint_func   = sary_ipoint_char_ascii;
static SaryProgressFunc progress_func = progress_bar;
static ProcessFunc 	process       = index_and_sort;
static SortFunc		sort_func     = sary_builder_sort;
static gchar*		array_name    = NULL;
static SaryInt		block_size    = 4 * 1024 * 1024; /* 4 MB */
static SaryInt		nthreads      = 1;

int
main (int argc, char **argv)
{
    gchar *file_name;
    SaryBuilder *builder;

    parse_options(argc, argv);

    if (optind + 1 != argc) {
	show_mini_help();
    }

    file_name  = argv[optind];
    if (array_name == NULL) {
	array_name = g_strconcat(file_name, ".ary", NULL);
    }

    builder = new_builder(file_name, array_name);
    process(builder, file_name, array_name);

    sary_builder_destroy(builder);
    g_free(array_name);

    return 0;
}

static SaryIpointFunc
dispatch_codeset_func (const gchar *codeset)
{
    CodesetFunc *cursor;

    for (cursor = codeset_func_tab; cursor->codeset != NULL; cursor++) {
	if (g_strcasecmp(cursor->codeset, codeset) == 0) {
	    return cursor->ipoint_func;
	}
    }
    g_warning("invalid codeset: %s", codeset);
    return sary_ipoint_char_ascii;
}

static SaryBuilder *
new_builder (const gchar *file_name, const gchar *array_name)
{
    SaryBuilder *builder;

    builder = sary_builder_new2(file_name, array_name);
    if (builder == NULL) {
	g_printerr("mksary: %s, %s: %s\n", file_name, array_name, 
		   g_strerror(errno));
	exit(EXIT_FAILURE);
    }

    sary_builder_set_block_size(builder, block_size);
    sary_builder_set_nthreads(builder, nthreads);
    sary_builder_set_ipoint_func(builder, ipoint_func);
    sary_builder_connect_progress(builder, progress_func, NULL);
    return builder;
}

static void
index (SaryBuilder *builder,
       const gchar *file_name, 
       const gchar *array_name)
{
    SaryInt ipoints;

    ipoints = sary_builder_index(builder);
    if (ipoints == -1) {
	g_printerr("mksary: %s, %s: %s\n", file_name, array_name, 
		   g_strerror(errno));
	exit(EXIT_FAILURE);
    }
}


static void
sort (SaryBuilder *builder,
      const gchar *file_name, 
      const gchar *array_name)
{
    gboolean status;

    status = sort_func(builder);

    if (status == FALSE) {
	g_printerr("mksary: %s, %s: %s\n", file_name, array_name,
		   g_strerror(errno));
	exit(EXIT_FAILURE);
    }
}


static void
index_and_sort (SaryBuilder *builder,
		const gchar *file_name, 
		const gchar *array_name)
{
    index(builder, file_name, array_name);
    sort(builder, file_name, array_name);
}

static void
print_time (SaryProgress *progress, time_t t)
{
    gint sec, min, hour;

    sec  = t % 60;
    min  = (t / 60) % 60;
    hour = t / 3600;

    g_print("%02d:%02d:%02d", hour, min, sec);
}


/*
 * ETA stands for Estimated Time of Arrival.
 */
static void
print_eta (SaryProgress *progress)
{
    time_t eta;
    gint current   = progress->current;
    gint total     = progress->total;
    time_t elapsed = time(NULL) - progress->start_time;

    if (progress->is_finished) {
	g_print("00:00:00");
	return;
    } else if (progress->current == 0) {
	g_print("--:--:--");
	return;
    }
    /*
     * Adding one for avoiding "division by zero" error.
     */
    current++;
    total++;

    /* 
     * Cast to gdouble for avoding an overflow. 
     */
    eta  = (gdouble)elapsed * total / current - elapsed;

    g_print("ETA:  ");
    print_time(progress, eta);
}

static void
print_elapsed (SaryProgress *progress)
{
    time_t elapsed = time(NULL) - progress->start_time;

    g_print("Time: ");
    print_time(progress, elapsed);
}

static void
progress_bar (SaryProgress *progress)
{
    static char bar[] = "ooooooooooooooooooooooooooooooooooooooooo";
    static gint scale = sizeof(bar) - 1;
    gint cur_percentage, prev_percentage, bar_len;

    /*
     * Adding one for avoiding "division by zero" error.
     */
    gint current  = progress->current  + 1;
    gint previous = progress->previous + 1;
    gint total    = progress->total    + 1;

    cur_percentage  = (gint)((gdouble)current  * 100 / total);
    prev_percentage = (gint)((gdouble)previous * 100 / total);
    bar_len         = (gint)((gdouble)current  * scale / total);

    if (cur_percentage > prev_percentage || progress->is_finished) {
	  g_print("%s:	%3d%% |%.*s%*s| ", progress->task, 
		  cur_percentage, bar_len, bar, scale - bar_len, "");

	  if (progress->is_finished) {
	      print_elapsed(progress);
	      g_print("\n");
	  } else {
	      print_eta(progress);
	      g_print("\r");
	  }
	  fflush(stdout);
    }
}

static void
progress_quiet (SaryProgress *progress)
{
    /* do nothing */
}

static const char *short_options = "a:b::c:hilLqst:w";
static struct option long_options[] = {
    { "array",		required_argument,		NULL, 'a' },
    { "block",		optional_argument,		NULL, 'b' },
    { "encoding",	required_argument,		NULL, 'c' },
    { "help",		no_argument,			NULL, 'h' },
    { "index",		no_argument,			NULL, 'i' },
    { "line",		no_argument,			NULL, 'l' },
    { "locale",		no_argument,			NULL, 'L' },
    { "quiet",		no_argument,			NULL, 'q' },
    { "sort",		no_argument,			NULL, 's' },
    { "threads",	no_argument,			NULL, 't' },
    { "word",		no_argument,			NULL, 'w' },
    { "version",	no_argument,			NULL, 'v' },
    { NULL, 0, NULL, 0 }
};

static void
show_help (void)
{
    g_print("\
Usage: mksary [OPTION]... FILE\n\
  -a, --array=NAME       set the array file name to NAME\n\
  -b, --block=[SIZE]     sort block by block with SIZE [%d] KB block\n\
  -i, --index            assign index points and write them to an array file\n\
  -s, --sort             sort an array file\n\
  -l, --line             index every line\n\
  -w, --word             index every word delimited by white spaces\n\
  -c, --encoding=NAME    handle NAME encoding for indexing\n\
                         [bytestream], ASCII, ISO-8859,\n\
                         EUC-JP, Shift_JIS, UTF-8\n\
  -L, --locale           enable locale support (use mblen for indexing)\n\
  -t, --threads=NUM      set number of threads for block sorting to NUM\n\
  -q, --quiet            suppress all normal output\n\
  -v, --version          print version information and exit\n\
  -h, --help             display this help and exit\n\
", block_size / 1024);
    exit(EXIT_SUCCESS);

}


static void
parse_options (int argc, char **argv)
{
    while (1) {
        int ch = getopt_long(argc, argv, short_options, long_options, NULL);
        if (ch == EOF) {
            break;
	}
	switch (ch) {
	case 'a':
	    array_name = g_strdup(optarg);
	    break;
	case 'b':
	    sort_func  = sary_builder_block_sort;
	    if (optarg) {
		if (ck_atoi(optarg, &block_size)) {
		    g_printerr("mksary: invalid block size argument\n");
		    exit(EXIT_FAILURE);
		}
		block_size = block_size * 1024;
		if (block_size == 0) {
		    block_size = sizeof(SaryInt);  /* for test suites */
		}
	    }
	    break;
	case 'c':
	    ipoint_func = dispatch_codeset_func(optarg);
	    break;
	case 'h':
	    show_help();
	    break;
	case 'i':
	    process = index;
	    break;
	case 'l':
	    ipoint_func = sary_ipoint_line;
	    break;
	case 'L':
	    if (setlocale(LC_CTYPE, "") == NULL) {
		g_warning("Unable to set locale: %s\n", g_strerror(errno));
	    }
	    ipoint_func = sary_ipoint_locale;
	    break;
	case 'q':
	    progress_func = progress_quiet;
	    break;
	case 's':
	    process = sort;
	    break;
	case 't':
	    if (optarg) {
		if (ck_atoi(optarg, &nthreads)) {
		    g_printerr("mksary: invalid nthreads argument\n");
		    exit(EXIT_FAILURE);
		}
	    }
	    break;
	case 'w':
	    ipoint_func = sary_ipoint_word;
	    break;
	case 'v':
	    show_version();
	    break;
	}
    }
    if (nthreads > 1 && sort_func != sary_builder_block_sort) {
	g_print("mksary: -t option must be used with -b option.\n");
	exit(EXIT_FAILURE);
    }
}


static void 
show_mini_help(void)
{
    g_print("Usage: mksary [OPTION]... FILE\n");
    g_print("Try `mksary --help' for more information.\n");
    exit(EXIT_SUCCESS);
}

static void 
show_version(void)
{
    g_print("mksary %s\n", VERSION);
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

