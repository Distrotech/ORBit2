/*
 * linc-server.c: This file is part of the linc library.
 *
 * Authors:
 *    Elliot Lee     (sopwith@redhat.com)
 *    Michael Meeks  (michael@ximian.com)
 *    Mark McLouglin (mark@skynet.ie) & others
 *
 * Copyright 2001, Red Hat, Inc., Ximian, Inc.,
 *                 Sun Microsystems, Inc.
 */
#include <config.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <linc/linc.h>
#include <linc/linc-server.h>
#include <linc/linc-connection.h>

#include "linc-private.h"

#undef DEBUG

enum {
	NEW_CONNECTION,
	LAST_SIGNAL
};
static guint server_signals [LAST_SIGNAL] = { 0 };

static GObjectClass *parent_class = NULL;

static void
my_cclosure_marshal_VOID__OBJECT (GClosure     *closure,
                                  GValue       *return_value,
                                  guint         n_param_values,
                                  const GValue *param_values,
                                  gpointer      invocation_hint,
                                  gpointer      marshal_data)
{
	typedef void (*GSignalFunc_VOID__OBJECT) (gpointer     data1,
						  GObject     *arg_1,
						  gpointer     data2);
	register GSignalFunc_VOID__OBJECT callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;

	g_return_if_fail (n_param_values >= 2);

	if (G_CCLOSURE_SWAP_DATA (closure)) {
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	} else {
		data1 = g_value_peek_pointer (param_values + 0);
		data2 = closure->data;
	}
	callback = (GSignalFunc_VOID__OBJECT) (
		marshal_data ? marshal_data : cc->callback);

	callback (data1,
		  g_value_peek_pointer (param_values + 1),
		  data2);
}

static void
linc_server_init (LINCServer *cnx)
{
	cnx->mutex = linc_mutex_new ();
	cnx->fd = -1;
}

static void
linc_server_dispose (GObject *obj)
{
	LINCServer *cnx = (LINCServer *)obj;

#ifdef G_THREADS_ENABLED
	if (cnx->mutex)
		g_mutex_free (cnx->mutex);
#endif
	if (cnx->tag)
		linc_io_remove_watch (cnx->tag);
	cnx->tag = NULL;

	if (cnx->proto && cnx->proto->destroy)
		cnx->proto->destroy (
			cnx->fd, cnx->local_host_info,
			cnx->local_serv_info);
	cnx->proto = NULL;

	if (cnx->fd >= 0)
		close (cnx->fd);
	cnx->fd = -1;

	g_free (cnx->local_host_info);
	cnx->local_host_info = NULL;

	g_free (cnx->local_serv_info);
	cnx->local_serv_info = NULL;

	if (parent_class->dispose)
		parent_class->dispose (obj);
}

static LINCConnection *
linc_server_create_connection (LINCServer *cnx)
{
	return g_object_new (linc_connection_get_type (), NULL);
}

static gboolean
linc_server_accept_connection (LINCServer *server, LINCConnection **connection)
{
	LINCServerClass *klass;
	struct sockaddr *saddr;
	int              addrlen, fd;
	char            *hostname;
	char            *service;
	
	g_return_val_if_fail (connection != NULL, FALSE);

	addrlen = server->proto->addr_len;
	saddr = g_alloca (addrlen);

	fd = accept (server->fd, saddr, &addrlen);
	if (fd < 0)
		return FALSE; /* error */

	if (!linc_protocol_get_sockinfo (server->proto, saddr, &hostname, &service)) {
		close (fd);
		return FALSE;
	}

	klass = (LINCServerClass *) G_OBJECT_GET_CLASS (server);

	g_assert (klass->create_connection);
	*connection = klass->create_connection (server);

	g_return_val_if_fail (*connection != NULL, FALSE);

	if (!linc_connection_from_fd (
		*connection, fd, server->proto, hostname, service,
		FALSE, LINC_CONNECTED, server->create_options)) {

		g_object_unref (G_OBJECT (*connection));
		*connection = NULL;

		g_free (hostname);
		g_free (service);

		return FALSE;
	}

	return TRUE;
}

static gboolean
linc_server_handle_io (GIOChannel  *gioc,
		       GIOCondition condition,
		       gpointer     data)
{
	gboolean        accepted;
	LINCServer     *server = data;
	LINCConnection *connection;

	if (condition != G_IO_IN)
		g_error ("condition on server fd is %#x", condition);

	LINC_MUTEX_LOCK (server->mutex);

	accepted = linc_server_accept_connection (server, &connection);

	LINC_MUTEX_UNLOCK (server->mutex);

	if (!accepted) {
		GValue parms[2];

		memset (parms, 0, sizeof (parms));
		g_value_init (parms, G_OBJECT_TYPE (server));
		g_value_set_object (parms, G_OBJECT (server));
		g_value_init (parms + 1, G_TYPE_OBJECT);
		g_value_set_object (parms + 1, G_OBJECT (connection));
		
		g_signal_emitv (parms, server_signals [NEW_CONNECTION], 0, NULL);
		
		g_value_unset (parms);
		g_value_unset (parms + 1);
	}

	return TRUE;
}

void
linc_server_handle (LINCServer *cnx)
{
	linc_server_handle_io (NULL, G_IO_IN, cnx);
}

gboolean
linc_server_setup (LINCServer            *cnx,
		   const char            *proto_name,
		   const char            *local_host_info,
		   const char            *local_serv_info,
		   LINCConnectionOptions  create_options)
{
	const LINCProtocolInfo *proto;
	GIOChannel             *gioc;
	int                     fd, n;
	struct sockaddr        *saddr;
	socklen_t               saddr_len;
	char                    hnbuf[NI_MAXHOST];
	char                   *service, *hostname;

#if !LINC_SSL_SUPPORT
	if (create_options & LINC_CONNECTION_SSL)
		return FALSE;
#endif

	proto = linc_protocol_find (proto_name);
	if (!proto) {
#ifdef DEBUG
		fprintf (stderr, "Can't find proto '%s'\n", proto_name);
#endif
		return FALSE;
	}

	if (!local_host_info) {
		local_host_info = hnbuf;
		if (gethostname (hnbuf, sizeof (hnbuf)))
			perror ("gethostname failed!");
	}

 address_in_use:

	saddr = linc_protocol_get_sockaddr (proto, local_host_info, 
					    local_serv_info, &saddr_len);

	if (!saddr) {
#ifdef DEBUG
		fprintf (stderr, "Can't get_sockaddr proto '%s' '%s'\n",
			 local_host_info,
			 local_serv_info ? local_serv_info : "(null)");
#endif
		return FALSE;
	}

	fd = socket (proto->family, SOCK_STREAM, 
		     proto->stream_proto_num);
	if (fd < 0) {
		g_free (saddr);
#ifdef DEBUG
		fprintf (stderr, "socket (%d, %d, %d) failed\n",
			 proto->family, SOCK_STREAM, 
			 proto->stream_proto_num);
#endif
		return FALSE;
	}

	{
		static const int oneval = 1;

		setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &oneval, sizeof (oneval));
	}
    
	n = 0;
	errno = 0;

	if ((proto->flags & LINC_PROTOCOL_NEEDS_BIND) || local_serv_info)
		n = bind (fd, saddr, saddr_len);

	if (n && errno == EADDRINUSE) {
#ifdef DEBUG
		perror ("bind failed, retrying.");
#endif
		goto address_in_use;
	}

	if (!n)
		n = listen (fd, 10);
#ifdef DEBUG
	else {
		fprintf (stderr, "Errno: %d (0x%x)\n", errno, errno);
		perror ("bind really failed");
	}
#endif

	/*
	 * FIXME: If we get EINUSE we should loop to getaddrinfo ?
	 */

	if (!n)
		n = getsockname (fd, saddr, &saddr_len);
#ifdef DEBUG
	else
		perror ("listen failed");
#endif

	if (n) {
		close(fd);
#ifdef DEBUG
		perror ("get_sockname failed");
#endif
		return FALSE;
	}

	if (!linc_protocol_get_sockinfo (proto, saddr, &hostname, &service)) {

		g_free (saddr);
#ifdef DEBUG
		fprintf (stderr, "linc_getsockinfo failed.\n");
#endif
		return FALSE;
	}

	g_free (saddr);

	cnx->proto = proto;
	cnx->fd = fd;

	if ((create_options & LINC_CONNECTION_NONBLOCKING)) {
		gioc = g_io_channel_unix_new (fd);

		g_assert (cnx->tag == NULL);

		cnx->tag = linc_io_add_watch (
			gioc, 
			G_IO_IN|G_IO_HUP|G_IO_ERR|G_IO_NVAL,
			linc_server_handle_io, cnx);

		g_io_channel_unref(gioc);
	}

	cnx->create_options = create_options;
	cnx->local_host_info = hostname;
	cnx->local_serv_info = service;

	return TRUE;
}

static void
linc_server_class_init (LINCServerClass *klass)
{
	GType         ptype;
	GClosure     *closure;
	GObjectClass *object_class = (GObjectClass *) klass;

	object_class->dispose    = linc_server_dispose;
	klass->create_connection = linc_server_create_connection;

	parent_class = g_type_class_peek_parent (klass);
	closure = g_signal_type_cclosure_new (
		G_OBJECT_CLASS_TYPE (klass),
		G_STRUCT_OFFSET (LINCServerClass, new_connection));

	ptype = G_TYPE_OBJECT;
	server_signals [NEW_CONNECTION] = g_signal_newv (
		"new_connection",
		G_OBJECT_CLASS_TYPE (klass),
		G_SIGNAL_RUN_LAST, closure,
		NULL, NULL,
		my_cclosure_marshal_VOID__OBJECT,
		G_TYPE_NONE,
		1, &ptype);
}

GType
linc_server_get_type (void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof (LINCServerClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) linc_server_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (LINCServer),
			0,              /* n_preallocs */
			(GInstanceInitFunc) linc_server_init,
		};
      
		object_type = g_type_register_static (
			G_TYPE_OBJECT, "LINCServer",
			&object_info, 0);
	}  

	return object_type;
}
