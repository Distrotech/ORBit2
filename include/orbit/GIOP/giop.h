#ifndef GIOP_H
#define GIOP_H 1

#include <linc/linc.h>
#define ORBIT_SSL_SUPPORT LINC_SSL_SUPPORT

#include <orbit/GIOP/giop-types.h>
#include <orbit/GIOP/giop-send-buffer.h>
#include <orbit/GIOP/giop-recv-buffer.h>
#include <orbit/GIOP/giop-connection.h>
#include <orbit/GIOP/giop-server.h>
#include <orbit/GIOP/giop-endian.h>

G_BEGIN_DECLS

#ifdef ORBIT2_INTERNAL_API

void        giop_init             (gboolean threaded,
				   gboolean blank_wire_data);
void        giop_main_run         (void);
void        giop_shutdown         (void);
gboolean    giop_threaded         (void);
GIOPThread *giop_thread_self      (void);
void        giop_thread_push_recv (GIOPMessageQueueEntry *ent);
void        giop_recv_set_limit   (glong limit);

typedef struct _GIOPQueue GIOPQueue;
GIOPThread *giop_thread_get_main  (void);
void        giop_thread_set_main_handler (gpointer    request_handler);
void        giop_thread_request_push     (GIOPThread *tdata,
					  gpointer   *poa_object,
					  gpointer   *recv_buffer);
void        giop_thread_request_process  (GIOPThread *tdata);

#endif /* ORBIT2_INTERNAL_API */

G_END_DECLS

#endif
