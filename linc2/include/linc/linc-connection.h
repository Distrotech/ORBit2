#ifndef LINC_CONNECTION_H
#define LINC_CONNECTION_H 1

#include <sys/uio.h>
#include <netdb.h>

#include <linc/linc-types.h>
#include <linc/linc-protocol.h>

#ifdef LINC_SSL_SUPPORT
#include <openssl/ssl.h>
#endif

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

int linc_connection_read(LINCConnection *cnx, guchar *buf, int len);
int linc_connection_write(LINCConnection *cnx, const guchar *buf, int len);
int linc_connection_writev(LINCConnection *cnx, struct iovec *vecs, int nvecs);
#endif
