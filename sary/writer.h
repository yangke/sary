#ifndef __SARY_WRITER_H__
#define __SARY_WRITER_H__

#include <glib.h>
#include <sary/saryconfig.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _SaryWriter SaryWriter;

SaryWriter*	sary_writer_new		(const gchar *file_name);
void		sary_writer_destroy	(SaryWriter *writer);
gboolean	sary_writer_write	(SaryWriter *writer, SaryInt data);
gboolean	sary_writer_flush	(SaryWriter *writer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_WRITER_H__ */
