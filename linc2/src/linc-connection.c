#include "config.h"
#include <linc/linc-connection.h>

static void linc_connection_init       (LINCConnection      *cnx);
static void linc_connection_destroy    (GObject             *obj);
static void linc_connection_class_init (LINCConnectionClass *klass);

static struct {
  O_MUTEX_DEFINE(lock);
  GList *list;
} cnx_list = {NULL};

static void
linc_connection_list_add(LINCConnection *cnx)
{
  O_MUTEX_LOCK(cnx_list.lock);

  cnx_list.list = g_list_prepend(cnx_list.list, cnx);

  O_MUTEX_UNLOCK(cnx_list.lock);  
}

static void
linc_connection_list_remove(LINCConnection *cnx)
{
  O_MUTEX_LOCK(cnx_list.lock);

  cnx_list.list = g_list_remove(cnx_list.list, cnx);

  O_MUTEX_UNLOCK(cnx_list.lock);
}

static LINCConnection *
linc_connection_list_lookup(const char *proto_name, const char *remote_host_info,
			    const char *remote_serv_info)
{
  GList *ltmp;
  LINCConnection *retval = NULL;

#ifdef LINC_THREADSAFE
  if(!cnx_list.lock) /* If things haven't been initialized yet, outta here */
    return NULL;
#endif

  O_MUTEX_LOCK(cnx_list.lock);
  for(ltmp = cnx_list.list; ltmp && !retval; ltmp = ltmp->next)
    {
      LINCConnection *cnx = ltmp->data;

      if(!strcmp(remote_host_info, cnx->remote_host_info)
	 && !strcmp(remote_serv_info, cnx->remote_serv_info)
	 && !strcmp(proto_name, cnx->proto->name))
	retval = cnx;
    }
  if(retval)
    linc_connection_ref(retval);
  O_MUTEX_UNLOCK(cnx_list.lock);

  return retval;
}

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

  object_class->destroy = linc_connection_destroy;
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

  O_MUTEX_LOCK(cnx->incoming_mutex);
  O_MUTEX_LOCK(cnx->outgoing_mutex);
  O_MUTEX_DESTROY(cnx->incoming_mutex);
  O_MUTEX_DESTROY(cnx->outgoing_mutex);

#ifdef XXX_FIXME_XXX
  if(cnx->incoming_msg)
    linc_recv_buffer_unuse(cnx->incoming_msg);

  g_list_foreach(cnx->outgoing_mutex, (GFunc)linc_send_buffer_unuse, NULL);
#endif

  g_source_remove(cnx->tag);
  g_free(cnx->remote_host_info);
  g_free(cnx->remote_serv_info);
  close(cnx->fd);
}

LINCConnection *
linc_connection_from_fd(int fd, const LINCProtocolInfo *proto,
			const char *remote_host_info,
			const char *remote_serv_info,
			gboolean was_initiated,
			LINCConnectionOptions options)
{
  LINCConnection *cnx;

  cnx = g_object_new(linc_connection_get_type(), NULL);

  cnx->gioc = g_io_channel_unix_new(fd);
  cnx->was_initiated = was_initiated;
  cnx->is_auth = (proto->flags & LINC_PROTOCOL_SECURE)?TRUE:FALSE;
  cnx->remote_host_info = g_strdup(remote_host_info);
  cnx->remote_serv_info = g_strdup(remote_serv_info);
  cnx->options = options;

  linc_connection_list_add(cnx);

#if LINC_SSL_SUPPORT
  if(options & LINC_CONNECTION_SSL)
    {
      cnx->ssl = SSL_new(linc_ssl_ctx);
      SSL_set_fd(cnx->ssl, fd);
    }
#endif

  if(was_initiated)
    {
#if LINC_SSL_SUPPORT
      SSL_connect(cnx->ssl);
#endif
      /* Send greeting message */
    }
  else
    {
#if LINC_SSL_SUPPORT
      SSL_accept(cnx->ssl);
#endif
    }

  return cnx;
}

/* This will just create the fd, do the connect and all, and then call
   linc_connection_from_fd */
LINCConnection *
linc_connection_initiate(const char *proto_name,
			 const char *remote_host_info,
			 const char *remote_serv_info,
			 LINCConnectionOptions options)
{
  LINCConnection *retval = NULL;
  struct addrinfo *ai, hints = {AI_CANONNAME, 0, SOCK_STREAM, 0, 0,
				NULL, NULL, NULL};
  int rv;
  int fd;
  const LINCProtocolInfo *proto;

  retval = linc_connection_list_lookup(proto_name, remote_host_info,
				       remote_serv_info);
  if(retval)
    return retval;

  proto = linc_protocol_find(proto_name);

  if(!proto)
    return NULL;

  hints.ai_family = proto->family;
  rv = linc_getaddrinfo(remote_host_info, remote_serv_info, &hints, &ai);
  if(rv)
    return NULL;

  fd = socket(proto->family, SOCK_STREAM, proto->stream_proto_num);
  if(fd < 0)
    goto out;

  if(connect(fd, ai->ai_addr, ai->ai_addrlen))
    goto out;

  retval = linc_connection_from_fd(fd, proto, remote_host_info,
				   remote_serv_info, TRUE, options);

 out:
  if(fd >= 0)
    close(fd);
  freeaddrinfo(ai);

  return retval;
}

void
linc_connection_ref(LINCConnection *cnx)
{
  g_object_ref((GObject *)cnx);
}

void
linc_connection_unref(LINCConnection *cnx)
{
  GObject *gobj;

  O_MUTEX_LOCK(cnx_list.lock);
  gobj = (GObject *)cnx;
  if(gobj->ref_count == 1)
    linc_connection_list_remove(cnx);
  g_object_unref(gobj);
  O_MUTEX_UNLOCK(cnx_list.lock);  
}
