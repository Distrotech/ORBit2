#ifndef GIOP_SERVER_H
#define GIOP_SERVER_H 1

#include <linc/linc.h>

#define GIOP_TYPE_SERVER (giop_server_get_type())
#define GIOP_TYPE_IS_SERVER(type) (G_TYPE_FUNDAMENTAL (type) == GIOP_TYPE_SERVER)
#define GIOP_SERVER(object)	(GIOP_IS_SERVER (object) ? ((GObject*) (object)) : \
				    G_TYPE_CHECK_INSTANCE_CAST ((object), GIOP_TYPE_SERVER, GIOPServer))

#define GIOP_SERVER_CLASS(class)	   (GIOP_IS_SERVER_CLASS (class) ? ((GObjectClass*) (class)) : \
				    G_TYPE_CHECK_CLASS_CAST ((class), GIOP_TYPE_SERVER, GIOPServerClass))

#define GIOP_IS_SERVER(object)	   (((GObject*) (object)) != NULL && \
				    GIOP_IS_SERVER_CLASS (((GTypeInstance*) (object))->g_class))

#define GIOP_IS_SERVER_CLASS(class)   (((GTypeClass*) (class)) != NULL && \
				    GIOP_TYPE_IS_SERVER (((GTypeClass*) (class))->g_type))

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
