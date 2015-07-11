#ifndef __SARY_BSEARCH_H__
#define __SARY_BSEARCH_H__

#include <glib.h>
#include <sary/saryconfig.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

gpointer sary_bsearch_first	(gconstpointer key, 
				 gconstpointer base, 
				 gsize len,
				 gsize elt_size, 
				 SaryInt *next_low,
				 SaryInt *next_high,
				 GCompareFunc compare_func);

gpointer sary_bsearch_last	(gconstpointer key, 
				 gconstpointer base, 
				 gsize len,
				 gsize elt_size, 
				 SaryInt prev_low,
				 SaryInt prev_high,
				 GCompareFunc compare_func);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_BSEARCH_H__ */
