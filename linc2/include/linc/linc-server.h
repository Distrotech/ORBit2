#ifndef GIOP_SERVER_H
#define GIOP_SERVER_H 1

#include <orbit/IIOP/giop-protocol.h>

typedef struct {
  GObject parent;

  GIOChannel *gioc;
  GMutex *mutex;

  GIOPProtocolInfo *proto;
  guint tag;
  GIOPConnectionOptions create_options; /* Options that incoming connections are created with */
} GIOPServer;

typedef struct {
  GObjectClass parent_class;
} GIOPServerClass;

GType giop_server_get_type(void) G_GNUC_CONST;
GIOPServer *giop_server_new(const char *proto_name, GIOPConnectionOptions create_options);

#endif
