#ifndef LINC_SERVER_H
#define LINC_SERVER_H 1

#include <linc/linc-protocol.h>

typedef struct {
  GObject parent;

  int fd;
  GMutex *mutex;

  char *local_host_info, *local_serv_info;

  LINCProtocolInfo *proto;
  guint tag;
  /* Options that incoming connections are created with */
  LINCConnectionOptions create_options;
} LINCServer;

typedef struct {
  GObjectClass parent_class;
} LINCServerClass;

GType linc_server_get_type(void) G_GNUC_CONST;
LINCServer *linc_server_new(const char *proto_name,
			    LINCConnectionOptions create_options);

#endif
