#ifndef GIOP_SERVER_H
#define GIOP_SERVER_H 1

#include <linc/linc.h>

typedef struct {
  LINCServer parent;
} GIOPServer;

typedef struct {
  LINCServerClass parent_class;
} GIOPServerClass;

GType giop_server_get_type(void) G_GNUC_CONST;
GIOPServer *giop_server_new(const char *proto_name,
			    const char *local_host_info, const char *local_serv_info,
			    LINCConnectionOptions create_options);

#endif
