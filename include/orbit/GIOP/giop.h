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
void        giop_shutdown         (void);
gboolean    giop_threaded         (void);
GIOPThread *giop_thread_self      (void);
void        giop_thread_push_recv (GIOPMessageQueueEntry *ent);

typedef struct _GIOPQueue GIOPQueue;
typedef void (*GIOPQueueHandler)  (gpointer poa_object,
				   gpointer recv_buffer);
GIOPQueue  *giop_queue_new        (gpointer   queue_handler);
GIOPQueue  *giop_queue_get_main   (void);
void        giop_queue_free       (GIOPQueue *queue);
void        giop_queue_push       (GIOPQueue *queue,
				   gpointer   poa_object,
				   gpointer   recv_buffer);

#endif /* ORBIT2_INTERNAL_API */

G_END_DECLS

#endif
