#ifndef LINC_SERVER_H
#define LINC_SERVER_H 1

#include <linc/linc-protocol.h>
#include <linc/linc-connection.h>

#define LINC_TYPE_SERVER (linc_server_get_type())
#define LINC_TYPE_IS_SERVER(type) (G_TYPE_FUNDAMENTAL (type) == LINC_TYPE_SERVER)
#define LINC_SERVER(object)	(LINC_IS_SERVER (object) ? ((LINCServer*) (object)) : \
				    G_TYPE_CHECK_INSTANCE_CAST ((object), LINC_TYPE_SERVER, LINCServer))

#define LINC_SERVER_CLASS(class)	   (LINC_IS_SERVER_CLASS (class) ? ((LINCServerClass*) (class)) : \
				    G_TYPE_CHECK_CLASS_CAST ((class), LINC_TYPE_SERVER, LINCServerClass))

#define LINC_IS_SERVER(object)	   (((LINCServer*) (object)) != NULL && \
				    LINC_IS_SERVER_CLASS (((GTypeInstance*) (object))->g_class))

#define LINC_IS_SERVER_CLASS(class)   (((GTypeClass*) (class)) != NULL && \
				    LINC_TYPE_IS_SERVER (((GTypeClass*) (class))->g_type))

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
			   const char *local_host_info,
			   const char *local_serv_info,
			   LINCConnectionOptions create_options);

#endif
