/*
 * linc-connection.c: This file is part of the linc library.
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

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifdef LINC_SSL_SUPPORT
#include <openssl/ssl.h>
#endif

#include "linc-debug.h"
#include "linc-private.h"
#include <linc/linc-config.h>
#include <linc/linc-connection.h>

struct _LINCConnectionPrivate {
#ifdef LINC_SSL_SUPPORT
	SSL        *ssl;
#endif
	GIOChannel *gioc;
	LincWatch  *tag;
	int         fd;
};

static GObjectClass *parent_class = NULL;

enum {
	BROKEN,
	LAST_SIGNAL
};
static guint linc_connection_signals [LAST_SIGNAL];


static gboolean linc_connection_connected (GIOChannel  *gioc,
					   GIOCondition condition,
					   gpointer     data);

static void
linc_source_remove (LINCConnection *cnx)
{
	if (cnx->priv->tag) {
		LincWatch *thewatch = cnx->priv->tag;
		cnx->priv->tag = NULL;
		linc_io_remove_watch (thewatch);
		d_printf ("Removed watch on %d\n", cnx->priv->fd);
	}
}

static void
linc_source_add (LINCConnection *cnx,
		 GIOCondition    condition)
{
	g_assert (cnx->priv->tag == NULL);

	cnx->priv->tag = linc_io_add_watch (
		cnx->priv->gioc, condition,
		linc_connection_connected, cnx);

	d_printf ("Added watch on %d (0x%x)\n",
		 cnx->priv->fd, condition);
}

static void
linc_close_fd (LINCConnection *cnx)
{
	if (cnx->priv->fd >= 0) {
		d_printf ("Close %d\n", cnx->priv->fd);
		close (cnx->priv->fd);
	}
	cnx->priv->fd = -1;
}

static void
linc_connection_dispose (GObject *obj)
{
	LINCConnection *cnx = (LINCConnection *)obj;

	linc_source_remove (cnx);

	if (cnx->priv->gioc) {
		g_io_channel_unref (cnx->priv->gioc);
		cnx->priv->gioc = NULL;
	}

	parent_class->dispose (obj);
}

static void
linc_connection_finalize (GObject *obj)
{
	LINCConnection *cnx = (LINCConnection *)obj;

	linc_close_fd (cnx);

	g_free (cnx->remote_host_info);
	g_free (cnx->remote_serv_info);

	g_free (cnx->priv);

	parent_class->finalize (obj);
}


/*
 * FIXME: a horribly inefficient way to simply stop
 * listening for G_IO_OUT - which we use initialy to
 * be notified of connection.
 */
static void
linc_source_unwatch_out (LINCConnection *cnx)
{
	linc_source_remove (cnx);
	linc_source_add (cnx, LINC_ERR_CONDS | LINC_IN_CONDS);
}

static gboolean
linc_connection_connected (GIOChannel  *gioc,
			   GIOCondition condition,
			   gpointer     data)
{
	LINCConnection      *cnx = data;
	LINCConnectionClass *klass;
	int rv, n;
	socklen_t n_size = sizeof(n);

	g_object_ref (G_OBJECT (cnx));

	klass = LINC_CONNECTION_CLASS (G_OBJECT_GET_CLASS (data));

	if (cnx->status == LINC_CONNECTED &&
	    (condition & LINC_IN_CONDS) &&
	    klass->handle_input) {

		d_printf ("Handle input on fd %d\n", cnx->priv->fd);
		klass->handle_input (cnx);

	} else if (condition & (LINC_ERR_CONDS | G_IO_OUT)) {

		switch (cnx->status) {
		case LINC_CONNECTING:
			n = 0;
			rv = getsockopt (cnx->priv->fd, SOL_SOCKET, SO_ERROR, &n, &n_size);
			if (!rv && !n && condition == G_IO_OUT) {
				d_printf ("State changed to connected on %d\n", cnx->priv->fd);
				linc_source_unwatch_out (cnx);
				linc_connection_state_changed (cnx, LINC_CONNECTED);
				
			} else {
				d_printf ("Error connecting %d %d %d on fd %d\n",
					   rv, n, errno, cnx->priv->fd);
				linc_connection_state_changed (cnx, LINC_DISCONNECTED);
			}
			break;
		case LINC_CONNECTED: {
			d_printf ("Disconnect %d\n", cnx->priv->fd);
			linc_connection_state_changed (cnx, LINC_DISCONNECTED);
			break;
		}
		default:
			break;
		}
	} else
		g_warning ("Dropping state on fd %d on the floor (%d)",
			   cnx->priv->fd, condition);

	g_object_unref (G_OBJECT (cnx));

	return TRUE;
}

/*
 * linc_connection_class_state_changed:
 * @cnx: a #LINCConnection
 * @status: a #LINCConnectionStatus value.
 *
 * Set up linc's #GSources if the connection is in the #LINC_CONNECTED
 * or #LINC_CONNECTING state.
 *
 * Remove the #GSources if the state has channged to #LINC_DISCONNECTED,
 * close the socket and a gobject broken signal which may be caught by
 * the application.
 *
 * Also perform SSL specific operations if the connection has move into
 * the #LINC_CONNECTED state.
 */

static void
linc_connection_class_state_changed (LINCConnection      *cnx,
				     LINCConnectionStatus status)
{
	d_printf ("State changing from '%s' to '%s' on fd %d\n",
		 STATE_NAME (cnx->status), STATE_NAME (status),
		 cnx->priv->fd);

	switch (status) {
	case LINC_CONNECTED:
#ifdef LINC_SSL_SUPPORT
		if (cnx->options & LINC_CONNECTION_SSL) {
			if (cnx->was_initiated)
				SSL_connect (cnx->priv->ssl);
			else
				SSL_accept (cnx->priv->ssl);
		}
#endif
		if (!cnx->priv->tag)
			linc_source_add (cnx, LINC_ERR_CONDS | LINC_IN_CONDS);
		break;

	case LINC_CONNECTING:
		/* FIXME: We might be re-connecting, and need to watch
		 * G_IO_OUT - again this could be more efficient */
		linc_source_remove (cnx);
		linc_source_add (cnx, G_IO_OUT | LINC_ERR_CONDS);
		break;

	case LINC_DISCONNECTED:
		linc_source_remove (cnx);
		linc_close_fd (cnx);

		g_signal_emit (G_OBJECT (cnx),
			       linc_connection_signals [BROKEN], 0);
		break;
	}

	cnx->status = status;
}

/*
 * linc_connection_from_fd:
 * @cnx: a #LINCConnection.
 * @fd: a connected/connecting file descriptor.
 * @proto: a #LINCProtocolInfo.
 * @remote_host_info: protocol dependant host information; gallocation swallowed
 * @remote_serv_info: protocol dependant service information(e.g. port number). gallocation swallowed
 * @was_initiated: #TRUE if the connection was initiated by us.
 * @status: a #LINCConnectionStatus value.
 * @options: combination of #LINCConnectionOptions.
 *
 * Fill in @cnx, call protocol specific initialisation methonds and then
 * call linc_connection_state_changed.
 *
 * Return Value: #TRUE if the function succeeds, #FALSE otherwise.
 */
gboolean
linc_connection_from_fd (LINCConnection         *cnx,
			 int                     fd,
			 const LINCProtocolInfo *proto,
			 gchar                  *remote_host_info,
			 gchar                  *remote_serv_info,
			 gboolean                was_initiated,
			 LINCConnectionStatus    status,
			 LINCConnectionOptions   options)
{
	cnx->was_initiated = was_initiated;
	cnx->is_auth       = (proto->flags & LINC_PROTOCOL_SECURE);
	cnx->proto         = proto;
	cnx->options       = options;
	cnx->priv->fd      = fd;
	cnx->priv->gioc    = g_io_channel_unix_new (fd);

	cnx->remote_host_info = remote_host_info;
	cnx->remote_serv_info = remote_serv_info;

	d_printf ("Cnx from fd (%d) '%s', '%s', '%s'\n",
		 fd, proto->name, 
		 remote_host_info ? remote_host_info : "<Null>",
		 remote_serv_info ? remote_serv_info : "<Null>");

	if (proto->setup)
		proto->setup (fd, options);

#ifdef LINC_SSL_SUPPORT
	if (options & LINC_CONNECTION_SSL) {
		cnx->priv->ssl = SSL_new (linc_ssl_ctx);
		SSL_set_fd (cnx->priv->ssl, fd);
	}
#endif

	linc_connection_state_changed (cnx, status);

  	return TRUE;
}

/*
 * linc_connection_initiate:
 * @cnx: a #LINCConnection.
 * @proto_name: the name of the protocol to use.
 * @host: protocol dependant host information.
 * @service: protocol dependant service information(e.g. port number).
 * @options: combination of #LINCConnectionOptions.
 *
 * Initiate a connection to @service on @host using the @proto_name protocol.
 *
 * Note: this function may be successful without actually having connected
 *       to @host - the connection handshake may not have completed.
 *
 * Return Value: #TRUE if the function succeeds, #FALSE otherwise.
 */
gboolean
linc_connection_initiate (LINCConnection        *cnx,
			  const char            *proto_name,
			  const char            *host,
			  const char            *service,
			  LINCConnectionOptions  options)
{
	const LINCProtocolInfo *proto;
	int                     rv;
	int                     fd;
	gboolean                retval = FALSE;
	struct sockaddr        *saddr;
	socklen_t		saddr_len;

	proto = linc_protocol_find (proto_name);

	if (!proto)
		return FALSE;


	saddr = linc_protocol_get_sockaddr (
		proto, host, service, &saddr_len);

	if (!saddr)
		return FALSE;

	fd = socket (proto->family, SOCK_STREAM, 
		     proto->stream_proto_num);

	if (fd < 0)
		goto out;

	if (options & LINC_CONNECTION_NONBLOCKING)
		if (fcntl (fd, F_SETFL, O_NONBLOCK) < 0)
			goto out;

	if (fcntl (fd, F_SETFD, FD_CLOEXEC) < 0)
		goto out;

	rv = connect (fd, saddr, saddr_len);
	if (rv && errno != EINPROGRESS)
		goto out;

	d_printf ("initiate 'connect' on new fd %d [ %d; %d ]\n",
		 fd, rv, errno);

	retval = linc_connection_from_fd (
		cnx, fd, proto, 
		g_strdup (host), g_strdup (service),
		TRUE, rv ? LINC_CONNECTING : LINC_CONNECTED,
		options);

 out:
	if (!cnx && fd >= 0) {
		d_printf ("initiation failed\n");
		close (fd);
	}

	g_free (saddr);

	return retval;
}

/*
 * linc_connection_state_changed:
 * @cnx: a #LINCConnection.
 * @status: a #LINCConnectionStatus.
 *
 * A wrapper for the #LINCConnectionClass's state change method.
 */
void
linc_connection_state_changed (LINCConnection      *cnx,
			       LINCConnectionStatus status)
{
	LINCConnectionClass *klass;

	klass = (LINCConnectionClass *)G_OBJECT_GET_CLASS (cnx);

	if (klass->state_changed)
		klass->state_changed (cnx, status);
}

/**
 * linc_connection_read:
 * @cnx: the connection to write to
 * @buf: a pointer to the start of an array of bytes to read data into
 * @len: the length of the array in bytes to read ingo
 * @block_for_full_read: whether to block for a full read
 * 
 * Warning, block_for_full_read is of limited usefullness.
 *
 * Return value: number of bytes written on success; negative on error.
 **/
int
linc_connection_read (LINCConnection *cnx,
		      guchar         *buf,
		      int             len,
		      gboolean        block_for_full_read)
{
	int bytes_read = 0;

	d_printf ("Read up to %d bytes from fd %d\n", len, cnx->priv->fd);

	if (!len)
		return 0;

	if (cnx->status != LINC_CONNECTED)
		return -1;

	do {
		int n;

#ifdef LINC_SSL_SUPPORT
		if (cnx->options & LINC_CONNECTION_SSL)
			n = SSL_read (cnx->priv->ssl, buf, len);
		else
#endif
			n = read (cnx->priv->fd, buf, len);

		if (n < 0) {
#ifdef LINC_SSL_SUPPORT
			if (cnx->options & LINC_CONNECTION_SSL) {
				gulong rv;

				rv = SSL_get_error (cnx->priv->ssl, n);

				if ((rv == SSL_ERROR_WANT_READ ||
				     rv == SSL_ERROR_WANT_WRITE) &&
				    (cnx->options & LINC_CONNECTION_NONBLOCKING))
					return bytes_read;
				else
					return -1;
			} else
#endif
			{
				if (errno == EINTR)
					continue;

				else if (errno == EAGAIN &&
					 (cnx->options & LINC_CONNECTION_NONBLOCKING))
					return bytes_read;

				else if (errno == EBADF) {
					g_warning ("Serious fd usage error %d", cnx->priv->fd);
					return -1;

				} else
					return -1;
			}

		} else if (n == 0) {
			if (cnx->options & LINC_CONNECTION_NONBLOCKING)
				return -1;
		} else {
			buf += n;
			len -= n;
			bytes_read += n;
		}
	} while (len > 0 && block_for_full_read);

	d_printf ("we read %d bytes\n", bytes_read);

	return bytes_read;
}

/**
 * linc_connection_write:
 * @cnx: the connection to write to
 * @buf: a pointer to the start of an array of bytes
 * @len: the length of the array in bytes
 * 
 * Writes a contiguous block of data to the abstract connection.
 * 
 * FIXME: it allows re-enterancy via linc_connection_iterate
 *        in certain cases.
 * FIXME: on this basis, the connection can die underneath
 *        our feet eg. between the main_iteration and the
 *        g_return_if_fail.
 *
 * Return value: 0 on success, non 0 on error.
 **/
int
linc_connection_write (LINCConnection *cnx,
		       const guchar   *buf,
		       gulong          len)
{
	int n;

	if (cnx->options & LINC_CONNECTION_NONBLOCKING) {
		while (cnx->status == LINC_CONNECTING)
			linc_main_iteration (TRUE);
	}

	g_return_val_if_fail (cnx->status == LINC_CONNECTED, -1);

	d_printf ("Write %ld bytes to fd %d\n", len, cnx->priv->fd);

	while (len > 0) {
#ifdef LINC_SSL_SUPPORT
		if (cnx->options & LINC_CONNECTION_SSL)
			n = SSL_write (cnx->priv->ssl, buf, len);
		else
#endif
			n = write (cnx->priv->fd, buf, len);

		if (n < 0) {
#ifdef LINC_SSL_SUPPORT
			if (cnx->options & LINC_CONNECTION_SSL) {
				gulong rv;

				rv = SSL_get_error (cnx->priv->ssl, n);

				if ((rv == SSL_ERROR_WANT_READ || 
				     rv == SSL_ERROR_WANT_WRITE) &&
				    (cnx->options & LINC_CONNECTION_NONBLOCKING))
					linc_main_iteration (FALSE);
				else
					return -1;
			} else
#endif
			{
				if (errno == EINTR)
					continue;

				else if (errno == EAGAIN &&
					 (cnx->options & LINC_CONNECTION_NONBLOCKING))
					linc_main_iteration (FALSE);

				else if (errno == EBADF) {
					g_warning ("Serious fd usage error %d", cnx->priv->fd);
					return -1;

				} else
					return -1;
			}
		} else if (n == 0)
			return -1;

		else {
			buf += n;
			len -= n;
		}
	}

	return 0;
}

/**
 * linc_connection_writev:
 * @cnx: the connection to write to
 * @vecs: a structure of iovecs to write
 * @nvecs: the number of populated iovecs
 * @total_size: the total size of the data
 * 
 * This routine writes data to the abstract connection.
 * FIXME: it allows re-enterancy via linc_connection_iterate
 *        in certain cases.
 * FIXME: on this basis, the connection can die underneath
 *        our feet.
 * 
 * Return value: 0 on success, non 0 on error.
 **/
int
linc_connection_writev (LINCConnection *cnx,
			struct iovec   *vecs,
			int             nvecs,
			gulong          total_size)
{
	if (cnx->options & LINC_CONNECTION_NONBLOCKING) {
		while (cnx->status == LINC_CONNECTING)
			linc_main_iteration (TRUE);
	}

	d_printf ("Writev %ld bytes to fd %d\n", total_size, cnx->priv->fd);

	g_return_val_if_fail (cnx->status == LINC_CONNECTED, -1);

#ifdef LINC_SSL_SUPPORT
	if (cnx->options & LINC_CONNECTION_SSL) {
		int i;

		for (i = 0; i < nvecs; i++) {
			if (linc_connection_write (
				cnx, vecs[i].iov_base, vecs[i].iov_len))
				return -1;
		}
		return 0;
	} else
#endif
	{
		int fd, vecs_left;
		struct iovec *vptr;
		gulong size_left;
		
		vptr = vecs;
		vecs_left = nvecs;
		size_left = total_size;

		fd = cnx->priv->fd;

		while (size_left > 0) {
			int n;

			n = writev (fd, vptr, MIN (vecs_left, WRITEV_IOVEC_LIMIT));

			if (n < 0) {
				if (errno == EINTR)
					continue;

				else if (errno == EAGAIN &&
					 (cnx->options & LINC_CONNECTION_NONBLOCKING))
					linc_main_iteration (FALSE);

				else if (errno == EBADF) {
					g_warning ("Serious fd usage error %d", cnx->priv->fd);
					return -1;
				} else
					return -1; /* Unhandlable error */

			} else if (n == 0)
				return -1;

			else {
				size_left -= n;
				if (size_left) {
					while (n > vptr->iov_len) {
						n -= vptr->iov_len;
						vecs_left--;
						vptr++;
					}
					vptr->iov_len -= n;
				}
			}
		}
	}

	return 0;
}

static void
linc_connection_init (LINCConnection *cnx)
{
	cnx->priv = g_new0 (LINCConnectionPrivate, 1);
	cnx->priv->fd = -1;
}

static void
linc_connection_class_init (LINCConnectionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	object_class->dispose  = linc_connection_dispose;
	object_class->finalize = linc_connection_finalize;

	klass->state_changed  = linc_connection_class_state_changed;
	klass->broken         = NULL;

	linc_connection_signals [BROKEN] =
		g_signal_new ("broken",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (LINCConnectionClass, broken),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	parent_class = g_type_class_peek_parent (klass);
}

GType
linc_connection_get_type (void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof (LINCConnectionClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) linc_connection_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (LINCConnection),
			0,              /* n_preallocs */
			(GInstanceInitFunc) linc_connection_init,
		};
      
		object_type = g_type_register_static (G_TYPE_OBJECT,
						      "LINCConnection",
						      &object_info,
						      0);
	}  

	return object_type;
}
