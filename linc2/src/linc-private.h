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
#ifndef _LINK_PRIVATE_H_
#define _LINK_PRIVATE_H_

#include "config.h"
#include <linc/linc.h>

#ifdef LINK_SSL_SUPPORT

#include <openssl/ssl.h>
#include <openssl/bio.h>
extern SSL_METHOD *link_ssl_method;
extern SSL_CTX *link_ssl_ctx;

#endif /* LINK_SSL_SUPPORT */

typedef struct {
	enum {
		LINK_COMMAND_DISCONNECT,
		LINK_COMMAND_SET_CONDITION,
		LINK_COMMAND_SET_IO_THREAD
	} type;
} LinkCommand;

typedef struct {
	LinkCommand     cmd;
	gboolean        complete;
} LinkSyncCommand;

typedef struct {
	LinkCommand     cmd;
	LinkConnection *cnx;
	GIOCondition    condition;
} LinkCommandSetCondition;

typedef struct {
	LinkCommand     cmd;
	LinkConnection *cnx;
} LinkCommandDisconnect;

void link_exec_command (LinkCommand *cmd);
void link_connection_exec_disconnect (LinkCommandDisconnect *cmd, gboolean immediate);
void link_connection_exec_set_condition (LinkCommandSetCondition *cmd, gboolean immediate);

/*
 * Really raw internals, exported for the tests
 */

struct _LinkServerPrivate {
	int        fd;
	LinkWatch *tag;
	GSList    *connections;
};

struct _LinkWriteOpts {
	gboolean block_on_write;
};

struct _LinkConnectionPrivate {
#ifdef LINK_SSL_SUPPORT
	SSL         *ssl;
#endif
	LinkWatch   *tag;
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
	gpointer      user_data;
} LinkUnixWatch;

struct _LinkWatch {
	GSource *main_source;
	GSource *link_source;
};

#define LINK_ERR_CONDS (G_IO_ERR|G_IO_HUP|G_IO_NVAL)
#define LINK_IN_CONDS  (G_IO_PRI|G_IO_IN)

#define LINK_CLOSE(fd)  while (close (fd) < 0 && errno == EINTR)

const char      *link_get_local_hostname    (void);

struct sockaddr *link_protocol_get_sockaddr (const LinkProtocolInfo *proto,
					     const char             *hostname,
					     const char             *service,
					     LinkSockLen            *saddr_len);

gboolean         link_protocol_get_sockinfo (const LinkProtocolInfo *proto,
					     const struct sockaddr  *saddr,
					     gchar                 **hostname,
					     gchar                 **service);

gboolean         link_protocol_is_local     (const LinkProtocolInfo  *proto,
					     const struct sockaddr   *saddr,
					     LinkSockLen              saddr_len);

void             link_protocol_destroy_cnx  (const LinkProtocolInfo  *proto,
					     int                      fd,
					     const char              *host,
					     const char              *service);

void             link_protocol_destroy_addr (const LinkProtocolInfo  *proto,
					     int                      fd,
					     struct sockaddr         *saddr);

LinkWatch       *link_io_add_watch_fd       (int                     fd,
					     GIOCondition            condition,
					     GIOFunc                 func,
					     gpointer                user_data);

void             link_io_remove_watch       (LinkWatch              *w);
void             link_watch_set_condition   (LinkWatch              *w,
					     GIOCondition            condition);
void             link_watch_move_io         (LinkWatch              *w,
					     gboolean                to_io_thread);

GMainContext    *link_main_get_context      (void);
GMainContext    *link_thread_io_context     (void);
gboolean         link_in_io_thread          (void);
gboolean         link_mutex_is_locked       (GMutex *lock);
void             link_lock                  (void);
void             link_unlock                (void);
gboolean         link_is_locked             (void);
void             link_servers_move_io_T     (gboolean to_io_thread);
void             link_connections_move_io_T (gboolean to_io_thread);

#endif /* _LINK_PRIVATE_H */
