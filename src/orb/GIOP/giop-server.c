#include "config.h"
#include <orbit/GIOP/giop-server.h>
#include <orbit/GIOP/giop-connection.h>

static void giop_server_init       (GIOPServer      *server);
static void giop_server_class_init (GIOPServerClass *klass);
static LINCConnection *giop_server_handle_create_connection(LINCServer *server);

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
      
      object_type = g_type_register_static (linc_server_get_type(),
                                            "GIOPServer",
                                            &object_info);
    }  

  return object_type;
}

static void
giop_server_init       (GIOPServer      *server)
{
}

static void
giop_server_class_init (GIOPServerClass *klass)
{
#if 0
  GObjectClass *object_class = (GObjectClass *)klass;
#endif

  klass->parent_class.create_connection = giop_server_handle_create_connection;
}

GIOPServer *
giop_server_new(const char *proto_name, const char *local_host_info, const char *local_serv_info,
		LINCConnectionOptions create_options)
{
  GIOPServer *server = (GIOPServer *)g_object_new(giop_server_get_type(), NULL);

  linc_server_setup((LINCServer *)server, proto_name, local_host_info, local_serv_info,
		    create_options);

  return server;
}

static LINCConnection *
giop_server_handle_create_connection(LINCServer *server)
{
  GIOPConnection *retval = g_object_new(giop_connection_get_type(), NULL);

  retval->giop_version = GIOP_LATEST;

  return (LINCConnection *)retval;
}
