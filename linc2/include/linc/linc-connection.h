/*
 * linc-connection.h: This file is part of the linc library.
 *
 * Authors:
 *    Elliot Lee     (sopwith@redhat.com)
 *    Michael Meeks  (michael@ximian.com)
 *    Mark McLouglin (mark@skynet.ie) & others
 *
 * Copyright 2001, Red Hat, Inc., Ximian, Inc.,
 *                 Sun Microsystems, Inc.
 */
#ifndef _LINC_CONNECTION_H_
#define _LINC_CONNECTION_H_

#include <glib/gmacros.h>

G_BEGIN_DECLS

#include <sys/uio.h>
#include <netdb.h>

#include <linc/linc-types.h>
#include <linc/linc-protocol.h>

#define LINC_TYPE_CONNECTION            (linc_connection_get_type())
#define LINC_TYPE_IS_CONNECTION(type)   (G_TYPE_FUNDAMENTAL (type) == LINC_TYPE_CONNECTION)
#define LINC_CONNECTION(object)	        (LINC_IS_CONNECTION (object) ? ((LINCConnection*) (object)) : \
				         G_TYPE_CHECK_INSTANCE_CAST ((object), LINC_TYPE_CONNECTION, LINCConnection))
#define LINC_CONNECTION_CLASS(class)    (LINC_IS_CONNECTION_CLASS (class) ? ((LINCConnectionClass*) (class)) : \
				         G_TYPE_CHECK_CLASS_CAST ((class), LINC_TYPE_CONNECTION, LINCConnectionClass))
#define LINC_IS_CONNECTION(object)      (((LINCConnection*) (object)) != NULL && \
				         LINC_IS_CONNECTION_CLASS (((GTypeInstance*) (object))->g_class))
#define LINC_IS_CONNECTION_CLASS(class) (((GTypeClass*) (class)) != NULL && \
				         LINC_TYPE_IS_CONNECTION (((GTypeClass*) (class))->g_type))

typedef enum { LINC_CONNECTING, LINC_CONNECTED, LINC_DISCONNECTED } LINCConnectionStatus;

typedef struct _LINCConnectionPrivate LINCConnectionPrivate;

typedef struct {
	GObject                 parent;

	const LINCProtocolInfo *proto;

	LINCConnectionStatus    status;
	LINCConnectionOptions   options;
	guint                   was_initiated : 1;
	guint                   is_auth : 1;

	guchar                 *remote_host_info;
	guchar                 *remote_serv_info;

	LINCConnectionPrivate  *priv;
} LINCConnection;

typedef struct {
	GObjectClass parent_class;

	/* subclasses should call parent impl first */
	void     (* state_changed) (LINCConnection      *cnx,
				    LINCConnectionStatus status);
	gboolean (* handle_input)  (LINCConnection      *cnx);

	/* signals */
	void     (* broken)        (LINCConnection      *cnx);
} LINCConnectionClass;

GType    linc_connection_get_type (void) G_GNUC_CONST;

gboolean linc_connection_from_fd  (LINCConnection *cnx, int fd,
				   const LINCProtocolInfo *proto,
				   gchar   *remote_host_info,
				   gchar   *remote_serv_info,
				   gboolean was_initiated,
				   LINCConnectionStatus status,
				   LINCConnectionOptions options);

gboolean linc_connection_initiate (LINCConnection *cnx,
				   const char *proto_name,
				   const char *remote_host_info,
				   const char *remote_serv_info,
				   LINCConnectionOptions options);

int      linc_connection_read     (LINCConnection *cnx, guchar *buf,
				   int len, gboolean block_for_full_read);

/* Return values from these functions are going to be "abnormal",
   since they make sure to write all the data out */
int      linc_connection_write    (LINCConnection *cnx, const guchar *buf, gulong len);

int      linc_connection_writev   (LINCConnection *cnx, struct iovec *vecs,
				   int nvecs, gulong total_size);

void     linc_connection_state_changed (LINCConnection *cnx,
					LINCConnectionStatus status);

G_END_DECLS

#endif /* _LINC_CONNECTION_H */
