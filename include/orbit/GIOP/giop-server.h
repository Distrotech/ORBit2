#ifndef GIOP_SERVER_H
#define GIOP_SERVER_H 1

#include <orbit/IIOP/giop-protocol.h>

typedef struct {
  GObject parent;

  int fd;
  GMutex *mutex;

  char *local_host_info, *local_serv_info;

  GIOPProtocolInfo *proto;
  guint tag;
  /* Options that incoming connections are created with */
  GIOPConnectionOptions create_options;
} GIOPServer;

typedef struct {
  GObjectClass parent_class;
} GIOPServerClass;

GType giop_server_get_type(void) G_GNUC_CONST;
GIOPServer *giop_server_new(const char *proto_name, GIOPConnectionOptions create_options);

#endif
