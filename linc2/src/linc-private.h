/*
 * linc-private.h: This file is part of the linc library.
 *
 * Authors:
 *    Elliot Lee     (sopwith@redhat.com)
 *    Michael Meeks  (michael@ximian.com)
 *    Mark McLouglin (mark@skynet.ie) & others
 *
 * Copyright 2001, Red Hat, Inc., Ximian, Inc.,
 *                 Sun Microsystems, Inc.
 */
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

#define LINC_ERR_CONDS (G_IO_ERR|G_IO_HUP|G_IO_NVAL)
#define LINC_IN_CONDS  (G_IO_PRI|G_IO_IN)

struct sockaddr *linc_protocol_get_sockaddr (const LINCProtocolInfo *proto,
					     const char             *hostname,
					     const char             *service,
					     socklen_t              *saddr_len);

gboolean         linc_protocol_get_sockinfo (const LINCProtocolInfo *proto,
					     const struct sockaddr  *saddr,
					     gchar                 **hostname,

					     gchar                 **service);

/* Redundant - to go. */
void     linc_server_handle   (LINCServer *cnx);

#endif /* _LINC_PRIVATE_H */
