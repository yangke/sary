#ifndef __SARY_MERGER_H__
#define __SARY_MERGER_H__

#include <glib.h>
#include <sary/saryconfig.h>
#include <sary/text.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _SaryMerger	SaryMerger;

SaryMerger*	sary_merger_new		(SaryText *text,
					 const gchar *array_name,
					 SaryInt nblocks);
void		sary_merger_destroy	(SaryMerger *merger);
void		sary_merger_add_block	(SaryMerger *merger,
					 SaryInt *head, 
					 SaryInt len);
gboolean	sary_merger_merge	(SaryMerger *merger, 
					 SaryProgressFunc progress_func,
					 gpointer progress_func_data,
					 SaryInt nipoints);
					 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_MERGER_H__ */
