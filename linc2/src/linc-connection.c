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

#include "linc-private.h"
#include <linc/linc-config.h>
#include <linc/linc-connection.h>

static GObjectClass *parent_class = NULL;

enum {
	BROKEN,
	LAST_SIGNAL
};
static guint linc_connection_signals [LAST_SIGNAL];

static void
linc_source_remove (LINCConnection *cnx, gboolean unref)
{
	if (cnx->tag) {
		LincWatch *thewatch = cnx->tag;
		cnx->tag = NULL;
		linc_io_remove_watch (thewatch);
		if (unref) 
			g_object_unref (G_OBJECT (cnx));
	}
}

static void
linc_close_fd (LINCConnection *cnx)
{
	if (cnx->fd >= 0)
		close (cnx->fd);
	cnx->fd = -1;
}

static void
linc_connection_dispose (GObject *obj)
{
	LINCConnection *cnx = (LINCConnection *)obj;

	linc_source_remove (cnx, FALSE);

	g_free (cnx->remote_host_info);
	cnx->remote_host_info = NULL;

	g_free (cnx->remote_serv_info);
	cnx->remote_serv_info = NULL;

	if (cnx->gioc)
		g_io_channel_unref (cnx->gioc);
	cnx->gioc = NULL;

	linc_close_fd (cnx);

	if (parent_class->dispose)
		parent_class->dispose (obj);
}

#define ERR_CONDS (G_IO_ERR|G_IO_HUP|G_IO_NVAL)
#define IN_CONDS  (G_IO_PRI|G_IO_IN)

static gboolean
linc_connection_connected (GIOChannel  *gioc,
			   GIOCondition condition,
			   gpointer     data)
{
	LINCConnection      *cnx = data;
	LINCConnectionClass *klass;
	int rv, n;
	socklen_t n_size = sizeof(n);
	gboolean disconnected = FALSE;

	g_object_ref (G_OBJECT (cnx));

	klass = LINC_CONNECTION_CLASS (G_OBJECT_GET_CLASS (data));

	if (cnx->status == LINC_CONNECTED &&
	    condition & IN_CONDS &&
	    klass->handle_input)

		klass->handle_input (cnx);

	else if (condition & (ERR_CONDS | G_IO_OUT)) {

		switch (cnx->status) {
		case LINC_CONNECTING:
			n = 0;
			rv = getsockopt (cnx->fd, SOL_SOCKET, SO_ERROR, &n, &n_size);
			if (!rv && !n && condition == G_IO_OUT) {
				linc_connection_state_changed (cnx, LINC_CONNECTED);
				g_assert (cnx->tag == NULL);
				cnx->tag = linc_io_add_watch (
					cnx->gioc, ERR_CONDS,
					linc_connection_connected, cnx);

			} else {
				linc_connection_state_changed (cnx, LINC_DISCONNECTED);
				disconnected = TRUE;
			}
			break;
		case LINC_CONNECTED: {
			linc_connection_state_changed(cnx, LINC_DISCONNECTED);
			disconnected = TRUE;
			break;
		}
		default:
			break;
		}
	}

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
	switch (status) {
	case LINC_CONNECTED:
#ifdef LINC_SSL_SUPPORT
		if (cnx->options & LINC_CONNECTION_SSL) {
			if (cnx->was_initiated)
				SSL_connect (cnx->ssl);
			else
				SSL_accept (cnx->ssl);
		}
#endif
		if (!cnx->tag)
			cnx->tag = linc_io_add_watch (
				cnx->gioc, ERR_CONDS | IN_CONDS,
				linc_connection_connected, cnx);
		break;

	case LINC_CONNECTING:
		linc_source_remove (cnx, FALSE);
		cnx->tag = linc_io_add_watch (
			cnx->gioc, G_IO_OUT|ERR_CONDS,
			linc_connection_connected, cnx);
		break;

	case LINC_DISCONNECTED:
		linc_source_remove (cnx, TRUE);
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
 * @host: protocol dependant host information.
 * @service: protocol dependant service information(e.g. port number).
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
			 char                   *host,
			 char                   *service,
			 gboolean                was_initiated,
			 LINCConnectionStatus    status,
			 LINCConnectionOptions   options)
{
	cnx->fd               = fd;
	cnx->gioc             = g_io_channel_unix_new (fd);
	cnx->was_initiated    = was_initiated;
	cnx->is_auth          = (proto->flags & LINC_PROTOCOL_SECURE) ? TRUE : FALSE;
	cnx->remote_host_info = host;
	cnx->remote_serv_info = service;
	cnx->options          = options;

	if (proto->setup)
		proto->setup (fd, options);

#ifdef LINC_SSL_SUPPORT
	if (options & LINC_CONNECTION_SSL) {
		cnx->ssl = SSL_new (linc_ssl_ctx);
		SSL_set_fd (cnx->ssl, fd);
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


	saddr = linc_protocol_get_sockaddr (proto, host, 
					    service, &saddr_len);
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

	retval = linc_connection_from_fd (
				cnx, fd, proto, 
				g_strdup (host),
				g_strdup (service),
				TRUE, rv ? LINC_CONNECTING : LINC_CONNECTED,
				options);

 out:
	if (!cnx && fd >= 0)
		close (fd);

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
		klass->state_changed(cnx, status);
}

/* FIXME: the linc_main_iteration stuff here
   looks like a deadlock / re-enterancy hell */
int
linc_connection_read (LINCConnection *cnx, guchar *buf,
		      int len, gboolean block_for_full_read)
{
	int n;
	int written = 0;

	if (!len)
		return 0;

	if (cnx->options & LINC_CONNECTION_NONBLOCKING) {
		while (cnx->status == LINC_CONNECTING)
			linc_main_iteration (TRUE);
	}

	if (cnx->status != LINC_CONNECTED)
		return -1;

	do {
#ifdef LINC_SSL_SUPPORT
		if (cnx->options & LINC_CONNECTION_SSL)
			n = SSL_read (cnx->ssl, buf, len);
		else
#endif
			n = read (cnx->fd, buf, len);

		if (n < 0) {
#ifdef LINC_SSL_SUPPORT
			if (cnx->options & LINC_CONNECTION_SSL) {
				gulong rv;
				rv = SSL_get_error (cnx->ssl, n);
				if ((rv == SSL_ERROR_WANT_READ
				     || rv == SSL_ERROR_WANT_WRITE)
				    && (cnx->options & LINC_CONNECTION_NONBLOCKING))
					linc_main_iteration (FALSE);
				else
					return -1;
			} else
#endif
			{
				if (errno == EAGAIN
				   && (cnx->options & LINC_CONNECTION_NONBLOCKING))
					linc_main_iteration (FALSE);
				else
					return -1;
			}

		} else if (n == 0)
			return 0;

		else {
			buf += n;
			len -= n;
			written += n;
		}
	} while (len > 0 && block_for_full_read);

	return written;
}

int
linc_connection_write (LINCConnection *cnx, const guchar *buf, gulong len)
{
  int n;

  if(cnx->options & LINC_CONNECTION_NONBLOCKING)
    {
      while(cnx->status == LINC_CONNECTING)
	linc_main_iteration(TRUE);
    }

  g_return_val_if_fail(cnx->status == LINC_CONNECTED, -1);

  while(len > 0)
    {
#ifdef LINC_SSL_SUPPORT
      if(cnx->options & LINC_CONNECTION_SSL)
	n = SSL_write(cnx->ssl, buf, len);
      else
#endif
	n = write(cnx->fd, buf, len);

      if(n < 0)
	{
#ifdef LINC_SSL_SUPPORT
	  if(cnx->options & LINC_CONNECTION_SSL)
	    {
	      gulong rv;
	      rv = SSL_get_error(cnx->ssl, n);
	      if((rv == SSL_ERROR_WANT_READ
		  || rv == SSL_ERROR_WANT_WRITE)
		 && (cnx->options & LINC_CONNECTION_NONBLOCKING))
		linc_main_iteration(FALSE);
	      else
		return -1;
	    }
	  else
#endif
	    {
	      if(errno == EAGAIN
		 && (cnx->options & LINC_CONNECTION_NONBLOCKING))
		linc_main_iteration(FALSE);
	      else
		return -1;
	    }
	}
      else if(n == 0)
	return -1;
      else
	{
	  buf += n;
	  len -= n;
	}
    }

  return 0;
}

int
linc_connection_writev (LINCConnection *cnx, struct iovec *vecs,
			int nvecs, gulong total_size)
{
  register int n, fd, vecs_left;
  register struct iovec *vptr;
  register gulong size_left;

  if(cnx->options & LINC_CONNECTION_NONBLOCKING)
    {
      while(cnx->status == LINC_CONNECTING)
	linc_main_iteration(TRUE);
    }

  g_return_val_if_fail(cnx->status == LINC_CONNECTED, -1);

  fd = cnx->fd;

#ifdef LINC_SSL_SUPPORT
  if(cnx->options & LINC_CONNECTION_SSL)
    {
      for(n = 0; n < nvecs; n++)
	{
	  if(linc_connection_write(cnx, vecs[n].iov_base, vecs[n].iov_len))
	    return -1;
	}

      return 0;
    }
#endif

  vptr = vecs;
  vecs_left = nvecs;
  size_left = total_size;
  while(size_left > 0)
    {
      n = writev(fd, vptr, MIN(vecs_left, WRITEV_IOVEC_LIMIT));
      if(n < 0)
	{
	  if(errno == EAGAIN
	     && (cnx->options & LINC_CONNECTION_NONBLOCKING))
	    linc_main_iteration(FALSE); /* Try to give other things a
					   chance to run */
	  else
	    return -1; /* Unhandlable error */
	}
      else if(n == 0)
	return -1;
      else
	{
	  size_left -= n;
	  if(size_left)
	    {
	      while(n > vptr->iov_len)
		vptr++;
	      vptr->iov_len -= n;
	    }
	}
    }

  return 0;
}

static void
linc_connection_init (LINCConnection *cnx)
{
	cnx->fd = -1;
}

static void
linc_connection_class_init (LINCConnectionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	object_class->dispose = linc_connection_dispose;

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
