/* 
 * sary - a suffix array library
 *
 * $Id: ipoint.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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
 *  Functions for managing index points.
 */

gchar *
sary_ipoint_bytestream (SaryText *text)
{
    gchar *cursor;

    if (sary_text_is_eof(text)) {
	return NULL;
    }

    cursor = sary_text_get_cursor(text);
    sary_text_forward_cursor(text, 1);
    return cursor;
}

gchar *
sary_ipoint_char_ascii (SaryText *text)
{
    return sary_ipoint_bytestream(text);
}

gchar *
sary_ipoint_char_iso8859 (SaryText *text)
{
    return sary_ipoint_bytestream(text);
}

gchar *
sary_ipoint_char_eucjp (SaryText *text)
{
    gchar *cursor, *eof;
    gint len;

    if (sary_text_is_eof(text)) {
	return NULL;
    }

    cursor = sary_text_get_cursor(text);
    eof =    sary_text_get_eof(text);

    if ((guint)cursor[0] < '\x7f') {  
	/* ASCII */
	len = 1;
    } else if ((cursor + 1 < eof) &&
	       (guint)cursor[0] >= '\xa1' && (guint)cursor[0] <= '\xfe' &&
	       (guint)cursor[1] >= '\xa1' && (guint)cursor[1] <= '\xfe')
    {
	/* JIS X 0208 */
	len = 2;
    } else if ((cursor + 1 < eof) && (guint)cursor[0] ==  '\x8e' && 
	       (guint)cursor[1] >= '\xa1' && (guint)cursor[1] <= '\xdf')
    {
	/* JIS X 0201 */
	len = 2;
    } else if ((cursor + 2 < eof) && (guint)cursor[0] ==  '\x8f' && 
	       (guint)cursor[1] >= '\xa1' && (guint)cursor[1] <= '\xfe' &&
	       (guint)cursor[2] >= '\xa1' && (guint)cursor[2] <= '\xfe')
    {
	/* JIS X 0212 */
	len = 3;
    } else {
	/* invalid character */
	gchar *bof = sary_text_get_bof(text);
	g_warning("invalid character at %d", cursor - bof);
	len = 1;
    }

    sary_text_forward_cursor(text, len);
    return cursor;
}

gchar *
sary_ipoint_char_sjis (SaryText *text)
{
    gchar *cursor, *eof;
    gint len;

    if (sary_text_is_eof(text)) {
	return NULL;
    }

    cursor = sary_text_get_cursor(text);
    eof =    sary_text_get_eof(text);

    if ((guint)cursor[0] < '\x7f' || 
	((guint)cursor[0] >= '\xa1' && (guint)cursor[0] <= '\xdf'))
    {
	/* JIS X 0201 including Hankaku-Kana */
	len = 1;
    } else if ((cursor + 1 < eof) &&
	       (((guint)cursor[0] >= '\x81' && (guint)cursor[0] <= '\x9f') ||
		((guint)cursor[0] >= '\xe0' && (guint)cursor[0] <= '\xef')) &&
	       (((guint)cursor[1] >= '\x40' && (guint)cursor[1] <= '\x7e') ||
		((guint)cursor[1] >= '\x80' && (guint)cursor[1] <= '\xfc')))
    {
	/* JIS X 0208 */
	len = 2;
    } else {
	/* invalid character */
	gchar *bof = sary_text_get_bof(text);
	g_warning("invalid character at %d", cursor - bof);
	len = 1;
    }

    sary_text_forward_cursor(text, len);
    return cursor;
}

/*
 * Excerpt from RFC 2044:
 *
 * UCS-4 range (hex.)    UTF-8 octet sequence (binary)
 * 0000 0000-0000 007F   0xxxxxxx
 * 0000 0080-0000 07FF   110xxxxx 10xxxxxx
 * 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx
 * 
 * 0001 0000-001F FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 0020 0000-03FF FFFF   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 0400 0000-7FFF FFFF   1111110x 10xxxxxx ... 10xxxxxx
 */

gchar *
sary_ipoint_char_utf8 (SaryText *text)
{
    gchar *cursor, *eof;
    gint len;

    if (sary_text_is_eof(text)) {
	return NULL;
    }

    cursor = sary_text_get_cursor(text);
    eof =    sary_text_get_eof(text);

    if ((guint)cursor[0] < '\x80') {
	len = 1;
    } else if ((cursor + 1 < eof) && (cursor[0] & 0xe0) == 0xc0) {
	len = 2;
    } else if ((cursor + 2 < eof) && (cursor[0] & 0xf0) == 0xe0) {
	len = 3;
    } else if ((cursor + 3 < eof) && (cursor[0] & 0xf8) == 0xf0) {
	len = 4;
    } else if ((cursor + 4 < eof) && (cursor[0] & 0xfc) == 0xf8) {
	len = 5;
    } else if ((cursor + 5 < eof) && (cursor[0] & 0xfe) == 0xfc) {
	len = 6;
    } else {
	/* invalid character */
	gchar *bof = sary_text_get_bof(text);
	g_warning("invalid character at %d", cursor - bof);
	len = 1;
    }

    sary_text_forward_cursor(text, len);
    return cursor;
}

gchar *
sary_ipoint_locale (SaryText *text)
{
    gchar *cursor, *eof;
    SaryInt maxlen, len;

    if (sary_text_is_eof(text)) {
	return NULL;
    }

    eof    = sary_text_get_eof(text);
    cursor = sary_text_get_cursor(text);
    maxlen = eof - cursor;

    len    = mblen(cursor, maxlen);
    if (len == -1) {
	/* invalid character */
	gchar *bof = sary_text_get_bof(text);
	g_warning("invalid character at %d", cursor - bof);
	len = 1;
    }

    sary_text_forward_cursor(text, len);
    return cursor;
}

gchar *
sary_ipoint_line (SaryText *text)
{
    gchar *cursor;

    if (sary_text_is_eof(text)) {
	return NULL;
    }

    cursor = sary_text_get_cursor(text);
    sary_text_goto_next_line(text);
    return cursor;
}

gchar *
sary_ipoint_word (SaryText *text)
{
    gchar *cursor, *eof;

    if (sary_text_is_eof(text)) {
	return NULL;
    }

    cursor = sary_text_get_cursor(text);
    eof =    sary_text_get_eof(text);

    if (cursor == sary_text_get_bof(text)) { /* the first time */
	cursor = sary_str_skip_forward(cursor, eof, 
				       sary_str_get_whitespaces());
	sary_text_set_cursor(text, cursor);
    }

    sary_text_goto_next_word(text);
    return cursor;
}

