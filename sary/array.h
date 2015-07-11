#ifndef __SARY_ARRAY_H__
#define __SARY_ARRAY_H__

#include <glib.h>
#include <sary/ipoint.h>
#include <sary/saryconfig.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

SaryInt		sary_array_index	(const gchar *file_name, 
					 SaryIpointFunc next_ipoint);
SaryInt		sary_array_index2	(const gchar *file_name, 
					 SaryIpointFunc next_ipoint,
					 const gchar *array_name);
gboolean	sary_array_sort		(const gchar *file_name);
gboolean	sary_array_sort2	(const gchar *file_name, 
					 const gchar *array_name);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_ARRAY_H__ */
