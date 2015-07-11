#ifndef __SARY_SEARCHER_H__
#define __SARY_SEARCHER_H__

#include <glib.h>
#include <sary/mmap.h>
#include <sary/text.h>
#include <sary/i.h>
#include <sary/saryconfig.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _SarySearcher 	SarySearcher;

typedef struct {
    const gchar *str;
    SaryInt len;   /* length of pattern */
    SaryInt skip;  /* length of bytes which can be skipped */
} SaryPattern;

typedef struct {
    SaryInt  *first;
    SaryInt  *last;
} SaryResult;


SarySearcher* sary_searcher_new                     (const gchar 
                                                     *file_name);
SarySearcher* sary_searcher_new2                    (const gchar *file_name, 
                                                     const gchar *array_name);
void          sary_searcher_destroy                 (SarySearcher *searcher);
gboolean      sary_searcher_search                  (SarySearcher *searcher, 
                                                     const gchar *pattern);
gboolean      sary_searcher_search2                 (SarySearcher *searcher, 
                                                     const gchar *pattern, 
                                                     SaryInt len);
gboolean      sary_searcher_isearch                 (SarySearcher *searcher, 
                                                     const gchar *pattern, 
                                                     SaryInt len);
void          sary_searcher_isearch_reset           (SarySearcher *searcher);
gboolean      sary_searcher_icase_search            (SarySearcher *searcher, 
                                                     const gchar *pattern);
gboolean      sary_searcher_icase_search2           (SarySearcher *searcher, 
                                                     const gchar *pattern, 
                                                     SaryInt len);
gboolean      sary_searcher_multi_search            (SarySearcher *searcher, 
                                                     gchar **patterns,
                                                     gint npatterns);
SaryText*     sary_searcher_get_text                (SarySearcher *searcher);
SaryMmap*     sary_searcher_get_array               (SarySearcher *searcher);
gchar*        sary_searcher_get_next_line           (SarySearcher *searcher);
gchar*        sary_searcher_get_next_line2          (SarySearcher *searcher, 
                                                     SaryInt *len);
gchar*        sary_searcher_get_next_context_lines  (SarySearcher *searcher, 
                                                     SaryInt backward, 
                                                     SaryInt forward);
gchar*        sary_searcher_get_next_context_lines2 (SarySearcher *searcher, 
                                                     SaryInt backward, 
                                                     SaryInt forward,
                                                     SaryInt *len);
gchar*        sary_searcher_get_next_tagged_region  (SarySearcher *searcher, 
                                                     const gchar 
                                                     *start_tag,
                                                     const gchar *end_tag);
gchar*        sary_searcher_get_next_tagged_region2 (SarySearcher *searcher,
                                                     const gchar 
                                                     *start_tag,
                                                     SaryInt start_tag_len,
                                                     const gchar *end_tag,
                                                     SaryInt end_tag_len,
                                                     SaryInt *len);
SaryText*     sary_searcher_get_next_occurrence     (SarySearcher *searcher);
SaryInt       sary_searcher_get_next_position       (SarySearcher *searcher);
SaryInt       sary_searcher_count_occurrences       (SarySearcher *searcher);
void          sary_searcher_sort_occurrences        (SarySearcher *searcher);
void          sary_searcher_enable_cache            (SarySearcher *searcher);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_SEARCHER_H__ */
