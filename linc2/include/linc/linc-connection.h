#ifndef GIOP_CONNECTION_H
#define GIOP_CONNECTION_H 1

#include <orbit/IIOP/giop-types.h>
#include <orbit/IIOP/giop-protocol.h>
#include <orbit/IIOP/giop-server.h>
#include <netdb.h>
#if ORBIT_SSL_SUPPORT
#include <openssl/ssl.h>
#endif

typedef struct {
  GObject parent;

  O_MUTEX_DEFINE(incoming_mutex);
  GIOPRecvBuffer *incoming_msg;
  O_MUTEX_DEFINE(outgoing_mutex);
  GList *outgoing_msgs;

  GIOPProtocolInfo *proto;

  char *remote_host_info, *remote_serv_info;

#if ORBIT_SSL_SUPPORT
  SSL *ssl;
#endif

  GIOPConnectionOptions options;
  GIOPVersion giop_version;
  guint tag;
  int fd;
  guint8 was_initiated : 1, is_auth : 1;
} GIOPConnection;

typedef struct {
  GObjectClass parent_class;
} GIOPConnectionClass;

GType giop_connection_get_type(void) G_GNUC_CONST;

GIOPConnection *giop_connection_from_fd(int fd, const GIOPProtocolInfo *proto, const char *remote_host_info,
					const char *remote_serv_info, gboolean was_initiated);
GIOPConnection *giop_connection_initiate(const char *proto_name, const char *remote_host_info, const char *remote_serv_info);

/* These do internal locking & caching */
void giop_connection_ref(GIOPConnection *cnx);
void giop_connection_unref(GIOPConnection *cnx);
#endif
