#ifndef _LINC_PRIVATE_H_
#define _LINC_PRIVATE_H_

#include "config.h"
#include <linc/linc.h>

#ifdef LINC_SSL_SUPPORT

#include <openssl/ssl.h>
#include <openssl/bio.h>
extern SSL_METHOD *linc_ssl_method;
extern SSL_CTX *linc_ssl_ctx;

#endif /* LINC_SSL_SUPPORT */

struct sockaddr *linc_protocol_get_sockaddr (const LINCProtocolInfo *proto,
					     const char             *hostname,
					     const char             *service,
					     socklen_t              *saddr_len);

gboolean         linc_protocol_get_sockinfo (const LINCProtocolInfo *proto,
					     const struct sockaddr  *saddr,
					     gchar                 **hostname,
					     gchar                 **service);

#endif /* _LINC_PRIVATE_H */
