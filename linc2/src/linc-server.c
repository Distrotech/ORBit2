#include "config.h"
#include <linc/linc-server.h>
#include <linc/linc-connection.h>
#include <netdb.h>
#include <unistd.h>

static void linc_server_init       (LINCServer      *cnx);
static LINCConnection *linc_server_create_connection (LINCServer      *cnx);
static void linc_server_destroy    (GObject         *obj);
static void linc_server_class_init (LINCServerClass *klass);

GType
linc_server_get_type(void)
{
  static GType object_type = 0;

  if (!object_type)
    {
      static const GTypeInfo object_info =
      {
        sizeof (LINCServerClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) linc_server_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (LINCServer),
        0,              /* n_preallocs */
        (GInstanceInitFunc) linc_server_init,
      };
      
      object_type = g_type_register_static (G_TYPE_OBJECT,
                                            "LINCServer",
                                            &object_info);
    }  

  return object_type;
}

static void
linc_server_class_init (LINCServerClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  object_class->shutdown = linc_server_destroy;
  klass->create_connection = linc_server_create_connection;
}

static void
linc_server_init       (LINCServer      *cnx)
{
  O_MUTEX_INIT(cnx->mutex);
}

static void
linc_server_destroy(GObject         *obj)
{
  LINCServer *cnx = (LINCServer *)obj;

  O_MUTEX_LOCK(cnx->mutex);
  O_MUTEX_DESTROY(cnx->mutex);
  g_source_remove(cnx->tag);
  close(cnx->fd);
  g_free(cnx->local_host_info);
  g_free(cnx->local_serv_info);
}

static LINCConnection *
linc_server_create_connection(LINCServer      *cnx)
{
  return g_object_new(linc_connection_get_type(), NULL);
}

static gboolean
linc_server_handle_io(GIOChannel *gioc,
		      GIOCondition condition,
		      gpointer data)
{
  LINCServer *server = data;
  LINCServerClass *klass = (LINCServerClass *)G_OBJECT_GET_CLASS(server);
  struct sockaddr *saddr;
  int addrlen, fd;
  char hnbuf[NI_MAXHOST], servbuf[NI_MAXSERV];
  LINCConnection *connection;

  O_MUTEX_LOCK(server->mutex);

  if(condition != G_IO_IN)
    g_error("condition on server fd is %#x", condition);

  addrlen = server->proto->addr_len;
  saddr = g_alloca(addrlen);
  fd = accept(server->fd, saddr, &addrlen);

  if(fd < 0)
    return TRUE; /* error */

  if(linc_getnameinfo(saddr, server->proto->addr_len, hnbuf, sizeof(hnbuf),
		      servbuf, sizeof(servbuf), NI_NUMERICSERV))
    {
      close(fd);
      return TRUE;
    }

  g_assert(klass->create_connection);
  connection = klass->create_connection(server);
  if(!linc_connection_from_fd(connection, fd, server->proto, hnbuf, servbuf,
			      FALSE, LINC_CONNECTED, server->create_options))
    {
      g_object_unref(G_OBJECT(connection));
      connection = NULL;
    }

  O_MUTEX_UNLOCK(server->mutex);

  return TRUE;
}

gboolean
linc_server_setup(LINCServer *cnx, const char *proto_name, 
		  const char *local_host_info, const char *local_serv_info,
		  LINCConnectionOptions create_options)
{
  GIOChannel *gioc;
  int fd, n;
  const LINCProtocolInfo * proto;
  struct addrinfo *ai, hints = {0};
  char hnbuf[NI_MAXHOST], servbuf[NI_MAXSERV];

  proto = linc_protocol_find(proto_name);
  if(!proto)
    return FALSE;

  hints.ai_flags = AI_PASSIVE|AI_CANONNAME;
  hints.ai_family = proto->family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = proto->stream_proto_num;
  if(linc_getaddrinfo(local_host_info, local_serv_info, &hints, &ai))
    return FALSE;
  if(linc_getnameinfo(ai->ai_addr, ai->ai_addrlen, hnbuf, sizeof(hnbuf),
		      servbuf, sizeof(servbuf), NI_NUMERICSERV))
    {
      freeaddrinfo(ai);
      return FALSE;
    }

  fd = socket(proto->family, SOCK_STREAM, proto->stream_proto_num);
  if(fd < 0)
    {
      freeaddrinfo(ai);
      return FALSE;
    }

  n = 0;
  if((proto->flags & LINC_PROTOCOL_NEEDS_BIND)
     || local_serv_info)
    n = bind(fd, ai->ai_addr, ai->ai_addrlen);
  freeaddrinfo(ai);

  if(!n)
    n = listen(fd, 10);
  if(n)
    {
      close(fd);
      return FALSE;
    }

  cnx->proto = proto;
  cnx->fd = fd;
  gioc = g_io_channel_unix_new(fd);
  cnx->tag = g_io_add_watch(gioc, G_IO_IN|G_IO_HUP|G_IO_ERR|G_IO_NVAL,
			    linc_server_handle_io, cnx);
  g_io_channel_unref(gioc);
  cnx->create_options = create_options;
  cnx->local_host_info = g_strdup(hnbuf);
  cnx->local_serv_info = g_strdup(servbuf);

  return TRUE;
}
