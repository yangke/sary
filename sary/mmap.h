#ifndef __SARY_MMAP_H__
#define __SARY_MMAP_H__

#include <glib.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
    size_t   len;
    gpointer map;
    gpointer data;
} SaryMmap;

SaryMmap* 	sary_mmap	(const gchar *file_name, const gchar *mode);
void		sary_munmap	(SaryMmap *mobj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_MMAP_H__ */
