#include "config.h"
#include <orbit/GIOP/giop.h>
#include <orbit/GIOP/giop-connection.h>

#ifdef G_ENABLE_DEBUG
void (*giop_debug_hook_new_connection) (GIOPServer     *server,
					GIOPConnection *new_cnx) = NULL;
#endif

GIOPServer *
giop_server_new (GIOPVersion            giop_version,
		 const char            *proto_name, 
		 const char            *local_host_info,
		 const char            *local_serv_info,
		 LINCConnectionOptions  create_options,
		 gpointer               create_orb_data)
{
	GIOPServer *server = (GIOPServer *)
		g_object_new (GIOP_TYPE_SERVER, NULL);

#ifndef ORBIT_THREADED
	create_options |= LINC_CONNECTION_NONBLOCKING;
#endif

	server->giop_version = giop_version;

	if (!linc_server_setup (LINC_SERVER (server), proto_name, 
				local_host_info, local_serv_info, 
				create_options)) {

		g_object_unref (G_OBJECT (server));

		return NULL;
	} else
		server->orb_data = create_orb_data;

	return server;
}

static LINCConnection *
giop_server_handle_create_connection (LINCServer *server)
{
	GIOPConnection *retval;
	GIOPServer     *gserver = (GIOPServer *) server;

	retval = g_object_new (giop_connection_get_type (), NULL);

	retval->giop_version = gserver->giop_version;
	retval->orb_data     = gserver->orb_data;

#ifdef G_ENABLE_DEBUG
	if (giop_debug_hook_new_connection)
		giop_debug_hook_new_connection (gserver, retval);
#endif	

	return (LINCConnection *)retval;
}

static void
giop_server_class_init (GIOPServerClass *klass)
{
	klass->parent_class.create_connection = giop_server_handle_create_connection;
}

GType
giop_server_get_type(void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
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
      
		object_type = g_type_register_static (
			linc_server_get_type (),
			"GIOPServer",
			&object_info, 0);
	}  

	return object_type;
}
