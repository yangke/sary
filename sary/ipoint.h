#ifndef __SARY_IPOINT_H__
#define __SARY_IPOINT_H__

#include <glib.h>
#include <sary/text.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef gchar* 	(*SaryIpointFunc)		(SaryText *text);

gchar*		sary_ipoint_bytestream		(SaryText *text);
gchar*		sary_ipoint_char_ascii		(SaryText *text);
gchar*		sary_ipoint_char_iso8859	(SaryText *text);
gchar*		sary_ipoint_char_eucjp		(SaryText *text);
gchar*		sary_ipoint_char_sjis		(SaryText *text);
gchar*		sary_ipoint_char_utf8		(SaryText *text);
gchar*		sary_ipoint_locale		(SaryText *text);
gchar*		sary_ipoint_line		(SaryText *text);
gchar*		sary_ipoint_word		(SaryText *text);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_IPOINT_H__ */
