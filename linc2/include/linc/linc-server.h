#ifndef LINC_SERVER_H
#define LINC_SERVER_H 1

#include <linc/linc-protocol.h>
#include <linc/linc-connection.h>

typedef struct {
  GObject parent;

  int fd;
  O_MUTEX_DEFINE(mutex);

  char *local_host_info, *local_serv_info;

  const LINCProtocolInfo * proto;
  guint tag;
  /* Options that incoming connections are created with */
  LINCConnectionOptions create_options;
} LINCServer;

typedef struct {
  GObjectClass parent_class;

  LINCConnection *(* create_connection) (LINCServer *server);
} LINCServerClass;

GType linc_server_get_type(void) G_GNUC_CONST;
gboolean linc_server_setup(LINCServer *cnx, const char *proto_name,
			 const char *local_host_info, const char *local_serv_info,
			 LINCConnectionOptions create_options);

#endif
