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

typedef struct {
	enum {
		LINC_COMMAND_DISCONNECT,
		LINC_COMMAND_SET_CONDITION
	} type;
} LINCCommand;

typedef struct {
	LINCCommand     cmd;
	LINCConnection *cnx;
	GIOCondition    condition;
} LINCCommandSetCondition;

typedef struct {
	LINCCommand     cmd;
	LINCConnection *cnx;
} LINCCommandDisconnect;

void linc_exec_command (LINCCommand *cmd);
void linc_connection_exec_disconnect (LINCCommandDisconnect *cmd);
void linc_connection_exec_set_condition (LINCCommandSetCondition *cmd);
void _linc_connection_thread_init (gboolean thread);

/*
 * Really raw internals, exported for the tests
 */

struct _LINCServerPrivate {
	int        fd;
	LincWatch *tag;
	GSList    *connections;
};

struct _LINCWriteOpts {
	gboolean block_on_write;
};

struct _LINCConnectionPrivate {
#ifdef LINC_SSL_SUPPORT
	SSL         *ssl;
#endif
	LincWatch   *tag;
	int          fd;

	gulong       max_buffer_bytes;
	gulong       write_queue_bytes;
	GList       *write_queue;
};

typedef struct {
	GSource       source;

        GIOChannel   *channel;
	GPollFD       pollfd;
	GIOCondition  condition;
	GIOFunc       callback;
} LincUnixWatch;

struct _LincWatch {
	GSource *main_source;
	GSource *linc_source;
};

#define LINC_ERR_CONDS (G_IO_ERR|G_IO_HUP|G_IO_NVAL)
#define LINC_IN_CONDS  (G_IO_PRI|G_IO_IN)

#define LINC_CLOSE(fd)  while (close (fd) < 0 && errno == EINTR)

const char      *linc_get_local_hostname    (void);

struct sockaddr *linc_protocol_get_sockaddr (const LINCProtocolInfo *proto,
					     const char             *hostname,
					     const char             *service,
					     LincSockLen            *saddr_len);

gboolean         linc_protocol_get_sockinfo (const LINCProtocolInfo *proto,
					     const struct sockaddr  *saddr,
					     gchar                 **hostname,
					     gchar                 **service);

gboolean         linc_protocol_is_local     (const LINCProtocolInfo  *proto,
					     const struct sockaddr   *saddr,
					     LincSockLen              saddr_len);

void             linc_protocol_destroy_cnx  (const LINCProtocolInfo  *proto,
					     int                      fd,
					     const char              *host,
					     const char              *service);

void             linc_protocol_destroy_addr (const LINCProtocolInfo  *proto,
					     int                      fd,
					     struct sockaddr         *saddr);

LincWatch       *linc_io_add_watch_fd       (int                     fd,
					     GIOCondition            condition,
					     GIOFunc                 func,
					     gpointer                user_data);

void             linc_watch_set_condition   (LincWatch              *w,
					     GIOCondition            condition);

GMainContext    *linc_main_get_context      (void);
gboolean         linc_get_threaded          (void);
gboolean         linc_in_io_thread          (void);
gboolean         linc_mutex_is_locked       (GMutex *lock);

#endif /* _LINC_PRIVATE_H */
