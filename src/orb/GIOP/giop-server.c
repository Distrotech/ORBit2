#include "config.h"
#include <orbit/IIOP/giop-server.h>
#include <netdb.h>

static void giop_server_init       (GIOPServer      *cnx);
static void giop_server_class_init (GIOPServerClass *klass);

static void giop_server_get_param(GObject        *object,
				  guint           param_id,
				  GValue         *value,
				  GParamSpec     *pspec,
				  const gchar    *trailer);
static void giop_server_set_param(GObject        *object,
				 guint           param_id,
				 GValue         *value,
				 GParamSpec     *pspec,
				 const gchar    *trailer);

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
  GParamSpec *pspec;

  object_class->get_param = giop_server_get_param;
  object_class->set_param = giop_server_set_param;

  pspec = g_param_spec_string_c("GIOPServer::proto_name",
				"proto_name", "Protocol names",
				"IPv4", G_PARAM_READABLE|G_PARAM_WRITABLE);
  g_object_class_install_param(object_class, PARAM_SA_FAMILY, pspec);
}

static void
giop_server_get_param(GObject        *object,
		      guint           param_id,
		      GValue         *value,
		      GParamSpec     *pspec,
		      const gchar    *trailer)
{
  GIOPServer *cnx = (GIOPServer *)object;

  g_return_if_fail(PARAM_SA_FAMILY == param_id);
  g_value_set_string(value, cnx->proto->name);
}

static void
giop_server_set_param(GObject        *object,
		      guint           param_id,
		      GValue         *value,
		      GParamSpec     *pspec,
		      const gchar    *trailer)
{
  GIOPServer *cnx = (GIOPServer *)object;
  int fd;
  const GIOPProtocolInfo * proto;

  g_return_if_fail(PARAM_SA_FAMILY == param_id);

  cnx->proto = giop_protocol_find(g_value_get_string(value));
  g_assert(cnx->proto);

  fd = socket(proto->family, SOCK_STREAM, proto->stream_proto_num);
  g_assert(fd >= 0);

  
}

GIOPServer *
giop_server_new(const char *proto_name, GIOPConnectionOptions create_options)
{
  GIOPServer *server;

  server = g_object_new(giop_server_get_type(), NULL);

  return server;
}
