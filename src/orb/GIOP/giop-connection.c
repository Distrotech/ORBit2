#include "config.h"
#include <orbit/GIOP/giop.h>
#include <string.h>
#include <unistd.h>

static LINCConnectionClass *parent_class = NULL;

static struct {
	GMutex *lock;
	GList  *list;
} cnx_list = { NULL, NULL };

void
giop_connection_list_init (void)
{
	cnx_list.lock = linc_mutex_new ();
	cnx_list.list = NULL;
}

static void
giop_connection_list_add (GIOPConnection *cnx)
{
	g_return_if_fail (cnx != NULL);
	cnx_list.list = g_list_prepend (cnx_list.list, cnx);
}

void
giop_connection_list_remove (GIOPConnection *cnx)
{
	cnx_list.list = g_list_remove (cnx_list.list, cnx);
}

static GIOPConnection *
giop_connection_list_lookup (const char *proto_name,
			     const char *remote_host_info,
			     const char *remote_serv_info,
			     gboolean    is_ssl)
{
	GList *l;
	GIOPConnection *retval = NULL;

	for (l = cnx_list.list; l && !retval; l = l->next) {
		GIOPConnection *cnx = l->data;

		if (!strcmp (remote_host_info, cnx->parent.remote_host_info) &&
		    !strcmp (remote_serv_info, cnx->parent.remote_serv_info) &&
		    !strcmp (proto_name, cnx->parent.proto->name) &&
		    cnx->parent.status != LINC_DISCONNECTED &&
		    ((cnx->parent.options & LINC_CONNECTION_SSL) == 
		     (is_ssl ? LINC_CONNECTION_SSL : 0)))
			retval = cnx; /* FIXME: break here for speed ? */
	}

	if (retval)
		g_object_ref (G_OBJECT (retval));

	return retval;
}

static void
giop_connection_real_state_changed (LINCConnection      *cnx,
				    LINCConnectionStatus status)
{
	GIOPConnection *gcnx = GIOP_CONNECTION (cnx);

	if (parent_class->state_changed)
		parent_class->state_changed (cnx, status);

	switch (status) {
	case LINC_DISCONNECTED:
		LINC_MUTEX_LOCK (gcnx->incoming_mutex);
		if (gcnx->incoming_msg)
			giop_recv_buffer_unuse (gcnx->incoming_msg);
		gcnx->incoming_msg = NULL;
		LINC_MUTEX_UNLOCK (gcnx->incoming_mutex);
		giop_recv_list_zap (gcnx);
		break;
	default:
		break;
	}
}

void
giop_connection_close (GIOPConnection *cnx)
{
	if (cnx->parent.status == LINC_DISCONNECTED)
		return;

	if (cnx->parent.status == LINC_CONNECTED &&
	    (!cnx->parent.was_initiated ||
	     cnx->giop_version == GIOP_1_2)) {
		GIOPSendBuffer *buf;

		buf = giop_send_buffer_use_close_connection (
			cnx->giop_version);
		giop_send_buffer_write (buf, cnx);
		fsync (cnx->parent.fd);
		giop_send_buffer_unuse (buf);
	}

	linc_connection_state_changed (
		LINC_CONNECTION (cnx), LINC_DISCONNECTED);
}

static void
giop_connection_dispose (GObject *obj)
{
	GIOPConnection *cnx = (GIOPConnection *)obj;

	giop_connection_close (cnx);
	if (cnx->incoming_mutex)
		g_mutex_free (cnx->incoming_mutex);

	if (cnx->outgoing_mutex)
		g_mutex_free (cnx->outgoing_mutex);

	LINC_MUTEX_LOCK (cnx_list.lock);
	giop_connection_list_remove (cnx);
	LINC_MUTEX_UNLOCK (cnx_list.lock);

	if (G_OBJECT_CLASS (parent_class)->dispose)
		G_OBJECT_CLASS (parent_class)->dispose (obj);
}

static void
giop_connection_class_init (GIOPConnectionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->dispose = giop_connection_dispose;

	klass->parent_class.state_changed = giop_connection_real_state_changed;
	klass->parent_class.handle_input  = giop_connection_handle_input;
}

static void
giop_connection_init (GIOPConnection *cnx)
{
	cnx->incoming_mutex = linc_mutex_new ();
	cnx->outgoing_mutex = linc_mutex_new ();
}

GType
giop_connection_get_type (void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof (GIOPConnectionClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) giop_connection_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (GIOPConnection),
			0,              /* n_preallocs */
			(GInstanceInitFunc) giop_connection_init,
		};
      
		object_type = g_type_register_static (
			linc_connection_get_type(),
			"GIOPConnection", &object_info, 0);
	}  

	return object_type;
}

GIOPConnection *
giop_connection_initiate (const char *proto_name,
			  const char *remote_host_info,
			  const char *remote_serv_info,
			  GIOPConnectionOptions options,
			  GIOPVersion giop_version)
{
	GIOPConnection *cnx;

	LINC_MUTEX_LOCK (cnx_list.lock);

#ifndef ORBIT_THREADED
	options |= LINC_CONNECTION_NONBLOCKING;
#endif

	cnx = giop_connection_list_lookup (
		proto_name, remote_serv_info,
		remote_serv_info, (options & LINC_CONNECTION_SSL) ? TRUE : FALSE);

	if (!cnx) {
		cnx = (GIOPConnection *) g_object_new (giop_connection_get_type (), NULL);

		cnx->giop_version = giop_version;
		if (!linc_connection_initiate (
			(LINCConnection *) cnx,
			proto_name, remote_host_info,
			remote_serv_info, options)) {

			LINC_MUTEX_UNLOCK (cnx_list.lock);
			g_object_unref (G_OBJECT(cnx));
			return NULL;
		} else {
			giop_connection_list_add(cnx);
			g_object_ref (G_OBJECT(cnx));
		}
	}

	LINC_MUTEX_UNLOCK (cnx_list.lock);

	return cnx;
}

void
giop_connection_remove_by_orb (gpointer match_orb_data)
{
	GList *l, *next;

	LINC_MUTEX_LOCK (cnx_list.lock);

	for (l = cnx_list.list; l; l = next) {
		GIOPConnection *cnx = l->data;

		next = l->next;
		if (cnx->orb_data == match_orb_data) {
			cnx_list.list = g_list_delete_link (cnx_list.list, l);
			giop_connection_close (cnx);
		}
	}

	LINC_MUTEX_UNLOCK (cnx_list.lock);
}
