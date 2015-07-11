#ifndef __SARY_BUILDER_H__
#define __SARY_BUILDER_H__

#include <glib.h>
#include <sary/ipoint.h>
#include <sary/progress.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Suffix Array Builder.
 */
typedef struct _SaryBuilder SaryBuilder;

SaryBuilder*	sary_builder_new		(const gchar *file_name);
SaryBuilder*	sary_builder_new2		(const gchar *file_name,
						 const gchar *array_name);
void		sary_builder_destroy		(SaryBuilder *builder);

void		sary_builder_set_ipoint_func	(SaryBuilder *builder,
						 SaryIpointFunc ipoint_func);
SaryInt		sary_builder_index		(SaryBuilder *builder);

gboolean	sary_builder_sort		(SaryBuilder *builder);
gboolean	sary_builder_block_sort		(SaryBuilder *builder);
void		sary_builder_set_block_size	(SaryBuilder *builder,
						 SaryInt block_size);
void		sary_builder_set_nthreads	(SaryBuilder *builder,
						 SaryInt nthreads);
void		sary_builder_connect_progress	(SaryBuilder *builder,
						 SaryProgressFunc 
						 	progress_func,
						 gpointer progress_func_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_BUILDER_H__ */
