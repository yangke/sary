/* 
 * sary - a suffix array library
 *
 * $Id: str.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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
#include <sary.h>

/*
 *  Utility functions for string handling.
 */

/*
 * Several functions are imported from SUFARY's lib/region.c
 * and modified.  Thanks to TAKAOKA Kazuma
 * <kazuma-t@is.aist-nara.ac.jp> for permission to use
 *
 * Seek the end-of-line and return its pointer. `eof' is the
 * sentinel for the end of file.
 *
 * Note: At first, we employ `strchr' to do the same
 * job. But Mr. TAKAOKA tells us it's dangerous to assume
 * zero-fills for out-of-range spaces.
 */
inline gchar *
sary_str_seek_eol (const gchar *cursor, const gchar *eof)
{
    g_assert(cursor <= eof);

    while (cursor < eof) {
	if (*cursor == '\n') { /* found */
	    return (gchar *)cursor + 1;
	}
	cursor++;
    }
    return (gchar *)eof;
}

inline gchar *
sary_str_seek_bol (const gchar *cursor, const gchar *bof)
{
    g_assert(cursor >= bof);

    while (bof < cursor) {
	cursor--;
	if (*cursor == '\n') { /* found */
	    return (gchar *)cursor + 1;
	}
    }
    return (gchar *)bof;
}


gchar *
sary_str_get_region (const gchar *cursor, const gchar *eof, SaryInt len)
{
    gchar *region;

    g_assert(len >= 0 && cursor + len <= eof);

    region = g_new(gchar, len + 1);
    g_memmove(region, cursor, len);
    region[len] = '\0';

    return region;
}


SaryInt
sary_str_get_linelen (const gchar *cursor, 
		      const gchar *bof,
		      const gchar *eof)
{
    gchar *bol, *eol;
    SaryInt len;

    if (cursor == eof) {
	return 0;
    }

    bol = sary_str_seek_bol(cursor, bof);
    eol = sary_str_seek_eol(cursor, eof);
    len = eol - bol;

    return len;
}

gchar *
sary_str_get_line (const gchar *cursor, 
		   const gchar *bof,
		   const gchar *eof)
{
    gchar *bol;
    SaryInt len;

    if (cursor == eof) {
	return NULL;
    }

    bol = sary_str_seek_bol(cursor, bof);
    len = sary_str_get_linelen(cursor, bof, eof);

    return sary_str_get_region(bol, eof, len);
}

gchar *
sary_str_seek_lines_backward (const gchar *cursor, 
			      const gchar *bof, 
			      SaryInt n)
{
    cursor = sary_str_seek_bol(cursor, bof);
    while (bof < cursor && n > 0) {
	cursor = sary_str_seek_bol(cursor - 1, bof);
	n--;
    }
    return (gchar *)cursor;
}

gchar *
sary_str_seek_lines_forward (const gchar *cursor, 
			     const gchar *eof, 
			     SaryInt n)
{
    cursor = sary_str_seek_eol(cursor, eof);
    while (eof > cursor && n > 0) {
	cursor = sary_str_seek_eol(cursor, eof);
	n--;
    }
    return (gchar *)cursor;
}

gchar *
sary_str_seek_pattern_backward (const gchar *cursor, 
				const gchar *bof, 
				const gchar *pattern)
{
    g_assert(cursor >= bof);

    return sary_str_seek_pattern_backward2(cursor, bof, pattern, 
					   strlen(pattern));
}

gchar *
sary_str_seek_pattern_forward (const gchar *cursor, 
			       const gchar *eof, 
			       const gchar *pattern)
{
    g_assert(cursor < eof);

    return sary_str_seek_pattern_forward2(cursor, eof, pattern, 
					  strlen(pattern));
}

gchar *
sary_str_seek_pattern_backward2 (const gchar *cursor, 
				 const gchar *bof, 
				 const gchar *pattern,
				 SaryInt len)
{
    g_assert(len >= 0 && cursor >= bof);

    len = strlen(pattern);
    while (bof < cursor) {
	if (memcmp(cursor, pattern, len) == 0)
	    return (gchar *)cursor;
	cursor--;
    }
    return (gchar *)bof;
}

gchar *
sary_str_seek_pattern_forward2 (const gchar *cursor, 
				const gchar *eof, 
				const gchar *pattern,
				SaryInt len)
{
    g_assert(len >= 0 && cursor < eof);

    while (cursor <= eof - len) {
	if (memcmp(cursor, pattern, len) == 0)
	    return (gchar *)cursor + len;
	cursor++;
    }
    return (gchar *)eof;
}

gchar *
sary_str_seek_backward (const gchar *cursor, 
			const gchar *bof, 
			const gchar *charclass)
{
    gint len;
    g_assert(cursor >= bof);
    len = strlen(charclass);

    while (bof < cursor) {
	cursor--;
	if (memchr(charclass, *cursor, len) != NULL) { /* found */
	    return (gchar *)cursor + 1;
	}
    }
    return (gchar *)bof;
}

gchar *
sary_str_seek_forward (const gchar *cursor, 
		       const gchar *eof, 
		       const gchar *charclass)
{
    gint len;
    g_assert(cursor <= eof);

    len = strlen(charclass);
    while (cursor < eof) {
	if (memchr(charclass, *cursor, len) != NULL) { /* found */
	    return (gchar *)cursor + 1;
	}
	cursor++;
    }
    return (gchar *)eof;
}

gchar *
sary_str_skip_backward (const gchar *cursor, 
			const gchar *bof, 
			const gchar *charclass)
{
    gint len;
    g_assert(cursor >= bof);

    len = strlen(charclass);
    while (bof < cursor) {
	cursor--;
	if (memchr(charclass, *cursor, len) == NULL) { /* not found */
	    return (gchar *)cursor;
	}
    }
    return (gchar *)bof;
}

gchar *
sary_str_skip_forward (const gchar *cursor, 
		       const gchar *eof, 
		       const gchar *charclass)
{
    gint len;
    g_assert(cursor <= eof);

    len = strlen(charclass);
    while (cursor < eof) {
	if (memchr(charclass, *cursor, len) == NULL) { /* not found */
	    return (gchar *)cursor;
	}
	cursor++;
    }
    return (gchar *)eof;
}

gchar *
sary_str_get_whitespaces (void)
{
    return " \f\n\r\t\v";  /* from man isspace */
}

