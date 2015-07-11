#ifndef __SARY_PROGRESS_H__
#define __SARY_PROGRESS_H__

#include <glib.h>
#include <sary/saryconfig.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct	_SaryProgress		SaryProgress;
typedef void 	(*SaryProgressFunc)	(SaryProgress *progress);

struct _SaryProgress {
    gchar		*task;
    SaryInt		current;
    SaryInt		previous;
    SaryInt		total;
    time_t		start_time;
    clock_t		start_processer_time;
    gboolean		is_finished;
    SaryProgressFunc	func;
    gpointer		func_data;
};

SaryProgress*	sary_progress_new	(const gchar *task, 
					 SaryInt total);

void		sary_progress_connect	(SaryProgress *progress,
					 SaryProgressFunc func,
					 gpointer func_data);

void		sary_progress_destroy	(SaryProgress *progress);

void		sary_progress_set_count	(SaryProgress *progress, 
					 SaryInt count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SARY_PROGRESS_H__ */
