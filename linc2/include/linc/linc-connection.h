#ifndef LINC_CONNECTION_H
#define LINC_CONNECTION_H 1

#include <sys/uio.h>
#include <netdb.h>

#include <linc/linc-types.h>
#include <linc/linc-protocol.h>

#ifdef LINC_SSL_SUPPORT
#include <openssl/ssl.h>
#endif

#define LINC_TYPE_CONNECTION (linc_connection_get_type())
#define LINC_TYPE_IS_CONNECTION(type) (G_TYPE_FUNDAMENTAL (type) == LINC_TYPE_CONNECTION)
#define LINC_CONNECTION(object)	(LINC_IS_CONNECTION (object) ? ((GObject*) (object)) : \
				    G_TYPE_CHECK_INSTANCE_CAST ((object), LINC_TYPE_CONNECTION, LINCConnection))

#define LINC_CONNECTION_CLASS(class)	   (LINC_IS_CONNECTION_CLASS (class) ? ((GObjectClass*) (class)) : \
				    G_TYPE_CHECK_CLASS_CAST ((class), LINC_TYPE_CONNECTION, LINCConnectionClass))

#define LINC_IS_CONNECTION(object)	   (((GObject*) (object)) != NULL && \
				    LINC_IS_CONNECTION_CLASS (((GTypeInstance*) (object))->g_class))

#define LINC_IS_CONNECTION_CLASS(class)   (((GTypeClass*) (class)) != NULL && \
				    LINC_TYPE_IS_CONNECTION (((GTypeClass*) (class))->g_type))

typedef enum { LINC_CONNECTING, LINC_CONNECTED, LINC_DISCONNECTED } LINCConnectionStatus;
typedef struct {
  GObject parent;

  LINCProtocolInfo *proto;

  char *remote_host_info, *remote_serv_info;

#if LINC_SSL_SUPPORT
  SSL *ssl;
#endif

  GIOChannel *gioc;

  guint tag;
  int fd;
  LINCConnectionStatus status;
  LINCConnectionOptions options;
  guint8 was_initiated : 1, is_auth : 1;
} LINCConnection;

typedef struct {
  GObjectClass parent_class;
  void (* state_changed)(LINCConnection *cnx, LINCConnectionStatus status); /* subclasses should call parent impl first */
} LINCConnectionClass;

GType linc_connection_get_type(void) G_GNUC_CONST;

gboolean linc_connection_from_fd(LINCConnection *cnx, int fd, const LINCProtocolInfo *proto,
				 const char *remote_host_info,
				 const char *remote_serv_info,
				 gboolean was_initiated,
				 LINCConnectionStatus status,
				 LINCConnectionOptions options);
gboolean linc_connection_initiate(LINCConnection *cnx,
				  const char *proto_name,
				  const char *remote_host_info,
				  const char *remote_serv_info,
				  LINCConnectionOptions options);

int linc_connection_read(LINCConnection *cnx, guchar *buf, int len, gboolean block_for_full_read);

/* Return values from these functions are going to be "abnormal", since they make sure to write all the data out */
int linc_connection_write(LINCConnection *cnx, const guchar *buf, gulong len);
int linc_connection_writev(LINCConnection *cnx, struct iovec *vecs, int nvecs, gulong total_size);
#endif
