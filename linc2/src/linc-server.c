#include "config.h"
#include <linc/linc-server.h>
#include <linc/linc-connection.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

static void linc_server_init       (LINCServer      *cnx);
static LINCConnection *linc_server_create_connection (LINCServer      *cnx);
static void linc_server_destroy    (GObject         *obj);
static void linc_server_class_init (LINCServerClass *klass);

enum {
  NEW_CONNECTION,
  LAST_SIGNAL
};
static guint server_signals[LAST_SIGNAL] = {0};

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
                                            &object_info,
					    0);
    }  

  return object_type;
}

static GObjectClass *parent_class = NULL;

static void
my_cclosure_marshal_VOID__OBJECT (GClosure     *closure,
                                  GValue       *return_value,
                                  guint         n_param_values,
                                  const GValue *param_values,
                                  gpointer      invocation_hint,
                                  gpointer      marshal_data)
{
  typedef void (*GSignalFunc_VOID__OBJECT) (gpointer     data1,
					    GObject     *arg_1,
                                             gpointer     data2);
  register GSignalFunc_VOID__OBJECT callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;

  g_return_if_fail (n_param_values >= 2);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_get_as_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_get_as_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GSignalFunc_VOID__OBJECT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_value_get_as_pointer (param_values + 1),
            data2);
}

static void
linc_server_class_init (LINCServerClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;
  GClosure *closure;
  GType ptype;

  object_class->shutdown = linc_server_destroy;
  klass->create_connection = linc_server_create_connection;
  parent_class = g_type_class_ref(g_type_parent(G_TYPE_FROM_CLASS(object_class)));
  closure = g_signal_type_cclosure_new(G_OBJECT_CLASS_TYPE(klass),
				       G_STRUCT_OFFSET(LINCServerClass, new_connection));

  ptype = G_TYPE_OBJECT;
  server_signals[NEW_CONNECTION] = g_signal_newv("new_connection",
						 G_OBJECT_CLASS_TYPE(klass),
						 0, closure,
						 NULL,
						 my_cclosure_marshal_VOID__OBJECT,
						 G_TYPE_NONE,
						 1, &ptype);
}

static void
linc_server_init       (LINCServer      *cnx)
{
  O_MUTEX_INIT(cnx->mutex);
  cnx->fd = -1;
}

static void
linc_server_destroy(GObject         *obj)
{
  LINCServer *cnx = (LINCServer *)obj;

  O_MUTEX_DESTROY(cnx->mutex);
  if(cnx->tag)
    g_source_remove(cnx->tag);
  if(cnx->proto && cnx->proto->destroy)
    cnx->proto->destroy(cnx->fd, cnx->local_host_info, cnx->local_serv_info);
  if(cnx->fd >= 0)
    close(cnx->fd);
  g_free(cnx->local_host_info);
  g_free(cnx->local_serv_info);
  if(parent_class->shutdown)
    parent_class->shutdown(obj);
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
  GValue parms[2];

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

  memset(parms, 0, sizeof(parms));
  g_value_init(parms, G_OBJECT_TYPE(server));
  g_value_set_object(parms, G_OBJECT(server));
  g_value_init(parms + 1, G_TYPE_OBJECT);
  g_value_set_object(parms + 1, G_OBJECT(connection));
  g_signal_emitv(parms, server_signals[NEW_CONNECTION], 0, NULL);
  g_value_unset(parms);
  g_value_unset(parms+1);

  return TRUE;
}

void
linc_server_handle(LINCServer *cnx)
{
  linc_server_handle_io(NULL, G_IO_IN, cnx);
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
  char hnbuf[NI_MAXHOST], servbuf[256];

#if !LINC_SSL_SUPPORT
  if(create_options & LINC_CONNECTION_SSL)
     return FALSE;
#endif

  proto = linc_protocol_find(proto_name);
  if(!proto)
    return FALSE;

  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = proto->family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = proto->stream_proto_num;
  if(!local_host_info)
    {
      local_host_info = hnbuf;
      gethostname(hnbuf, sizeof(hnbuf));
    }
  if(linc_getaddrinfo(local_host_info, local_serv_info, &hints, &ai))
    return FALSE;

  fd = socket(proto->family, SOCK_STREAM, proto->stream_proto_num);
  if(fd < 0)
  {
      freeaddrinfo(ai);
      return FALSE;
    }
  {
    static const int oneval = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &oneval, sizeof(oneval));
  }
    
    n = 0;
  if((proto->flags & LINC_PROTOCOL_NEEDS_BIND)
     || local_serv_info)
  n = bind(fd, ai->ai_addr, ai->ai_addrlen);
  if(!n)
    n = listen(fd, 10);
  if(!n)
    n = getsockname(fd, ai->ai_addr, &ai->ai_addrlen);
  if(linc_getnameinfo(ai->ai_addr, ai->ai_addrlen, hnbuf, sizeof(hnbuf),
		      servbuf, sizeof(servbuf), NI_NUMERICSERV))
    {
      freeaddrinfo(ai);
      return FALSE;
    }
  freeaddrinfo(ai);

  if(n)
    {
      close(fd);
      return FALSE;
    }

  cnx->proto = proto;
  cnx->fd = fd;
  if((create_options & LINC_CONNECTION_NONBLOCKING))
    {
      gioc = g_io_channel_unix_new(fd);
      cnx->tag = g_io_add_watch(gioc, G_IO_IN|G_IO_HUP|G_IO_ERR|G_IO_NVAL,
				linc_server_handle_io, cnx);
      g_io_channel_unref(gioc);
    }
  cnx->create_options = create_options;
  cnx->local_host_info = g_strdup(hnbuf);
  cnx->local_serv_info = g_strdup(servbuf);

  return TRUE;
}
