#include "config.h"
#include <orbit/IIOP/giop-connection.h>

static void giop_connection_init       (GIOPConnection      *cnx);
static void giop_connection_destroy    (GObject             *obj);
static void giop_connection_class_init (GIOPConnectionClass *klass);
static void giop_connection_real_state_changed(LINCConnection *cnx, LINCConnectionStatus status);

#if 0
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
giop_connection_list_lookup(const char *proto_name, const char *remote_host_info,
			    const char *remote_serv_info)
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

      if(!strcmp(remote_host_info, cnx->parent.remote_host_info)
	 && !strcmp(remote_serv_info, cnx->parent.remote_serv_info)
	 && !strcmp(proto_name, cnx->parent.proto->name)
	 && cnx->parent.status != LINC_DISCONNECTED)
	retval = cnx;
    }
  if(retval)
    g_object_ref(G_OBJECT(retval));
  O_MUTEX_UNLOCK(cnx_list.lock);

  return retval;
}
#endif

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
                                            &object_info);
    }  

  return object_type;
}

static GObjectClass *parent_class = NULL;

static void
giop_connection_class_init (GIOPConnectionClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  parent_class = g_type_class_ref(g_type_parent(G_TYPE_FROM_CLASS(object_class)));
  object_class->shutdown = giop_connection_destroy;
  klass->parent_class.state_changed = giop_connection_real_state_changed;
}

static void
giop_connection_init       (GIOPConnection      *cnx)
{
}

static void
giop_connection_destroy    (GObject             *obj)
{
#ifdef ORBIT_THREADSAFE
  GIOPConnection *cnx = (GIOPConnection *)obj;
#endif

  O_MUTEX_LOCK(cnx->incoming_mutex);
  O_MUTEX_LOCK(cnx->outgoing_mutex);
  O_MUTEX_DESTROY(cnx->incoming_mutex);
  O_MUTEX_DESTROY(cnx->outgoing_mutex);

#ifdef XXX_FIXME_XXX
  if(cnx->incoming_msg)
    giop_recv_buffer_unuse(cnx->incoming_msg);

  g_list_foreach(cnx->outgoing_mutex, (GFunc)giop_send_buffer_unuse, NULL);
#endif

  if(parent_class->shutdown)
    parent_class->shutdown(obj);
}

static void
giop_connection_real_state_changed(LINCConnection *cnx, LINCConnectionStatus status)
{
  if(((LINCConnectionClass *)parent_class)->state_changed)
    ((LINCConnectionClass *)parent_class)->state_changed(cnx, status);

  if(status == LINC_CONNECTED)
    {
      if(cnx->was_initiated)
	/* Send greeting */ ;
    }
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
  GIOPConnection *cnx = (GIOPConnection *)g_object_new(giop_connection_get_type(), NULL);

  cnx->giop_version = giop_version;
  if(!linc_connection_from_fd((LINCConnection *)cnx, fd, proto, remote_host_info,
			      remote_serv_info, was_initiated, status, options))
    {
      g_object_unref(G_OBJECT(cnx)); cnx = NULL;
    }

  return cnx;
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
  GIOPConnection *cnx = (GIOPConnection *)g_object_new(giop_connection_get_type(), NULL);

  cnx->giop_version = giop_version;
  if(!linc_connection_initiate((LINCConnection *)cnx, proto_name, remote_host_info, remote_serv_info, options))
    {
      g_object_unref(G_OBJECT(cnx)); cnx = NULL;
    }

  return cnx;
}
