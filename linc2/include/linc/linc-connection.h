#ifndef LINC_CONNECTION_H
#define LINC_CONNECTION_H 1

#include <linc/linc-types.h>
#include <linc/linc-protocol.h>
#include <linc/linc-server.h>
#include <netdb.h>
#ifdef LINC_SSL_SUPPORT
#include <openssl/ssl.h>
#endif

typedef struct {
  GObject parent;

  LINCProtocolInfo *proto;

  char *remote_host_info, *remote_serv_info;

#if LINC_SSL_SUPPORT
  SSL *ssl;
#endif

  LINCConnectionOptions options;
  guint tag;
  int fd;
  guint8 was_initiated : 1, is_auth : 1;
} LINCConnection;

typedef struct {
  GObjectClass parent_class;
} LINCConnectionClass;

GType linc_connection_get_type(void) G_GNUC_CONST;

LINCConnection *linc_connection_from_fd(int fd, const LINCProtocolInfo *proto,
					const char *remote_host_info,
					const char *remote_serv_info,
					gboolean was_initiated);

LINCConnection *linc_connection_initiate(const char *proto_name,
					 const char *remote_host_info,
					 const char *remote_serv_info);

/* These do internal locking & caching */
void linc_connection_ref(LINCConnection *cnx);
void linc_connection_unref(LINCConnection *cnx);

int linc_connection_read(LINCConnection *cnx, guchar *buf, int len);
int linc_connection_write(LINCConnection *cnx, const guchar *buf, int len);
#endif
