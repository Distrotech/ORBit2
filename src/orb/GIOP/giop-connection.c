#include "config.h"
#include <orbit/GIOP/giop.h>
#include <string.h>
#include <unistd.h>

static LINCConnectionClass *parent_class = NULL;

static struct {
  O_MUTEX_DEFINE(lock);
  GList *list;
} cnx_list;

void
giop_connection_list_init(void)
{
  O_MUTEX_INIT(cnx_list.lock);
  cnx_list.list = NULL;
}

static void
giop_connection_list_add(GIOPConnection *cnx)
{
  g_return_if_fail (cnx != NULL);
  cnx_list.list = g_list_prepend(cnx_list.list, cnx);
}

void
giop_connection_list_remove(GIOPConnection *cnx)
{
  cnx_list.list = g_list_remove(cnx_list.list, cnx);
}

static GIOPConnection *
giop_connection_list_lookup(const char *proto_name, const char *remote_host_info,
			    const char *remote_serv_info, gboolean is_ssl)
{
  GList *ltmp;
  GIOPConnection *retval = NULL;

  for(ltmp = cnx_list.list; ltmp && !retval; ltmp = ltmp->next)
    {
      GIOPConnection *cnx = ltmp->data;

      if(!strcmp(remote_host_info, cnx->parent.remote_host_info)
	 && !strcmp(remote_serv_info, cnx->parent.remote_serv_info)
	 && !strcmp(proto_name, cnx->parent.proto->name)
	 && cnx->parent.status != LINC_DISCONNECTED
	 && ((cnx->parent.options & LINC_CONNECTION_SSL)==(is_ssl?LINC_CONNECTION_SSL:0)))
	retval = cnx;
    }
  if(retval)
    g_object_ref(G_OBJECT(retval));

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
		O_MUTEX_LOCK (gcnx->incoming_mutex);
		if (gcnx->incoming_msg)
			giop_recv_buffer_unuse (gcnx->incoming_msg);
		gcnx->incoming_msg = NULL;
		O_MUTEX_UNLOCK (gcnx->incoming_mutex);
		giop_recv_list_zap (gcnx);
		break;
	default:
		break;
	}
}

void
giop_connection_close (GIOPConnection *cnx)
{
	if(cnx->parent.status == LINC_DISCONNECTED)
		return;

	if(cnx->parent.status == LINC_CONNECTED
	   && (!cnx->parent.was_initiated
	       || cnx->giop_version == GIOP_1_2)) {
		GIOPSendBuffer *buf;

		buf = giop_send_buffer_use_close_connection (cnx->giop_version);
		giop_send_buffer_write (buf, cnx);
		fsync (cnx->parent.fd);
		giop_send_buffer_unuse (buf);
	}
	linc_connection_state_changed (LINC_CONNECTION (cnx), LINC_DISCONNECTED);
}

static void
giop_connection_dispose (GObject *obj)
{
	GIOPConnection *cnx = (GIOPConnection *)obj;

	giop_connection_close (cnx);
	O_MUTEX_DESTROY (cnx->incoming_mutex);
	O_MUTEX_DESTROY (cnx->outgoing_mutex);

	O_MUTEX_LOCK (cnx_list.lock);
	giop_connection_list_remove (cnx);
	O_MUTEX_UNLOCK (cnx_list.lock);

	if (G_OBJECT_CLASS (parent_class)->dispose)
		G_OBJECT_CLASS (parent_class)->dispose (obj);
}

static gboolean
giop_connection_handle_input (LINCConnection *lcnx)
{
	GIOPConnection *cnx = (GIOPConnection *) lcnx;
	int n;
	GIOPMessageInfo info;
	gboolean retval = TRUE;

	O_MUTEX_LOCK(cnx->incoming_mutex);

	do {
		if (!cnx->incoming_msg)
			cnx->incoming_msg = giop_recv_buffer_use_buf (
				cnx->parent.is_auth);

		n = linc_connection_read (
			lcnx, cnx->incoming_msg->cur, cnx->incoming_msg->left_to_read, FALSE);
		if (n <= 0) {
			retval = FALSE;
			goto out;
		}

		info = giop_recv_buffer_data_read (
			cnx->incoming_msg, n, cnx->parent.is_auth, cnx);

		if (info != GIOP_MSG_UNDERWAY) {
			if (info == GIOP_MSG_COMPLETE) {
				if (cnx->incoming_msg->msg.header.message_type == GIOP_REQUEST) {
					ORBit_handle_request (cnx->orb_data, cnx->incoming_msg);
					giop_recv_buffer_unuse (cnx->incoming_msg);
				}
			}
			cnx->incoming_msg = NULL;
			if (info == GIOP_MSG_INVALID) {
				/* Zap it for badness.
				   XXX We should probably handle oversized
				   messages more graciously XXX */
				giop_connection_close (cnx);
				if (!cnx->parent.was_initiated) {
					giop_connection_unref (cnx); /* If !was_initiated, then
									a refcount owned by a GIOPServer
									must be released */
					retval = FALSE;
					cnx = NULL;
				}
			}
		}
	} while (cnx && cnx->incoming_msg);

 out:
	if (cnx)
		O_MUTEX_UNLOCK(cnx->incoming_mutex);

	return retval;
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
	O_MUTEX_INIT (cnx->incoming_mutex);
	O_MUTEX_INIT (cnx->outgoing_mutex);
}

GType
giop_connection_get_type(void)
{
  static GType object_type = 0;

  if (!object_type)
    {
      static const GTypeInfo object_info =
      {
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
      
      object_type = g_type_register_static (linc_connection_get_type(),
                                            "GIOPConnection",
                                            &object_info,
					    0);
    }  

  return object_type;
}

GIOPConnection *
giop_connection_from_fd(int fd, const LINCProtocolInfo *proto,
			const char *remote_host_info,
			const char *remote_serv_info,
			gboolean was_initiated,
			LINCConnectionStatus status,
			LINCConnectionOptions options,
			GIOPVersion giop_version)
{
  GIOPConnection *cnx;
  
  O_MUTEX_LOCK(cnx_list.lock);

  cnx = (GIOPConnection *)g_object_new(giop_connection_get_type(), NULL);

  cnx->giop_version = giop_version;
  if(!linc_connection_from_fd((LINCConnection *)cnx, fd, proto, remote_host_info,
			      remote_serv_info, was_initiated, status, options))
    {
      g_object_unref(G_OBJECT(cnx)); cnx = NULL;
    }
  else
    giop_connection_list_add(cnx);

  O_MUTEX_UNLOCK(cnx_list.lock);

  return cnx;
}

void
giop_connection_unref(GIOPConnection *cnx)
{
  if(!cnx)
    return;

  g_object_unref(G_OBJECT(cnx));
}

/* This will just create the fd, do the connect and all, and then call
   giop_connection_from_fd */
GIOPConnection *
giop_connection_initiate(const char *proto_name,
			 const char *remote_host_info,
			 const char *remote_serv_info,
			 GIOPConnectionOptions options,
			 GIOPVersion giop_version)
{
  GIOPConnection *cnx;

  O_MUTEX_LOCK(cnx_list.lock);

  cnx = giop_connection_list_lookup(proto_name, remote_serv_info,
				    remote_serv_info, (options&LINC_CONNECTION_SSL)?TRUE:FALSE);

  if(!cnx)
    {
      cnx = (GIOPConnection *)g_object_new(giop_connection_get_type(), NULL);

      cnx->giop_version = giop_version;
      if(!linc_connection_initiate((LINCConnection *)cnx, proto_name, remote_host_info, remote_serv_info, options))
	{
	  g_object_unref(G_OBJECT(cnx)); cnx = NULL;
	}
      else
        {
	  giop_connection_list_add(cnx);
          g_object_ref (G_OBJECT(cnx));
        }
    }

  O_MUTEX_UNLOCK(cnx_list.lock);

  return cnx;
}

void
giop_connection_remove_by_orb(gpointer match_orb_data)
{
  GList *l, *next;

  O_MUTEX_LOCK(cnx_list.lock);
  for (l = cnx_list.list; l; l = next)
    {
      GIOPConnection *cnx = l->data;

      next = l->next;
      if ( cnx->orb_data == match_orb_data ) {
	cnx_list.list = g_list_delete_link(cnx_list.list, l);
	giop_connection_close (cnx);
      }
    }
  O_MUTEX_UNLOCK(cnx_list.lock);
}
