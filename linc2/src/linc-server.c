#include "config.h"
#include <orbit/IIOP/giop-server.h>
#include <netdb.h>

static void giop_server_init       (GIOPServer      *cnx);
static void giop_server_class_init (GIOPServerClass *klass);

GType
giop_server_get_type(void)
{
  static GType object_type = 0;

  if (!object_type)
    {
      static const GTypeInfo object_info =
      {
        sizeof (GIOPServerClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) giop_server_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (GIOPServer),
        0,              /* n_preallocs */
        (GInstanceInitFunc) giop_server_init,
      };
      
      object_type = g_type_register_static (G_TYPE_OBJECT,
                                            "GIOPServer",
                                            &object_info);
    }  

  return object_type;
}

static void
giop_server_init       (GIOPServer      *cnx)
{
  O_MUTEX_INIT(cnx->mutex);
}

enum {
  PARAM_SA_FAMILY,
  LAST_PARAM
};

static void
giop_server_class_init (GIOPServerClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;
}

static gboolean
giop_server_handle_io(GIOChannel *gioc,
		      GIOCondition condition,
		      gpointer data)
{
  GIOPServer *cnx = data;
  struct sockaddr *saddr;
  int addrlen, fd;
  char hnbuf[NI_MAXHOST], servbuf[NI_MAXSERV];

  if(condition != G_IO_IN)
    g_error("condition on server fd is %#x", condition);

  addrlen = cnx->proto->addr_len;
  saddr = orbit_alloca(addrlen);
  fd = accept(cnx->fd, saddr, &addrlen);

  if(fd < 0)
    return TRUE; /* error */

  if(giop_getnameinfo(saddr, cnx->proto->addr_len, hnbuf, sizeof(hnbuf),
		      servbuf, sizeof(servbuf), NI_NUMERICSERV))
    {
      close(fd);
      return TRUE;
    }

  giop_connection_from_fd(fd, cnx->proto, hnbuf, servbuf, FALSE);

  return TRUE;
}

GIOPServer *
giop_server_new(const char *proto_name, GIOPConnectionOptions create_options)
{
  GIOPServer *cnx;
  GIOChannel *gioc;
  int fd, n;
  const GIOPProtocolInfo * proto;
  struct addrinfo *ai, hints = {0};
  char hnbuf[NI_MAXHOST], servbuf[NI_MAXSERV];

  proto = giop_protocol_find(proto_name);
  if(!proto)
    return NULL;

  hints.ai_flags = AI_PASSIVE|AI_CANONNAME;
  hints.ai_family = proto->family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = proto->stream_proto_num;
  if(giop_getaddrinfo(NULL, NULL, &hints, &ai))
    return NULL;
  if(giop_getnameinfo(ai->ai_addr, ai->ai_addrlen, hnbuf, sizeof(hnbuf),
		      servbuf, sizeof(servbuf), NI_NUMERICSERV))
    {
      freeaddrinfo(ai);
      return NULL;
    }

  fd = socket(proto->family, SOCK_STREAM, proto->stream_proto_num);
  if(fd < 0)
    {
      freeaddrinfo(ai);
      return NULL;
    }

  n = 0;
  if(proto->flags & GIOP_PROTOCOL_NEEDS_BIND)
    n = bind(fd, ai->ai_addr, ai->ai_addrlen);
  freeaddrinfo(ai);

  if(!n)
    n = listen(fd, 10);
  if(n)
    {
      close(fd);
      return NULL;
    }

  cnx = g_object_new(giop_server_get_type(), NULL);
  cnx->proto = proto;
  cnx->fd = fd;
  gioc = g_io_channel_unix_new(fd);
  cnx->tag = g_io_add_watch(gioc, G_IO_IN|G_IO_HUP|G_IO_ERR|G_IO_NVAL,
			    giop_server_handle_io, cnx);
  g_io_channel_unref(gioc);
  cnx->create_options = create_options;
  cnx->local_host_info = g_strdup(hnbuf);
  cnx->local_serv_info = g_strdup(servbuf);

  return cnx;
}
