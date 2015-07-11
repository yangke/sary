/* 
 * sary - a suffix array library
 *
 * $Id: progress.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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
#include <time.h>
#include <glib.h>
#include <sary.h>

static void	do_nothing	(SaryProgress *progress);

SaryProgress *
sary_progress_new (const gchar *task, SaryInt total)
{
    SaryProgress *progress;

    g_assert(total >=0 && task != NULL);

    progress= g_new(SaryProgress, 1);
    progress->current  = 0;
    progress->previous = 0;
    progress->total    = total;
    progress->task     = g_strdup(task);
    progress->func     = do_nothing;
    progress->func_data   = NULL;
    progress->is_finished = FALSE;
    progress->start_processer_time =  clock();
    progress->start_time = time(NULL);

    sary_progress_set_count(progress, 0);
    return progress;
}

void
sary_progress_connect (SaryProgress *progress,
		       SaryProgressFunc func,
		       gpointer func_data)
{
    if (func != NULL) {
	progress->func = func;
	progress->func_data = func_data;
    }
}

void
sary_progress_destroy (SaryProgress *progress)
{
    g_assert(progress->is_finished == FALSE);

    progress->current     = progress->total;
    progress->is_finished = TRUE;
    progress->func(progress);

    g_free(progress->task);
    g_free(progress);
}

void
sary_progress_set_count (SaryProgress *progress, SaryInt count)
{
    g_assert(count >= progress->previous);

    progress->current = count;
    progress->func(progress);
    progress->previous = count;
}

static void
do_nothing (SaryProgress *progress)
{
    /* do nothing */
}
