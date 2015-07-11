#ifndef __SARY_CACHE_H__
#define __SARY_CACHE_H__

#include <glib.h>
#include <sary/searcher.h>
#include <sary/saryconfig.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef GHashTable	SaryCache;

SaryCache*	sary_cache_new		(void);
void		sary_cache_destroy	(SaryCache *cache);
SaryResult*	sary_cache_get		(SaryCache *cache, 
					 const gchar *pattern, 
					 SaryInt len);
void		sary_cache_add		(SaryCache *cache, 
					 const gchar *pattern,
					 SaryInt len,
					 SaryInt *first,
					 SaryInt *last);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_CACHE_H__ */
