/* 
 * sary - a suffix array library
 *
 * $Id: writer.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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
#include <sary.h>

enum { BUFSIZE = 1024 * 1024 / sizeof(SaryInt) };  /* 1MB */

struct _SaryWriter {
    FILE*	fp;
    SaryInt*	buf;
    SaryInt	buf_idx;
};

SaryWriter*
sary_writer_new (const gchar *file_name)
{
    SaryWriter *writer;

    g_assert(file_name != NULL);

    writer = g_new(SaryWriter, 1);
    writer->fp = fopen(file_name, "w");
    if (writer->fp == NULL) {
	return NULL;
    }

    writer->buf = g_new(SaryInt, BUFSIZE);
    writer->buf_idx = 0;

    return writer;
}

void
sary_writer_destroy (SaryWriter *writer)
{
    g_assert(writer->buf_idx == 0);

    fclose(writer->fp);
    g_free(writer->buf);
    g_free(writer);
}

gboolean
sary_writer_write (SaryWriter *writer, 
		   SaryInt data)
{
    writer->buf[writer->buf_idx] = data;
    writer->buf_idx++;

    if (writer->buf_idx == BUFSIZE) {
	if (sary_writer_flush(writer) == FALSE) {
	    return FALSE;
	}
    }
    return TRUE;
}

gboolean
sary_writer_flush (SaryWriter *writer)
{
    if (writer->buf_idx == 0) {
	return TRUE;
    }

    fwrite(writer->buf, sizeof(SaryInt), writer->buf_idx, writer->fp);
    if (ferror(writer->fp)) {
	fclose(writer->fp);
	return FALSE;
    }
    writer->buf_idx = 0;
    return TRUE;

}
