#include "config.h"
#include <orbit/GIOP/giop.h>
#include <string.h>
#include <unistd.h>

#include "giop-private.h"

#undef CNX_LIST_DEBUG

static LINCConnectionClass *parent_class = NULL;

static struct {
	GMutex *lock;
	GList  *list;
} cnx_list = { NULL, NULL };

#ifdef CNX_LIST_DEBUG
static char *
giop_cnx_descr (GIOPConnection *cnx)
{
	return g_strdup_printf ("Cnx (%p) - '%s' '%s' '%s' - %s",
				cnx, cnx->parent.proto->name,
				cnx->parent.remote_host_info ? cnx->parent.remote_host_info : "<Null>",
				cnx->parent.remote_serv_info ? cnx->parent.remote_serv_info : "<Null>",
				cnx->parent.options & LINC_CONNECTION_SSL ? "ssl" : "no ssl");
}
#endif

void
giop_connection_list_init (void)
{
	cnx_list.lock = linc_mutex_new ();
	cnx_list.list = NULL;
}

static void
giop_connection_list_add (GIOPConnection *cnx)
{
#ifdef CNX_LIST_DEBUG
	g_warning ("Add '%s'", giop_cnx_descr (cnx));
	g_assert (cnx->parent.was_initiated);
#endif

	cnx_list.list = g_list_prepend (
		cnx_list.list, 
		g_object_ref (G_OBJECT (cnx)));
}

static void
giop_connection_list_remove (GIOPConnection *cnx)
{
#ifdef CNX_LIST_DEBUG
	g_warning ("Remove '%s'", giop_cnx_descr (cnx));
#endif
	if (cnx->parent.was_initiated)
		cnx_list.list = g_list_remove (cnx_list.list, cnx);
}

static GIOPConnection *
giop_connection_list_lookup (const char *proto_name,
			     const char *remote_host_info,
			     const char *remote_serv_info,
			     gboolean    is_ssl)
{
	GList *l;
	const LINCProtocolInfo *proto;

	proto = linc_protocol_find (proto_name);

	for (l = cnx_list.list; l; l = l->next) {
		GIOPConnection *cnx = l->data;

		if (cnx->parent.proto == proto &&
		    cnx->parent.status != LINC_DISCONNECTED &&
		    ((cnx->parent.options & LINC_CONNECTION_SSL) == 
		     (is_ssl ? LINC_CONNECTION_SSL : 0)) &&
		    !strcmp (remote_host_info, cnx->parent.remote_host_info) &&
		    !strcmp (remote_serv_info, cnx->parent.remote_serv_info))

			return g_object_ref (G_OBJECT (cnx));
	}

	return NULL;
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
		if (gcnx->incoming_msg) {
			giop_recv_buffer_unuse (gcnx->incoming_msg);
			gcnx->incoming_msg = NULL;
		}
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
		giop_send_buffer_write (buf, cnx, TRUE);
		giop_send_buffer_unuse (buf);
	}

	linc_connection_state_changed (
		LINC_CONNECTION (cnx), LINC_DISCONNECTED);
}

static void
giop_connection_dispose (GObject *obj)
{
	GIOPConnection *cnx = (GIOPConnection *) obj;

	giop_connection_close (cnx);

	if (cnx->incoming_mutex) {
		g_mutex_free (cnx->incoming_mutex);
		cnx->incoming_mutex = NULL;
	}

	if (cnx->outgoing_mutex) {
		g_mutex_free (cnx->outgoing_mutex);
		cnx->outgoing_mutex = NULL;
	}

	giop_connection_list_remove (cnx);

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

	g_return_val_if_fail (remote_host_info != NULL, NULL);

	LINC_MUTEX_LOCK (cnx_list.lock);

#ifndef ORBIT_THREADED
	options |= LINC_CONNECTION_NONBLOCKING;
#endif

	cnx = giop_connection_list_lookup (
		proto_name, remote_host_info,
		remote_serv_info, (options & LINC_CONNECTION_SSL));

	if (!cnx) {
		cnx = (GIOPConnection *) g_object_new (
			giop_connection_get_type (), NULL);

		cnx->giop_version = giop_version;
		if (!linc_connection_initiate (
			(LINCConnection *) cnx,
			proto_name, remote_host_info,
			remote_serv_info, options)) {

			LINC_MUTEX_UNLOCK (cnx_list.lock);
			g_object_unref (G_OBJECT (cnx));

			return NULL;
		} else
			giop_connection_list_add (cnx);
	}

	LINC_MUTEX_UNLOCK (cnx_list.lock);

	return cnx;
}

void
giop_connection_remove_by_orb (gpointer match_orb_data)
{
	GList *l, *next;
	GSList *sl, *to_close = NULL;

	LINC_MUTEX_LOCK (cnx_list.lock);

	for (l = cnx_list.list; l; l = next) {
		GIOPConnection *cnx = l->data;

		next = l->next;

		if (cnx->orb_data == match_orb_data) {
			to_close = g_slist_prepend (to_close, cnx);
			cnx_list.list = g_list_delete_link (cnx_list.list, l);
		}
	}

	LINC_MUTEX_UNLOCK (cnx_list.lock);

	for (sl = to_close; sl; sl = sl->next) {
		GIOPConnection *cnx = sl->data;

		giop_connection_close (cnx);
		g_object_unref (G_OBJECT (cnx));
	}
	g_slist_free (to_close);
}

void
giop_connection_unref (GIOPConnection *cnx)
{
	if (cnx) {
		LINC_MUTEX_LOCK (cnx_list.lock);
		
		g_object_unref (G_OBJECT (cnx));
		
		LINC_MUTEX_UNLOCK (cnx_list.lock);
	}
}
