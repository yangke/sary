#ifndef __SARY_I_H__
#define __SARY_I_H__

#include <glib.h>
#include <sary/text.h>
#include <sary/saryconfig.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Convert an index pointer to a text_position.
 * (SaryInt*)index_ptr is a offset from the beginning of
 * file of a text.
 */
#define sary_i_text(text, index_ptr)  sary_text_get_bof((text)) + \
                                      GINT_FROM_BE(*(SaryInt *)(index_ptr))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_I_H__ */
