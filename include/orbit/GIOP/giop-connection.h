#ifndef GIOP_CONNECTION_H
#define GIOP_CONNECTION_H 1

#include <orbit/GIOP/giop-types.h>
#include <linc/linc.h>
#include <orbit/GIOP/giop-server.h>
#include <netdb.h>

#define GIOP_TYPE_CONNECTION (giop_connection_get_type())
#define GIOP_TYPE_IS_CONNECTION(type) (G_TYPE_FUNDAMENTAL (type) == GIOP_TYPE_CONNECTION)
#define GIOP_CONNECTION(object)	(GIOP_IS_CONNECTION (object) ? ((GIOPConnection*) (object)) : \
				    G_TYPE_CHECK_INSTANCE_CAST ((object), GIOP_TYPE_CONNECTION, GIOPConnection))

#define GIOP_CONNECTION_CLASS(class)	   (GIOP_IS_CONNECTION_CLASS (class) ? ((GIOPConnectionClass*) (class)) : \
				    G_TYPE_CHECK_CLASS_CAST ((class), GIOP_TYPE_CONNECTION, GIOPConnectionClass))

#define GIOP_IS_CONNECTION(object)	   (((GIOPConnection*) (object)) != NULL && \
				    GIOP_IS_CONNECTION_CLASS (((GTypeInstance*) (object))->g_class))

#define GIOP_IS_CONNECTION_CLASS(class)   (((GTypeClass*) (class)) != NULL && \
				    GIOP_TYPE_IS_CONNECTION (((GTypeClass*) (class))->g_type))

struct _GIOPConnection {
  LINCConnection parent;

  O_MUTEX_DEFINE(incoming_mutex);
  GIOPRecvBuffer *incoming_msg;
  O_MUTEX_DEFINE(outgoing_mutex);

  gpointer orb_data;

  GIOPVersion giop_version;
  guint incoming_tag;
};

typedef struct {
  LINCConnectionClass parent_class;
} GIOPConnectionClass;

GType giop_connection_get_type(void) G_GNUC_CONST;

GIOPConnection *giop_connection_from_fd(int fd, const LINCProtocolInfo *proto, const char *remote_host_info,
					const char *remote_serv_info, gboolean was_initiated,
					LINCConnectionStatus status,
					LINCConnectionOptions options,
					GIOPVersion giop_version);
GIOPConnection *giop_connection_initiate(const char *proto_name, const char *remote_host_info, const char *remote_serv_info,
					 LINCConnectionOptions options,
					 GIOPVersion giop_version);
#endif
