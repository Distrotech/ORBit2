#include "config.h"
#if LINC_SSL_SUPPORT
/* So we can check if we need to retry */
#include <openssl/bio.h>
#endif
#include "linc-private.h"
#include <linc/linc-connection.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

static void linc_connection_init       (LINCConnection      *cnx);
static void linc_connection_real_state_changed (LINCConnection  *cnx, LINCConnectionStatus status);
static void linc_connection_destroy    (GObject             *obj);
static void linc_connection_class_init (LINCConnectionClass *klass);

GType
linc_connection_get_type(void)
{
  static GType object_type = 0;

  if (!object_type)
    {
      static const GTypeInfo object_info =
      {
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

static void
linc_connection_class_init (LINCConnectionClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;

  object_class->shutdown = linc_connection_destroy;
  klass->state_changed = linc_connection_real_state_changed;
}

static void
linc_connection_init       (LINCConnection      *cnx)
{
}

static void
linc_connection_destroy    (GObject             *obj)
{
  LINCConnection *cnx = (LINCConnection *)obj;

  if(cnx->tag)
    g_source_remove(cnx->tag);
  g_free(cnx->remote_host_info);
  g_free(cnx->remote_serv_info);
  close(cnx->fd);
}

static gboolean
linc_connection_connected(GIOChannel *gioc, GIOCondition condition, gpointer data)
{
  LINCConnection *cnx = data;
  int rv, n;
  socklen_t n_size = sizeof(n);
  gboolean retval = TRUE;

  switch(cnx->status)
    {
    case LINC_CONNECTING:
      n = 0;
      rv = getsockopt(cnx->fd, SOL_SOCKET, SO_ERROR, &n, &n_size);
      if(!rv && !n && condition == G_IO_OUT)
	{
	  linc_connection_state_changed(cnx, LINC_CONNECTED);
	  cnx->tag = g_io_add_watch(cnx->gioc,
				    G_IO_ERR|G_IO_HUP|G_IO_NVAL|G_IO_PRI,
				    linc_connection_connected, cnx);
	  retval = FALSE;
	}
      else
	linc_connection_state_changed(cnx, LINC_DISCONNECTED);
      break;
    case LINC_CONNECTED:
      linc_connection_state_changed(cnx, LINC_DISCONNECTED);
      break;
    default:
      break;
    }

  return retval;
}

static void
linc_connection_real_state_changed (LINCConnection *cnx, LINCConnectionStatus status)
{
  switch(status)
    {
    case LINC_CONNECTED:
#if LINC_SSL_SUPPORT
      if(cnx->options & LINC_CONNECTION_SSL)
	{
	  if(cnx->was_initiated)
	    SSL_connect(cnx->ssl);
	  else
	    SSL_accept(cnx->ssl);
	}
#endif
      if(!cnx->tag)
	cnx->tag = g_io_add_watch(cnx->gioc,
				  G_IO_ERR|G_IO_HUP|G_IO_NVAL|G_IO_PRI,
				  linc_connection_connected, cnx);
      break;
    case LINC_CONNECTING:
      if(cnx->tag) g_source_remove(cnx->tag);
      cnx->tag = g_io_add_watch(cnx->gioc, G_IO_OUT|G_IO_ERR|G_IO_HUP|G_IO_NVAL, linc_connection_connected, cnx);
      break;
    case LINC_DISCONNECTED:
      if(cnx->tag) g_source_remove(cnx->tag);
      cnx->tag = 0;
      break;
    }

  cnx->status = status;
}

gboolean
linc_connection_from_fd(LINCConnection *cnx, int fd,
			const LINCProtocolInfo *proto,
			const char *remote_host_info,
			const char *remote_serv_info,
			gboolean was_initiated,
			LINCConnectionStatus status,
			LINCConnectionOptions options)
{
  cnx->fd = fd;
  cnx->gioc = g_io_channel_unix_new(fd);
  cnx->was_initiated = was_initiated;
  cnx->is_auth = (proto->flags & LINC_PROTOCOL_SECURE)?TRUE:FALSE;
  cnx->remote_host_info = g_strdup(remote_host_info);
  cnx->remote_serv_info = g_strdup(remote_serv_info);
  cnx->options = options;
  if(proto->setup)
    proto->setup(fd, options);

#if LINC_SSL_SUPPORT
  if(options & LINC_CONNECTION_SSL)
    {
      cnx->ssl = SSL_new(linc_ssl_ctx);
      SSL_set_fd(cnx->ssl, fd);
    }
#endif

  linc_connection_state_changed(cnx, status);

  return TRUE;
}

/* This will just create the fd, do the connect and all, and then call
   linc_connection_from_fd */
gboolean
linc_connection_initiate(LINCConnection *cnx, const char *proto_name,
			 const char *remote_host_info,
			 const char *remote_serv_info,
			 LINCConnectionOptions options)
{
  struct addrinfo *ai, hints = {AI_CANONNAME, 0, SOCK_STREAM, 0, 0,
				NULL, NULL, NULL};
  int rv;
  int fd;
  const LINCProtocolInfo *proto;
  gboolean retval = FALSE;

  proto = linc_protocol_find(proto_name);

  if(!proto)
    return FALSE;

  hints.ai_family = proto->family;
  rv = linc_getaddrinfo(remote_host_info, remote_serv_info, &hints, &ai);
  if(rv)
    return FALSE;

  fd = socket(proto->family, SOCK_STREAM, proto->stream_proto_num);
  if(fd < 0)
    goto out;
  if(options & LINC_CONNECTION_NONBLOCKING)
    fcntl(fd, F_SETFL, O_NONBLOCK);
  fcntl(fd, F_SETFD, FD_CLOEXEC);

  rv = connect(fd, ai->ai_addr, ai->ai_addrlen);
  if(rv && errno != EINPROGRESS)
    goto out;

  retval = linc_connection_from_fd(cnx, fd, proto, remote_host_info,
				   remote_serv_info, TRUE, rv?LINC_CONNECTING:LINC_CONNECTED, options);

 out:
  if(!cnx)
    {
      if(fd >= 0)
	close(fd);
    }
  freeaddrinfo(ai);

  return retval;
}

void
linc_connection_state_changed(LINCConnection *cnx, LINCConnectionStatus status)
{
  LINCConnectionClass *klass;

  klass = (LINCConnectionClass *)G_OBJECT_GET_CLASS(cnx);

  if(klass->state_changed)
    klass->state_changed(cnx, status);
}

int
linc_connection_read(LINCConnection *cnx, guchar *buf, int len, gboolean block_for_full_read)
{
  int n;
  int written = 0;

  if(cnx->options & LINC_CONNECTION_NONBLOCKING)
    {
      while(cnx->status == LINC_CONNECTING)
	g_main_iteration(TRUE);
    }

  if(cnx->status != LINC_CONNECTED)
    return -1;

  do {
#if LINC_SSL_SUPPORT
    if(cnx->options & LINC_CONNECTION_SSL)
      n = SSL_read(cnx->ssl, buf, len);
    else
#endif
      n = read(cnx->fd, buf, len);

      if(n < 0)
	{
#if LINC_SSL_SUPPORT
	  if(cnx->options & LINC_CONNECTION_SSL)
	    {
	      gulong rv;
	      rv = SSL_get_error(cnx->ssl, n);
	      if((rv == SSL_ERROR_WANT_READ
		  || rv == SSL_ERROR_WANT_WRITE)
		 && (cnx->options & LINC_CONNECTION_NONBLOCKING))
		g_main_iteration(FALSE);
	      else
		return -1;
	    }
	  else
#endif
	    {
	      if(errno == EAGAIN
		 && (cnx->options & LINC_CONNECTION_NONBLOCKING))
		g_main_iteration(FALSE);
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
	  written += n;
	}
  } while(len > 0 && block_for_full_read);

  return written;
}

int
linc_connection_write(LINCConnection *cnx, const guchar *buf, gulong len)
{
  int n;

  if(cnx->options & LINC_CONNECTION_NONBLOCKING)
    {
      while(cnx->status == LINC_CONNECTING)
	g_main_iteration(TRUE);
    }

  g_return_val_if_fail(cnx->status == LINC_CONNECTED, -1);

  while(len > 0)
    {
#if LINC_SSL_SUPPORT
      if(cnx->options & LINC_CONNECTION_SSL)
	n = SSL_write(cnx->ssl, buf, len);
      else
#endif
	n = write(cnx->fd, buf, len);

      if(n < 0)
	{
#if LINC_SSL_SUPPORT
	  if(cnx->options & LINC_CONNECTION_SSL)
	    {
	      gulong rv;
	      rv = SSL_get_error(cnx->ssl, n);
	      if((rv == SSL_ERROR_WANT_READ
		  || rv == SSL_ERROR_WANT_WRITE)
		 && (cnx->options & LINC_CONNECTION_NONBLOCKING))
		g_main_iteration(FALSE);
	      else
		return -1;
	    }
	  else
#endif
	    {
	      if(errno == EAGAIN
		 && (cnx->options & LINC_CONNECTION_NONBLOCKING))
		g_main_iteration(FALSE);
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
linc_connection_writev(LINCConnection *cnx, struct iovec *vecs, int nvecs, gulong total_size)
{
  register int n, fd, vecs_left;
  register struct iovec *vptr;
  register gulong size_left;

  if(cnx->options & LINC_CONNECTION_NONBLOCKING)
    {
      while(cnx->status == LINC_CONNECTING)
	g_main_iteration(TRUE);
    }

  g_return_val_if_fail(cnx->status == LINC_CONNECTED, -1);

  fd = cnx->fd;

#if LINC_SSL_SUPPORT
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
	    g_main_iteration(FALSE); /* Try to give other things a
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
