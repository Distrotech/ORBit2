#include "config.h"
#include <IIOP/giop-connection.h>

static void giop_connection_init       (GIOPConnection      *cnx);
static void giop_connection_class_init (GIOPConnectionClass *klass);

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
}

static void
giop_connection_class_init (GIOPConnectionClass *klass)
{
}

GIOPConnection *
giop_connection_from_fd(int fd, struct addrinfo *ai, gboolean was_initiated, gboolean adopt_ai)
{
  GIOPConnection *cnx = g_object_new(giop_connection_get_type(), NULL);

  cnx->gioc = g_io_channel_unix_new(fd);
  cnx->was_initiated = was_initiated;
  cnx->incoming_mutex = g_mutex_new();
  cnx->outgoing_mutex = g_mutex_new();
  if(adopt_ai)
    cnx->ai = ai;
}

/* This will just create the fd, do the connect and all, and then call giop_connection_from_fd */
GIOPConnection *
giop_connection_initiate(const char *remote_host_info, const char *remote_serv_info, int family)
{
  struct addrinfo *ai, hints = {AI_CANONNAME, 0, SOCK_STREAM, 0, 0, NULL, NULL, NULL};

  hints.ai_family = family;
  getaddrinfo(remote_host_info, remote_serv_info, &hints, &ai);

  return giop_connection_from_fd(fd, ai, TRUE, TRUE);
}
