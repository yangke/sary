#ifndef __SARY_SORTER_H__
#define __SARY_SORTER_H__

#include <glib.h>
#include <sary/progress.h>
#include <sary/saryconfig.h>
#include <sary/text.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _SarySorter SarySorter;

SarySorter*	sary_sorter_new			(SaryText *text,
						 const gchar *array_name);
void		sary_sorter_destroy		(SarySorter *sorter);
gboolean	sary_sorter_sort		(SarySorter *sorter);
gboolean	sary_sorter_sort_blocks		(SarySorter *sorter, 
						 SaryInt block_size);
void		sary_sorter_merge_blocks	(SarySorter *sorter, 
						 const gchar *array_name);
void		sary_sorter_set_nthreads	(SarySorter *sorter,
						 SaryInt nthreads);

void		sary_sorter_connect_progress	(SarySorter *sorter,
						 SaryProgressFunc 
						 	progress_func,
						 gpointer progress_func_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_SORTER_H__ */
