#include "config.h"
#include <orbit/GIOP/giop.h>
#include <string.h>
#include <unistd.h>

#include "giop-private.h"

#undef CNX_LIST_DEBUG

static LINCConnectionClass *parent_class = NULL;

static struct {
	GStaticRecMutex  lock;
	GList           *list;
} cnx_list = { G_STATIC_REC_MUTEX_INIT, NULL };

#define CNX_LIST_LOCK \
	G_STMT_START { \
		if (giop_threaded ()) \
			 g_static_rec_mutex_lock (&cnx_list.lock); \
	} G_STMT_END
#define CNX_LIST_UNLOCK \
	G_STMT_START { \
		if (giop_threaded ()) \
			 g_static_rec_mutex_unlock (&cnx_list.lock); \
	} G_STMT_END

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
	cnx_list.list = NULL;
}

static void
giop_connection_list_add (GIOPConnection *cnx)
{
#ifdef CNX_LIST_DEBUG
	g_warning ("Add '%s'", giop_cnx_descr (cnx));
	g_assert (cnx->parent.was_initiated);
#endif
	cnx_list.list = g_list_prepend (cnx_list.list, cnx);
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

			return linc_object_ref (cnx);
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
		if (gcnx->incoming_msg) {
			giop_recv_buffer_unuse (gcnx->incoming_msg);
			gcnx->incoming_msg = NULL;
		}
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

	linc_connection_disconnect (LINC_CONNECTION (cnx));
}

static void
giop_connection_dispose (GObject *obj)
{
	GIOPConnection *cnx = (GIOPConnection *) obj;

	giop_connection_close (cnx);

	giop_connection_destroy_frags (cnx);

	giop_connection_list_remove (cnx);

	g_assert (cnx->incoming_msg == NULL);

	if (((GObjectClass *)parent_class)->dispose)
		((GObjectClass *)parent_class)->dispose (obj);
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

void
giop_connection_set_orb_n_ver (GIOPConnection *cnx,
			       gpointer        orb_data,
			       GIOPVersion     version)
{
	cnx->orb_data = orb_data;
	cnx->giop_version = version;
}

GIOPConnection *
giop_connection_initiate (gpointer orb_data,
			  const char *proto_name,
			  const char *remote_host_info,
			  const char *remote_serv_info,
			  GIOPConnectionOptions options,
			  GIOPVersion giop_version)
{
	GIOPConnection *cnx;

	g_return_val_if_fail (remote_host_info != NULL, NULL);

	CNX_LIST_LOCK;

	options |= LINC_CONNECTION_NONBLOCKING;

	cnx = giop_connection_list_lookup (
		proto_name, remote_host_info,
		remote_serv_info, (options & LINC_CONNECTION_SSL));

	if (!cnx) {
		cnx = (GIOPConnection *) g_object_new (
			giop_connection_get_type (), NULL);

		giop_connection_set_orb_n_ver (
			cnx, orb_data, giop_version);

		if (!linc_connection_initiate (
			(LINCConnection *) cnx,
			proto_name, remote_host_info,
			remote_serv_info, options)) {

			CNX_LIST_UNLOCK;
			linc_object_unref (cnx);

			return NULL;
		} else
			giop_connection_list_add (cnx);
	}

	CNX_LIST_UNLOCK;

	return cnx;
}

void
giop_connections_shutdown (void)
{
	GList *l, *to_close;

	CNX_LIST_LOCK;

	to_close = cnx_list.list;
	cnx_list.list = NULL;

	CNX_LIST_UNLOCK;

	for (l = to_close; l; l = l->next) {
		GIOPConnection *cnx = l->data;

		giop_connection_close (cnx);
		linc_object_unref (cnx);
	}

	g_list_free (to_close);

	if (cnx_list.list != NULL)
		g_warning ("Wierd; new connections opened while shutting down");
}

void
giop_connection_unref (GIOPConnection *cnx)
{
	if (cnx)
		linc_object_unref (cnx);
}
