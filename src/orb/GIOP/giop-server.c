#include "config.h"
#include <orbit/GIOP/giop.h>
#include <orbit/GIOP/giop-connection.h>

static GObjectClass *parent_class = NULL;

GIOPServer *
giop_server_new(GIOPVersion giop_version,
		const char *proto_name, const char *local_host_info,
		const char *local_serv_info,
		LINCConnectionOptions create_options,
		gpointer create_orb_data)
{
  GIOPServer *server = (GIOPServer *)g_object_new(giop_server_get_type(), NULL);

  server->giop_version = giop_version;
  if(!linc_server_setup((LINCServer *)server, proto_name, local_host_info, local_serv_info,
			create_options))
    {
      g_object_unref((GObject *)server); server = NULL;
    }
  else
    {
      server->orb_data = create_orb_data;
    }

  return server;
}

static LINCConnection *
giop_server_handle_create_connection(LINCServer *server)
{
  GIOPConnection *retval = g_object_new(giop_connection_get_type(), NULL);
  GIOPServer *gserver = (GIOPServer *)server;

  retval->giop_version = gserver->giop_version;
  retval->orb_data = gserver->orb_data;

  return (LINCConnection *)retval;
}

/* Sucking */
extern void giop_connection_list_add(GIOPConnection *cnx);

static void
giop_server_handle_new_connection (LINCServer *server, LINCConnection *cnx)
{
  /* FIXME: This sucks great rocks through a straw - serious
     encapsulation breakdown: hence the nasty private externs */
  giop_connection_list_add (GIOP_CONNECTION (cnx));

  if (((LINCServerClass *)parent_class)->new_connection)
    ((LINCServerClass *)parent_class)->new_connection (server, cnx);
}

static void
giop_server_destroy (GObject *obj)
{
  if(parent_class->shutdown)
    parent_class->shutdown(obj);
}

static void
giop_server_class_init (GIOPServerClass *klass)
{
  parent_class = g_type_class_peek_parent(klass);

  G_OBJECT_CLASS(klass)->shutdown = giop_server_destroy;

  klass->parent_class.create_connection = giop_server_handle_create_connection;
  klass->parent_class.new_connection    = giop_server_handle_new_connection;
}

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
        (GInstanceInitFunc) NULL,
      };
      
      object_type = g_type_register_static (linc_server_get_type(),
                                            "GIOPServer",
                                            &object_info,
					    0);
    }  

  return object_type;
}
