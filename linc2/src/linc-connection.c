#include "config.h"
#include <linc/linc-connection.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

static void linc_connection_init       (LINCConnection      *cnx);
static void linc_connection_state_changed (LINCConnection      *cnx, LINCConnectionStatus status);
static void linc_connection_real_state_changed (LINCConnection  *cnx, LINCConnectionStatus status);
static void linc_connection_destroy    (GObject             *obj);
static void linc_connection_class_init (LINCConnectionClass *klass);

GType
linc_connection_get_type(void)
{
  static GType object_type = 0;

  if (!object_type)
    {
      static const GTypeInfo object_info =
      {
        sizeof (LINCConnectionClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) linc_connection_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (LINCConnection),
        0,              /* n_preallocs */
        (GInstanceInitFunc) linc_connection_init,
      };
      
      object_type = g_type_register_static (G_TYPE_OBJECT,
                                            "LINCConnection",
                                            &object_info);
    }  

  return object_type;
}

static void
linc_connection_class_init (LINCConnectionClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  O_MUTEX_INIT(cnx_list.lock);

  object_class->shutdown = linc_connection_destroy;
  klass->state_changed = linc_connection_real_state_changed;
}

static void
linc_connection_init       (LINCConnection      *cnx)
{
  O_MUTEX_INIT(cnx->incoming_mutex);
  O_MUTEX_INIT(cnx->outgoing_mutex);
}

static void
linc_connection_destroy    (GObject             *obj)
{
  LINCConnection *cnx = (LINCConnection *)obj;

  if(cnx->tag)
    g_source_remove(cnx->tag);
  g_free(cnx->remote_host_info);
  g_free(cnx->remote_serv_info);
  close(cnx->fd);
}

static gboolean
linc_connection_connected(GIOChannel *gioc, GIOCondition condition, gpointer data)
{
  LINCConnection *cnx = data;

  if(condition == G_IO_OUT)
    {
      linc_connection_state_changed(cnx, LINC_CONNECTED);
    }
  else
    {
      linc_connection_state_changed(cnx, LINC_DISCONNECTED);
    }

  return FALSE;
}

static void
linc_connection_real_state_changed (LINCConnection *cnx, LINCConnectionStatus status)
{
  switch(status)
    {
    case LINC_CONNECTED:
#if LINC_SSL_SUPPORT
      if(cnx->was_initiated)
	SSL_connect(cnx->ssl);
      else
	SSL_accept(cnx->ssl);
#endif
      break;
    case LINC_CONNECTING:
      if(cnx->tag) g_source_remove(cnx->tag);
      cnx->tag = g_io_add_watch(cnx->gioc, G_IO_OUT|G_IO_ERR|G_IO_HUP|G_IO_NVAL, linc_connection_connected, cnx);
      break;
    case LINC_DISCONNECTED:
      if(cnx->tag) g_source_remove(cnx->tag);
      cnx->tag = 0;
      break;
    }

  cnx->status = status;
}

gboolean
linc_connection_from_fd(LINCConnection *cnx, int fd, const LINCProtocolInfo *proto,
			const char *remote_host_info,
			const char *remote_serv_info,
			gboolean was_initiated,
			LINCConnectionStatus status,
			LINCConnectionOptions options)
{
  cnx->gioc = g_io_channel_unix_new(fd);
  cnx->was_initiated = was_initiated;
  cnx->is_auth = (proto->flags & LINC_PROTOCOL_SECURE)?TRUE:FALSE;
  cnx->remote_host_info = g_strdup(remote_host_info);
  cnx->remote_serv_info = g_strdup(remote_serv_info);
  cnx->options = options;

#if LINC_SSL_SUPPORT
  if(options & LINC_CONNECTION_SSL)
    {
      cnx->ssl = SSL_new(linc_ssl_ctx);
      SSL_set_fd(cnx->ssl, fd);
    }
#endif

  linc_connection_state_changed(cnx, status);

  return TRUE;
}

/* This will just create the fd, do the connect and all, and then call
   linc_connection_from_fd */
gboolean
linc_connection_initiate(LINCConnection *cnx, const char *proto_name,
			 const char *remote_host_info,
			 const char *remote_serv_info,
			 LINCConnectionOptions options)
{
  struct addrinfo *ai, hints = {AI_CANONNAME, 0, SOCK_STREAM, 0, 0,
				NULL, NULL, NULL};
  int rv;
  int fd;
  const LINCProtocolInfo *proto;
  gboolean retval = FALSE;

  proto = linc_protocol_find(proto_name);

  if(!proto)
    return FALSE;

  hints.ai_family = proto->family;
  rv = linc_getaddrinfo(remote_host_info, remote_serv_info, &hints, &ai);
  if(rv)
    return FALSE;

  fd = socket(proto->family, SOCK_STREAM, proto->stream_proto_num);
  if(fd < 0)
    goto out;
  fcntl(fd, F_SETFL, O_NONBLOCK);
  fcntl(fd, F_SETFD, FD_CLOEXEC);

  rv = connect(fd, ai->ai_addr, ai->ai_addrlen);
  if(rv && errno != EINPROGRESS)
    goto out;

  retval = linc_connection_from_fd(cnx, fd, proto, remote_host_info,
				   remote_serv_info, TRUE, rv?LINC_CONNECTING:LINC_CONNECTED, options);

 out:
  if(!cnx)
    {
      if(fd >= 0)
	close(fd);
    }
  freeaddrinfo(ai);

  return retval;
}

void
linc_connection_state_changed(LINCConnection *cnx, LINCConnectionStatus status)
{
  LINCConnectionClass *klass;

  klass = (LINCConnectionClass *)G_OBJECT_GET_CLASS(cnx);

  if(klass->state_changed)
    klass->state_changed(cnx, status);
}
