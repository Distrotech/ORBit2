#include "config.h"
#include <orbit/IIOP/giop-connection.h>

static void giop_connection_init       (GIOPConnection      *cnx);
static void giop_connection_class_init (GIOPConnectionClass *klass);

static struct {
  O_MUTEX_DEFINE(lock);
  GList *list;
} cnx_list = {NULL};

static void
giop_connection_list_add(GIOPConnection *cnx)
{
  O_MUTEX_LOCK(cnx_list.lock);

  cnx_list.list = g_list_prepend(cnx_list.list, cnx);

  O_MUTEX_UNLOCK(cnx_list.lock);  
}

static void
giop_connection_list_remove(GIOPConnection *cnx)
{
  O_MUTEX_LOCK(cnx_list.lock);

  cnx_list.list = g_list_remove(cnx_list.list, cnx);

  O_MUTEX_UNLOCK(cnx_list.lock);
}

static GIOPConnection *
giop_connection_list_lookup(const char *remote_host_info, const char *remote_serv_info, const char *proto_name)
{
  GList *ltmp;
  GIOPConnection *retval = NULL;

#ifdef ORBIT_THREADSAFE
  if(!cnx_list.lock) /* If things haven't been initialized yet, outta here */
    return NULL;
#endif

  O_MUTEX_LOCK(cnx_list.lock);
  for(ltmp = cnx_list.list; ltmp && !retval; ltmp = ltmp->next)
    {
      GIOPConnection *cnx = ltmp->data;

      if(!strcmp(remote_host_info, cnx->remote_host_info)
	 && !strcmp(remote_serv_info, cnx->remote_serv_info)
	 && !strcmp(proto_name, cnx->proto->name))
	retval = cnx;
    }
  if(retval)
    giop_connection_ref(retval);
  O_MUTEX_UNLOCK(cnx_list.lock);

  return retval;
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
      
      object_type = g_type_register_static (G_TYPE_OBJECT,
                                            "GIOPConnection",
                                            &object_info);
    }  

  return object_type;
}

static void
giop_connection_init       (GIOPConnection      *cnx)
{
  O_MUTEX_INIT(cnx->incoming_mutex);
  O_MUTEX_INIT(cnx->outgoing_mutex);
}

static void
giop_connection_class_init (GIOPConnectionClass *klass)
{
  O_MUTEX_INIT(cnx_list.lock);
}

GIOPConnection *
giop_connection_from_fd(int fd, const GIOPProtocolInfo *proto, const char *remote_host_info, const char *remote_serv_info,
			gboolean was_initiated)
{
  GIOPConnection *cnx;

  cnx = g_object_new(giop_connection_get_type(), NULL);

  cnx->gioc = g_io_channel_unix_new(fd);
  cnx->was_initiated = was_initiated;
  cnx->remote_host_info = g_strdup(remote_host_info);
  cnx->remote_serv_info = g_strdup(remote_serv_info);

  giop_connection_list_add(cnx);

  return cnx;
}

/* This will just create the fd, do the connect and all, and then call giop_connection_from_fd */
GIOPConnection *
giop_connection_initiate(const char *proto_name, const char *remote_host_info, const char *remote_serv_info)
{
  GIOPConnection *retval;
  struct addrinfo *ai, hints = {AI_CANONNAME, 0, SOCK_STREAM, 0, 0, NULL, NULL, NULL};
  int rv;
  int fd;
  const GIOPProtocolInfo *proto;

  retval = giop_connection_list_lookup(remote_host_info, remote_serv_info, proto_name);
  if(retval)
    return retval;

  proto = giop_protocol_find(proto_name);

  if(!proto)
    return NULL;

  hints.ai_family = proto->family;
  rv = giop_getaddrinfo(remote_host_info, remote_serv_info, &hints, &ai);

  if(rv)
    return NULL;

  retval = giop_connection_from_fd(fd, proto, remote_host_info, remote_serv_info, TRUE);

  freeaddrinfo(ai);

  return retval;
}

void
giop_connection_ref(GIOPConnection *cnx)
{
  g_object_ref((GObject *)cnx);
}

void
giop_connection_unref(GIOPConnection *cnx)
{
  GObject *gobj;

  O_MUTEX_LOCK(cnx_list.lock);
  gobj = (GObject *)cnx;
  if(gobj->ref_count == 1)
    giop_connection_list_remove(cnx);
  g_object_unref(gobj);
  O_MUTEX_UNLOCK(cnx_list.lock);  
}
