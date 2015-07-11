#ifndef __SARY_TEXT_H__
#define __SARY_TEXT_H__

#include <glib.h>
#include <sary/mmap.h>
#include <sary/saryconfig.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct  {
    SaryMmap *mobj;
    SaryInt  lineno;   /* current line number */
    gchar    *bof;     /* beginning of file */
    gchar    *eof;     /* end of file */
    gchar    *cursor;
    gchar    *file_name;
} SaryText;


SaryText*	sary_text_new			(const gchar *file_name);
void		sary_text_destroy		(SaryText *text);
SaryInt		sary_text_get_size		(SaryText *text);
SaryInt		sary_text_get_lineno		(SaryText *text);
void		sary_text_set_lineno		(SaryText *text, 
						 SaryInt lineno);
SaryInt		sary_text_get_linelen		(SaryText *text);
gchar*		sary_text_get_line		(SaryText *text);
gchar*		sary_text_get_region		(SaryText *cursor, 
						 SaryInt len);
gboolean	sary_text_is_eof		(SaryText *text);
gchar*		sary_text_get_cursor		(SaryText *text);
void		sary_text_set_cursor		(SaryText *text, 
						 gchar *cursor);
gchar*		sary_text_goto_bol		(SaryText *text);
gchar*		sary_text_goto_eol		(SaryText *text);
gchar*		sary_text_goto_next_line	(SaryText *text);
gchar*		sary_text_goto_next_word	(SaryText *text);
gchar*		sary_text_forward_cursor	(SaryText *text, 
						 SaryInt len);
gchar*		sary_text_backward_cursor	(SaryText *text, 
						 SaryInt len);

#define		sary_text_get_bof(text)		(text)->bof
#define		sary_text_get_eof(text)		(text)->eof

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_TEXT_H__ */
