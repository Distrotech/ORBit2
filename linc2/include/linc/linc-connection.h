#ifndef GIOP_CONNECTION_H
#define GIOP_CONNECTION_H 1

#include <IIOP/giop-types.h>

typedef struct {
  GObject parent;
  GIOChannel *gioc;
  guint tag;
  GIOPRecvBuffer *incoming_msg;
  GList *outgoing_msgs;
  GMutex *incoming_mutex, *outgoing_mutex;
  struct addrinfo *ai;
  gpointer addr; guint16 addr_len;
  guint8 was_initiated : 1, is_auth : 1;
} GIOPConnection;

typedef struct {
  GObjectClass parent_class;
} GIOPConnectionClass;

GType giop_connection_get_type(void);
GIOPConnection *giop_connection_from_fd(int fd, struct addrinfo *ai, gboolean was_initiated, gboolean adopt_ai);
GIOPConnection *giop_connection_initiate(const char *remote_host_info, const char *remote_serv_info, int family);

#endif
