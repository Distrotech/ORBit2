#ifndef GIOP_CONNECTION_H
#define GIOP_CONNECTION_H 1

#include <orbit/IIOP/giop-types.h>
#include <linc/linc.h>
#include <orbit/IIOP/giop-server.h>
#include <netdb.h>

typedef struct {
  LINCConnection parent;

  O_MUTEX_DEFINE(incoming_mutex);
  GIOPRecvBuffer *incoming_msg;
  O_MUTEX_DEFINE(outgoing_mutex);
  GList *outgoing_msgs;

  GIOPVersion giop_version;
} GIOPConnection;

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
