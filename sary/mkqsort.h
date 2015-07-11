#ifndef __SARY_MKQSORT_H__
#define __SARY_MKQSORT_H__

#include <sary/saryconfig.h>
#include <sary/progress.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void	sary_multikey_qsort (SaryProgress *progress,
			     SaryInt *array,
			     SaryInt len,
			     SaryInt depth,
			     const gchar *bof,
			     const gchar *eof);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_MKQSORT_H__ */
