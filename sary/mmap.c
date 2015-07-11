/* 
 * sary - a suffix array library
 *
 * $Id: mmap.c,v 1.1.1.1 2004/06/11 18:57:27 satoru-t Exp $
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
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>
#include <sary.h>

#if defined (_WIN32) && !defined (__CYGWIN__)
#  include <windows.h>
typedef struct {
    HANDLE hFile;
    HANDLE hMap;
} MmapHandles;
#else
#  include <sys/mman.h>
#  include <unistd.h>
#endif

/*
 * Windows support codes are imported from SUFARY's
 * lib/mmp.c and modfied.  Thanks to YAMASITA Tatuo
 * <yto@nais.to> for permission to use.
 */

#if defined (_WIN32) && !defined (__CYGWIN__)

/*
 * mode: "r" for reading only, "r+" for reading and writing.
 */
SaryMmap *
sary_mmap(const gchar *file_name, const gchar *mode)
{
    MmapHandles tmp;
    gulong mode1, mode2, mode3;
    SaryMmap *mobj;

    g_assert(file_name != NULL && mode != NULL);

    mobj = g_new(SaryMmap, 1);
    mobj->data = g_new(MmapHandles, 1);

    if (strcmp(mode, "r") == 0) {
	mode1 = GENERIC_READ;
	mode2 = PAGE_READONLY;
	mode3 = FILE_MAP_READ;
    } else if (strcmp(mode, "r+") == 0) {
	mode1 = GENERIC_READ | GENERIC_WRITE;
	mode2 = PAGE_READWRITE;
	mode3 = FILE_MAP_ALL_ACCESS;
    } else {
	g_assert_not_reached();
    }

    tmp.hFile = CreateFile(file_name, mode1, FILE_SHARE_READ,
			   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (tmp.hFile == INVALID_HANDLE_VALUE) {
	g_free(mobj);
        return NULL;
    }
    mobj->len = GetFileSize(tmp.hFile, NULL);

    tmp.hMap = CreateFileMapping(tmp.hFile, NULL, mode2, 0, 0, NULL);
    if (tmp.hMap == NULL) {
	CloseHandle(tmp.hFile);
	g_free(mobj);
	return NULL;
    }
				
    mobj->map = MapViewOfFile(tmp.hMap, mode3, 0, 0, 0);
    if (mobj->map == NULL) {
	CloseHandle(tmp.hFile);
	CloseHandle(tmp.hMap);
	g_free(mobj);
	return NULL;
    }

    ((MmapHandles *)(mobj->data))->hFile = tmp.hFile;
    ((MmapHandles *)(mobj->data))->hMap  = tmp.hMap;

    return mobj;
}

void
sary_munmap (SaryMmap *mobj)
{
    g_assert(mobj != NULL);

    UnmapViewOfFile(mobj->map);
    CloseHandle(((MmapHandles *)(mobj->data))->hMap);
    CloseHandle(((MmapHandles *)(mobj->data))->hFile);
    if (mobj->data != NULL) {
	g_free((MmapHandles *)(mobj->data));
    }

    g_free(mobj);
}

#else

/*
 * mode: "r" for reading only, "r+" for reading and writing.
 */
SaryMmap*
sary_mmap (const gchar *file_name, const gchar *mode)
{
    gint fd, flag = 0, prot = 0;
    struct stat stat;
    SaryMmap *mobj;

    g_assert(file_name != NULL && mode != NULL);

    mobj = g_new(SaryMmap, 1);

    if (strcmp(mode, "r") == 0) {
	flag = O_RDONLY;
	prot = PROT_READ;
    } else if (strcmp(mode, "r+") == 0) {
	flag = O_RDWR;
	prot = PROT_READ | PROT_WRITE;
    } else {
	g_assert_not_reached();
    }
    
    fd = open(file_name, flag);
    if (fd < 0) {
	g_free(mobj);
	return NULL;
    }
    if (fstat(fd, &stat) < 0) {
	g_free(mobj);
	return NULL;
    }

    mobj->len = stat.st_size;
    if (mobj->len == 0) {
	mobj->map = NULL;
    } else {
	mobj->map = mmap((void *)0, mobj->len, prot, MAP_SHARED, fd, 0);
	close(fd);
	if (mobj->map == MAP_FAILED) {
            return NULL;
	}
    }
    return mobj;
}

void
sary_munmap (SaryMmap *mobj)
{
    g_assert(mobj != NULL);

    munmap(mobj->map, mobj->len);
    g_free(mobj);
}

#endif

