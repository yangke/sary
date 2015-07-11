#ifndef __SARY_STR_H__
#define __SARY_STR_H__

#include <glib.h>
#include <sary/saryconfig.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

gchar*		sary_str_seek_eol		(const gchar *cursor, 
						 const gchar *eof);
gchar*		sary_str_seek_bol		(const gchar *cursor, 
						 const gchar *bof);
gchar*		sary_str_get_region		(const gchar *cursor, 
						 const gchar *eof,
						 SaryInt len);
SaryInt		sary_str_get_linelen		(const gchar *cursor, 
						 const gchar *bof,
						 const gchar *eof);
gchar*		sary_str_get_line		(const gchar *cursor,
						 const gchar *bof,
						 const gchar *eof);
gchar*		sary_str_seek_lines_backward	(const gchar *cursor, 
						 const gchar *bof, 
						 SaryInt n);
gchar*		sary_str_seek_lines_forward	(const gchar *cursor, 
						 const gchar *eof, 
						 SaryInt n);
gchar*		sary_str_seek_pattern_backward	(const gchar *cursor, 
						 const gchar *bof, 
						 const gchar *pattern);
gchar*		sary_str_seek_pattern_forward	(const gchar *cursor, 
						 const gchar *eof, 
						 const gchar *pattern);
gchar*		sary_str_seek_pattern_backward2	(const gchar *cursor, 
						 const gchar *bof, 
						 const gchar *pattern,
						 SaryInt len);
gchar*		sary_str_seek_pattern_forward2	(const gchar *cursor, 
						 const gchar *eof, 
						 const gchar *pattern,
						 SaryInt len);
gchar*		sary_str_seek_backward		(const gchar *cursor, 
						 const gchar *bof, 
						 const gchar *charclass);
gchar*		sary_str_seek_forward		(const gchar *cursor, 
						 const gchar *eof, 
						 const gchar *charclass);
gchar*		sary_str_skip_backward		(const gchar *cursor, 
						 const gchar *bof, 
						 const gchar *charclass);
gchar*		sary_str_skip_forward		(const gchar *cursor, 
						 const gchar *eof, 
						 const gchar *charclass);
gchar*		sary_str_get_whitespaces	(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_STR_H__ */
