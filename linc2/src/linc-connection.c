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
#include <string.h>
#include <stdio.h>

#ifdef LINC_SSL_SUPPORT
#include <openssl/ssl.h>
#endif

#include "linc-debug.h"
#include "linc-private.h"
#include <linc/linc-config.h>
#include <linc/linc-connection.h>

static GObjectClass *parent_class = NULL;

enum {
	BROKEN,
	BLOCKING,
	LAST_SIGNAL
};
static guint   signals [LAST_SIGNAL];
static GMutex *cnx_lock = NULL;
static GCond  *cnx_condition = NULL;

#define CNX_LOCK(cnx)      LINC_MUTEX_LOCK (cnx_lock)
#define CNX_UNLOCK(cnx)    LINC_MUTEX_UNLOCK (cnx_lock)
#define CNX_IS_LOCKED(cnx) linc_mutex_is_locked (cnx_lock)

static gboolean linc_connection_io_handler (GIOChannel  *gioc,
					    GIOCondition condition,
					    gpointer     data);

static void
linc_close_fd (LINCConnection *cnx)
{
	if (cnx->priv->fd >= 0) {
		d_printf ("Close %d\n", cnx->priv->fd);

		LINC_CLOSE (cnx->priv->fd);
	}
	cnx->priv->fd = -1;
}

typedef struct {
	guchar       *data;

	struct iovec *vecs;
	int           nvecs;
	struct iovec  single_vec;
} QueuedWrite;

static void
queue_signal (LINCConnection *cnx,
	      glong           delta)
{
	gulong old_size;
	gulong new_size;

	d_printf ("Queue signal %ld bytes, delta %ld, max %ld\n",
		  cnx->priv->write_queue_bytes, delta,
		  cnx->priv->max_buffer_bytes);

	old_size = cnx->priv->write_queue_bytes;
	cnx->priv->write_queue_bytes += delta;
	new_size = cnx->priv->write_queue_bytes;

	linc_object_ref (cnx);

	if (cnx->options & LINC_CONNECTION_BLOCK_SIGNAL) {
		if (new_size == 0 ||
		    (old_size < (cnx->priv->max_buffer_bytes >> 1) &&
		     new_size >= (cnx->priv->max_buffer_bytes >> 1)) ||
		    new_size >= cnx->priv->max_buffer_bytes)
			g_signal_emit (cnx, signals [BLOCKING], 0, new_size);
	}

	if (cnx->priv->max_buffer_bytes &&
	    cnx->priv->write_queue_bytes >= cnx->priv->max_buffer_bytes)
		linc_connection_state_changed (cnx, LINC_DISCONNECTED);

	linc_object_unref (cnx);
}

static gulong
calc_size (struct iovec *src_vecs,
	   int           nvecs)
{
	int i;
	gulong total_size = 0;

	for (i = 0; i < nvecs; i++)
		total_size += src_vecs [i].iov_len;

	return total_size;
}

static void
queue_flattened (LINCConnection *cnx,
		 struct iovec   *src_vecs,
		 int             nvecs,
		 gboolean        update_poll)
{
	int     i;
	guchar *p;
	gulong  total_size;
	QueuedWrite *qw = g_new (QueuedWrite, 1);

	total_size = calc_size (src_vecs, nvecs);

	p = g_malloc (total_size);

	qw->data  = p;
	qw->vecs  = &qw->single_vec;
	qw->nvecs = 1;

	qw->vecs->iov_base = p;
	qw->vecs->iov_len = total_size;

	for (i = 0; i < nvecs; i++) {
		memcpy (p, src_vecs [i].iov_base, src_vecs [i].iov_len);
		p += src_vecs [i].iov_len;
	}
	g_assert (p == (qw->data + total_size));

	d_printf ("Queueing write of %ld bytes on fd %d\n",
		  total_size, cnx->priv->fd);

	cnx->priv->write_queue = g_list_append (cnx->priv->write_queue, qw);
	queue_signal (cnx, total_size);


	if (update_poll) {
		LINCCommandSetCondition *cmd;

		g_assert (linc_get_threaded ());

		cmd = g_new (LINCCommandSetCondition, 1);
		cmd->cmd.type = LINC_COMMAND_SET_CONDITION;
		cmd->cnx = linc_object_ref (cnx);
		cmd->condition = (LINC_ERR_CONDS | LINC_IN_CONDS | G_IO_OUT);
		linc_exec_command (&cmd->cmd);
	}
}

static void
queued_write_free (QueuedWrite *qw)
{
	g_free (qw->data);
	g_free (qw);
}

static void
queue_free (LINCConnection *cnx)
{
	GList *l;

	for (l = cnx->priv->write_queue; l; l = l->next)
		queued_write_free (l->data);

	g_list_free (cnx->priv->write_queue);
	cnx->priv->write_queue = NULL;
}

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

	cnx->priv->tag = linc_io_add_watch_fd (
		cnx->priv->fd, condition,
		linc_connection_io_handler, cnx);

	d_printf ("Added watch on %d (0x%x)\n",
		 cnx->priv->fd, condition);
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
	gboolean changed;

	d_printf ("State changing from '%s' to '%s' on fd %d\n",
		 STATE_NAME (cnx->status), STATE_NAME (status),
		 cnx->priv->fd);

	changed = cnx->status != status;

	cnx->status = status;

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

		if (cnx->priv->tag) /* re-connecting */
			linc_watch_set_condition (
				cnx->priv->tag,
				G_IO_OUT | LINC_ERR_CONDS);
		else
			linc_source_add (cnx, G_IO_OUT | LINC_ERR_CONDS);
		break;

	case LINC_DISCONNECTED:
		linc_source_remove (cnx);
		linc_close_fd (cnx);
		/* don't free pending queue - we could get re-connected */
		if (changed)
			g_signal_emit (G_OBJECT (cnx),
				       signals [BROKEN], 0);
		break;
	}
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
 * Must be called from the I/O thread.
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
	CNX_LOCK (cnx);

	cnx->was_initiated = was_initiated;
	cnx->is_auth       = (proto->flags & LINC_PROTOCOL_SECURE);
	cnx->proto         = proto;
	cnx->options       = options;
	cnx->priv->fd      = fd;

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

	CNX_UNLOCK (cnx);

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
	LincSockLen		saddr_len;

	proto = linc_protocol_find (proto_name);

	if (!proto)
		return FALSE;


	saddr = linc_protocol_get_sockaddr (
		proto, host, service, &saddr_len);

	if (!saddr && (strcmp (proto_name, "IPv6") ==0)) {/* Falling back to IPv4 */
		proto = linc_protocol_find ("IPv4");
		
		saddr = linc_protocol_get_sockaddr (
			proto, host, service, &saddr_len);
	}

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
	if (!retval && fd >= 0) {
		d_printf ("initiation failed\n");
		LINC_CLOSE (fd);
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
 *
 * NB. this method must be called only in the I/O thread, or
 * at construction, and with the cnx lock held ...
 */
void
linc_connection_state_changed (LINCConnection      *cnx,
			       LINCConnectionStatus status)
{
	LINCConnectionClass *klass;

	g_assert (CNX_IS_LOCKED (cnx));

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
 * Can only be used from the main thread / with the cnx locked.
 *
 * Return value: number of bytes written on success; negative on error.
 **/
glong
linc_connection_read (LINCConnection *cnx,
		      guchar         *buf,
		      int             len,
		      gboolean        block_for_full_read)
{
	int bytes_read = 0;

	g_assert (CNX_IS_LOCKED (cnx));

	d_printf ("Read up to %d bytes from fd %d\n", len, cnx->priv->fd);

	if (!len)
		return 0;

	if (cnx->status != LINC_CONNECTED)
		goto fatal_error;

	do {
		int n;

#ifdef LINC_SSL_SUPPORT
		if (cnx->options & LINC_CONNECTION_SSL)
			n = SSL_read (cnx->priv->ssl, buf, len);
		else
#endif
			n = read (cnx->priv->fd, buf, len);

		g_assert (n <= len);

		if (n < 0) {
#ifdef LINC_SSL_SUPPORT
			if (cnx->options & LINC_CONNECTION_SSL) {
				gulong rv;

				rv = SSL_get_error (cnx->priv->ssl, n);

				if ((rv == SSL_ERROR_WANT_READ ||
				     rv == SSL_ERROR_WANT_WRITE) &&
				    (cnx->options & LINC_CONNECTION_NONBLOCKING))
					goto out;
				else
					goto fatal_error;
			} else
#endif
			{
				if (errno == EINTR)
					continue;

				else if (errno == EAGAIN &&
					 (cnx->options & LINC_CONNECTION_NONBLOCKING))
					goto out;

				else if (errno == EBADF) {
					g_warning ("Serious fd usage error %d", cnx->priv->fd);
					goto fatal_error;

				} else
					goto fatal_error;
			}

		} else if (n == 0) {
			d_printf ("we got EOF on fd %d\n", cnx->priv->fd);
			bytes_read = LINC_IO_FATAL_ERROR;
			goto out;
		} else {
			buf += n;
			len -= n;
			bytes_read += n;
		}
	} while (len > 0 && block_for_full_read);

	d_printf ("we read %d bytes\n", bytes_read);

 out:
	return bytes_read;

 fatal_error:
	return LINC_IO_FATAL_ERROR;
}

/* Determine the maximum size of the iovec vector */

#if defined (MAXIOV) /* HPUX */
# define LINC_IOV_MAX (MAXIOV)
#elif defined (IOV_MAX) /* AIX */
# define LINC_IOV_MAX (IOV_MAX)
#elif defined (_SC_IOV_MAX) /* SGI */
# define LINC_IOV_MAX_INIT (sysconf (_SC_IOV_MAX))
#elif defined (__APPLE__)
/* Even though the write(2) man page mentions it, UIO_MAXIOV is only
 * available if KERNEL is defined on MacOS X 10.1
 */
#  define LINC_IOV_MAX 1024
#elif defined (UIO_MAXIOV) /* Glibc */
# define LINC_IOV_MAX (UIO_MAXIOV)
#else /* Safe Guess */
# define LINC_IOV_MAX 16
#endif

/* If the value requires initialization, define the function here */
#if defined (LINC_IOV_MAX_INIT)
# define LINC_IOV_MAX linc_iov_max
  static guint linc_iov_max = 0;
  static inline void
  linc_iov_max_init () 
  {
    if (linc_iov_max == 0)
      {
        gint max;
        G_LOCK_DEFINE_STATIC (linc_iov_max);
        G_LOCK (linc_iov_max);
        if (linc_iov_max == 0)
          {
            max = LINC_IOV_MAX_INIT;
            if (max <= 0)
              max = 16;
            linc_iov_max = max;
          }
        G_UNLOCK (linc_iov_max);
      }
  }
#else
# define linc_iov_max_init()
#endif

static glong
write_data (LINCConnection *cnx, QueuedWrite *qw)
{
	glong bytes_written = 0;

	g_return_val_if_fail (cnx->status == LINC_CONNECTED,
			      LINC_IO_FATAL_ERROR);

	linc_iov_max_init ();

	while ((qw->nvecs > 0) && (qw->vecs->iov_len > 0)) {
		int n;

		d_printf ("write_data %ld bytes to fd %d - ",
			  calc_size (qw->vecs, qw->nvecs), cnx->priv->fd);

#ifdef LINC_SSL_SUPPORT
		if (cnx->options & LINC_CONNECTION_SSL)
			n = SSL_write (cnx->priv->ssl, qw->vecs->iov_base,
				       qw->vecs->iov_len);
		else
#endif
			n = writev (cnx->priv->fd, qw->vecs,
				    MIN (qw->nvecs, LINC_IOV_MAX));

		d_printf ("wrote %d bytes\n", n);

		if (n < 0) {
#ifdef LINC_SSL_SUPPORT
			if (cnx->options & LINC_CONNECTION_SSL) {
				gulong rv;
					
				rv = SSL_get_error (cnx->priv->ssl, n);
					
				if ((rv == SSL_ERROR_WANT_READ || 
				     rv == SSL_ERROR_WANT_WRITE) &&
				    cnx->options & LINC_CONNECTION_NONBLOCKING)
					return LINC_IO_QUEUED_DATA;
				else
					return LINC_IO_FATAL_ERROR;
			} else
#endif
			{
				if (errno == EINTR)
					continue;

				else if (errno == EAGAIN &&
					 (cnx->options & LINC_CONNECTION_NONBLOCKING))
					return LINC_IO_QUEUED_DATA;

				else if (errno == EBADF)
					g_warning ("Serious fd usage error %d", cnx->priv->fd);
				
				return LINC_IO_FATAL_ERROR; /* Unhandlable error */
			}

		} else if (n == 0) /* CHECK: is this really an error condition */
			return LINC_IO_FATAL_ERROR;

		else {
			bytes_written += n;

			while (qw->nvecs > 0 && n >= qw->vecs->iov_len) {
				n -= qw->vecs->iov_len;
				qw->nvecs--;
				qw->vecs++;
			}

			if (n) {
				qw->vecs->iov_len  -= n;
				qw->vecs->iov_base = (guchar *)qw->vecs->iov_base + n;
			}
		}
	}

	return bytes_written;
}

static gboolean
linc_connection_should_block (LINCConnection      *cnx,
			      const LINCWriteOpts *opt_write_opts)
{
	if (!opt_write_opts)
		return TRUE;

	if (opt_write_opts->block_on_write)
		return TRUE;

	return FALSE;
}

/* Always called in main thread */
static void
linc_connection_flush_write_queue (LINCConnection *cnx)
{
	gboolean done_writes = TRUE;

	if (cnx->priv->write_queue) {
		glong        status;
		QueuedWrite *qw = cnx->priv->write_queue->data;

		status = write_data (cnx, qw);

		d_printf ("Wrote queue %ld on fd %d\n", status, cnx->priv->fd);

		if (status >= LINC_IO_OK) {
			cnx->priv->write_queue = g_list_delete_link
				(cnx->priv->write_queue, cnx->priv->write_queue);
			queued_write_free (qw);

			queue_signal (cnx, -status);
			
			done_writes = (cnx->priv->write_queue == NULL);

		} else {
			if (status == LINC_IO_FATAL_ERROR) {
				d_printf ("Fatal error on queued write");
				linc_connection_state_changed (cnx, LINC_DISCONNECTED);
				
			} else {
				d_printf ("Write blocked\n");
				done_writes = FALSE;
			}
		}
	}

	d_printf ("Blocked write queue %s\n", done_writes ?
		  "flushed & empty" : "still active");

	if (done_writes) /* drop G_IO_OUT */
		linc_watch_set_condition
			(cnx->priv->tag,
			 LINC_ERR_CONDS | LINC_IN_CONDS);
	else
		linc_watch_set_condition
			(cnx->priv->tag,
			 LINC_ERR_CONDS | LINC_IN_CONDS | G_IO_OUT);
}

void
linc_connection_exec_set_condition (LINCCommandSetCondition *cmd)
{
	d_printf ("Exec defered set condition on %p -> 0x%x",
		  cmd->cnx, cmd->condition);

	CNX_LOCK (cmd->cnx);
	linc_watch_set_condition
		(cmd->cnx->priv->tag,
		 LINC_ERR_CONDS | LINC_IN_CONDS | G_IO_OUT);
	CNX_UNLOCK (cmd->cnx);
	linc_object_unref (cmd->cnx);
	g_free (cmd);
}

/**
 * linc_connection_writev:
 * @cnx: the connection to write to
 * @vecs: a structure of iovecs to write - this is altered.
 * @nvecs: the number of populated iovecs
 * @opt_write_opts: optional write options, or NULL
 * 
 * This routine writes data to the abstract connection.
 * FIXME: it allows re-enterancy via linc_connection_iterate
 *        in certain cases.
 * FIXME: on this basis, the connection can die underneath
 *        our feet.
 * 
 * Return value: 0 on success, non 0 on error.
 **/
LINCIOStatus
linc_connection_writev (LINCConnection       *cnx,
			struct iovec         *vecs,
			int                   nvecs,
			const LINCWriteOpts  *opt_write_opts)
{
	QueuedWrite qw;
	int         status;

	CNX_LOCK (cnx);

	if (linc_get_threaded ()) {
		d_printf ("Transfer output to main thread\n");
		if (cnx->status == LINC_CONNECTING) {
			queue_flattened (cnx, vecs, nvecs, TRUE);
			CNX_UNLOCK (cnx);
			return LINC_IO_QUEUED_DATA;
		}
	} else if (cnx->options & LINC_CONNECTION_NONBLOCKING)
		linc_connection_wait_connected (cnx);

	if (cnx->status == LINC_DISCONNECTED) {
		CNX_UNLOCK (cnx);
		return LINC_IO_FATAL_ERROR;
	}

	if (cnx->priv->write_queue) {
		/* FIXME: we should really retry the write here, but we'll
		 * get a POLLOUT for this lot at some stage anyway */
		queue_flattened (cnx, vecs, nvecs, FALSE);
		CNX_UNLOCK (cnx);
		return LINC_IO_QUEUED_DATA;
	}

	qw.vecs  = vecs;
	qw.nvecs = nvecs;

 continue_write:
	status = write_data (cnx, &qw);

	if (status == LINC_IO_QUEUED_DATA) {
		if (linc_get_threaded ()) {
			queue_flattened (cnx, qw.vecs, qw.nvecs, TRUE);
			CNX_UNLOCK (cnx);
			return LINC_IO_QUEUED_DATA;
		}

		/* Queue data & listen for buffer space */
		linc_watch_set_condition
			(cnx->priv->tag,
			 LINC_ERR_CONDS | LINC_IN_CONDS | G_IO_OUT);

		if (!linc_connection_should_block (cnx, opt_write_opts)) {
			queue_flattened (cnx, qw.vecs, qw.nvecs, FALSE);
			CNX_UNLOCK (cnx);
			return LINC_IO_QUEUED_DATA;

		} else {
			linc_main_iteration (TRUE);
			goto continue_write;
		}

	} else if (status >= LINC_IO_OK)
		status = LINC_IO_OK;

	CNX_UNLOCK (cnx);

	return status;
}

/**
 * linc_connection_write:
 * @cnx: the connection to write to
 * @buf: a pointer to the start of an array of bytes
 * @len: the length of the array in bytes
 * @opt_write_opts: optional write options, or NULL
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
LINCIOStatus
linc_connection_write (LINCConnection       *cnx,
		       const guchar         *buf,
		       gulong                len,
		       const LINCWriteOpts  *opt_write_opts)
{
	struct iovec vec;

	vec.iov_base = (guchar *) buf;
	vec.iov_len  = len;

	return linc_connection_writev (cnx, &vec, 1, opt_write_opts);
}

static void
linc_connection_dispose (GObject *obj)
{
	LINCConnection *cnx = (LINCConnection *)obj;

	d_printf ("dispose connection %p\n", obj);

	linc_source_remove (cnx);
	queue_free (cnx);

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

static void
linc_connection_init (LINCConnection *cnx)
{
	d_printf ("create new connection %p\n", cnx);

	cnx->priv = g_new0 (LINCConnectionPrivate, 1);
	cnx->priv->fd = -1;
}

static void
linc_connection_class_init (LINCConnectionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	cnx_lock = linc_mutex_new();
	if (cnx_lock)
		cnx_condition = g_cond_new();

	object_class->dispose  = linc_connection_dispose;
	object_class->finalize = linc_connection_finalize;

	klass->state_changed  = linc_connection_class_state_changed;
	klass->broken         = NULL;

	signals [BROKEN] =
		g_signal_new ("broken",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (LINCConnectionClass, broken),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	signals [BLOCKING] =
		g_signal_new ("blocking",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (LINCConnectionClass, blocking),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__ULONG,
			      G_TYPE_NONE, 1, G_TYPE_ULONG);

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


LINCWriteOpts *
linc_write_options_new (gboolean block_on_write)
{
	LINCWriteOpts *write_opts = g_new0 (LINCWriteOpts, 1);

	write_opts->block_on_write = block_on_write;

	return write_opts;
}

void
linc_write_options_free (LINCWriteOpts *write_opts)
{
	g_free (write_opts);
}

void
linc_connection_set_max_buffer (LINCConnection *cnx,
				gulong          max_buffer_bytes)
{
	g_return_if_fail (cnx != NULL);

	CNX_LOCK (cnx);
	/* FIXME: we might want to check the current buffer size */
	cnx->priv->max_buffer_bytes = max_buffer_bytes;

	CNX_UNLOCK (cnx);
}

static gboolean
linc_connection_io_handler (GIOChannel  *gioc,
			    GIOCondition condition,
			    gpointer     data)
{
	LINCConnection      *cnx = data;
	LINCConnectionClass *klass;

	linc_object_ref (cnx);

	CNX_LOCK (cnx);

	klass = (LINCConnectionClass *) G_TYPE_INSTANCE_GET_CLASS (
		data, LINC_TYPE_CONNECTION, LINCConnection);

	if (cnx->status == LINC_CONNECTED &&
	    condition & LINC_IN_CONDS && klass->handle_input) {
		
		d_printf ("Handle input on fd %d\n", cnx->priv->fd);

		klass->handle_input (cnx);
	}

	if (cnx->status == LINC_CONNECTED && condition & G_IO_OUT) {
		d_printf ("IO Out - buffer space free ...\n");
		linc_connection_flush_write_queue (cnx);
	}

	if (condition & (LINC_ERR_CONDS | G_IO_OUT)) {
		int rv, n;
		LincSockLen n_size = sizeof (n);

		switch (cnx->status) {
		case LINC_CONNECTING:
			n = 0;
			rv = getsockopt (cnx->priv->fd, SOL_SOCKET, SO_ERROR, &n, &n_size);
			if (!rv && !n && condition == G_IO_OUT) {
				d_printf ("State changed to connected on %d\n", cnx->priv->fd);

				linc_watch_set_condition (
					cnx->priv->tag,
					LINC_ERR_CONDS | LINC_IN_CONDS);

				linc_connection_state_changed (cnx, LINC_CONNECTED);

				if (cnx->priv->write_queue) {
					d_printf ("Connected, with queued writes, start flush ...\n");
					linc_connection_flush_write_queue (cnx);
				}
			} else {
				d_printf ("Error connecting %d %d %d on fd %d\n",
					   rv, n, errno, cnx->priv->fd);
				linc_connection_state_changed (cnx, LINC_DISCONNECTED);
			}
			break;
		case LINC_CONNECTED: {
			if (condition & LINC_ERR_CONDS) {
				d_printf ("Disconnect on err: %d\n", cnx->priv->fd);
				linc_connection_state_changed (cnx, LINC_DISCONNECTED);
			}
			break;
		}
		default:
			break;
		}
	}

	CNX_UNLOCK (cnx);
	linc_object_unref (cnx);

	return TRUE;
}

LINCConnectionStatus
linc_connection_get_status (LINCConnection *cnx)
{
	LINCConnectionStatus status;

	CNX_LOCK (cnx);
	status = cnx->status;
	CNX_UNLOCK (cnx);

	d_printf ("Get status on %p = %d", cnx, status);

	return status;
}

void
linc_connection_exec_disconnect (LINCCommandDisconnect *cmd)
{
	d_printf ("Exec defered disconnect on %p", cmd->cnx);

	CNX_LOCK (cmd->cnx);
	linc_connection_state_changed (cmd->cnx, LINC_DISCONNECTED);
	CNX_UNLOCK (cmd->cnx);

	linc_object_unref (cmd->cnx);
	g_free (cmd);
}

void
linc_connection_disconnect (LINCConnection *cnx)
{
	LINCCommandDisconnect *cmd;

	cmd = g_new (LINCCommandDisconnect, 1);
	cmd->cmd.type = LINC_COMMAND_DISCONNECT;
	cmd->cnx = linc_object_ref (cnx);

	linc_exec_command ((LINCCommand *) cmd);
}

LINCConnectionStatus
linc_connection_wait_connected (LINCConnection *cnx)
{
	g_return_val_if_fail (!linc_get_threaded (),
			      LINC_CONNECTED);
	
	while (cnx && cnx->status == LINC_CONNECTING) 
		linc_main_iteration (TRUE);

	return cnx ? cnx->status : LINC_DISCONNECTED;
}
