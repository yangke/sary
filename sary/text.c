/* 
 * sary - a suffix array library
 *
 * $Id: text.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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

/*
 * Class for `mmap'ed text files.
 */

#include "config.h"
#include <string.h>
#include <glib.h>
#include <sary.h>

SaryText *
sary_text_new (const gchar *file_name)
{
    SaryText *text;
    SaryMmap *mobj;

    g_assert(file_name != NULL);

    mobj = sary_mmap(file_name, "r");
    if (mobj == NULL) {
	return NULL;
    }

    /*
     * zero-length (empty) text file can be handled. In that
     * case, text->{bol,eof,cursor} are NULL. Be careful!
     */
    text = g_new(SaryText, 1);
    text->mobj      = mobj;
    text->bof       = (gchar *)mobj->map;
    text->eof       = (gchar *)mobj->map + mobj->len;  /* sentinel */
    text->cursor    = (gchar *)mobj->map;
    text->lineno    = 1;  /* 1-origin */
    text->file_name = g_strdup(file_name);

    return text;
}

void
sary_text_destroy (SaryText *text)
{
    sary_munmap(text->mobj);
    g_free(text->file_name);
    g_free(text);
}

SaryInt
sary_text_get_size (SaryText *text)
{
    return text->eof - text->bof;
}

SaryInt
sary_text_get_lineno (SaryText *text)
{
    return text->lineno;
}

void
sary_text_set_lineno (SaryText *text, SaryInt lineno)
{
    text->lineno = lineno;
}

SaryInt
sary_text_get_linelen (SaryText *text)
{
    return sary_str_get_linelen(text->cursor, text->bof, text->eof);
}

gchar *
sary_text_get_line (SaryText *text)
{
    return sary_str_get_line(text->cursor, text->bof, text->eof);
}

gchar *
sary_text_get_region (SaryText *text, SaryInt len)
{
    return sary_str_get_region(text->cursor, text->eof, len);
}

gboolean
sary_text_is_eof (SaryText *text)
{
    if (text->cursor == text->eof) {
	return TRUE;
    } else {
	return FALSE;
    }
}

gchar *
sary_text_get_cursor (SaryText *text)
{
    return text->cursor;
}

void
sary_text_set_cursor (SaryText *text, gchar *cursor)
{
    text->cursor = cursor;
}

gchar *
sary_text_goto_bol (SaryText *text)
{
    text->cursor = sary_str_seek_bol(text->cursor, text->bof);
    return text->cursor;
}

gchar *
sary_text_goto_eol (SaryText *text)
{
    text->cursor = sary_str_seek_eol(text->cursor, text->eof);
    return text->cursor;
}

gchar *
sary_text_goto_next_line (SaryText *text)
{
    text->cursor = sary_str_seek_eol(text->cursor, text->eof);
    g_assert(text->cursor <= text->eof);
    text->lineno++;
    return text->cursor;
}

gchar *
sary_text_goto_next_word (SaryText *text)
{
    text->cursor = sary_str_seek_forward(text->cursor, 
					 text->eof, 
					 sary_str_get_whitespaces());
    text->cursor = sary_str_skip_forward(text->cursor, 
					 text->eof,
					 sary_str_get_whitespaces());
    return text->cursor;
}

gchar *
sary_text_forward_cursor (SaryText *text, SaryInt len)
{
    g_assert(len >= 0);

    text->cursor += len;
    if (text->cursor > text->eof) { /* overrun */
	text->cursor = text->eof;
    }
    return text->cursor;
}

gchar *
sary_text_backward_cursor (SaryText *text, SaryInt len)
{
    g_assert(len >= 0);

    text->cursor -= len;
    if (text->cursor < text->bof) { /* overrun */
	text->cursor = text->bof;
    }
    return text->cursor;
}

