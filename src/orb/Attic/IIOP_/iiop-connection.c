#include "IIOP-private.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#ifndef _XOPEN_SOURCE_EXTENDED
#  define _XOPEN_SOURCE_EXTENDED 1
#  define WE_DEFINED_XOPEN_SOURCE_EXTENDED
#endif
#include <arpa/inet.h>
#include <netdb.h>
#ifdef WE_DEFINED_XOPEN_SOURCE_EXTENDED
#  undef _XOPEN_SOURCE_EXTENDED
#endif
#include <ctype.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <syslog.h>

#ifndef HAVE_GETADDRINFO
#  include <ORBitutil/getaddrinfo.h>
#endif

#ifdef AF_INET6
#define SIZEOF_SOCKADDR_IN_4_6 \
   (MAX (sizeof (struct sockaddr_in), sizeof (struct sockaddr_in6)))
#else /* !AF_INET6 */
#define SIZEOF_SOCKADDR_IN_4_6 (sizeof (struct sockaddr_in))
#endif /* AF_INET6 */

#define MAX_SIZEOF_SOCKADDR    \
   (MAX (SIZEOF_SOCKADDR_IN_4_6, sizeof (struct sockaddr_un)))

#ifdef USE_TCP_WRAPPERS
#include <tcpd.h>
#endif /* USE_TCP_WRAPPERS */

#define IIOP_ADDRESS(x) (((IIOPConnection *)(x))->address)

typedef struct _IIOPConnection IIOPConnection;
struct _IIOPConnection
{
  GIOPConnection giop_connection;
  struct sockaddr *address;
};

static GIOPConnection* iiop_connection_new (GIOPConnectionClass cnxclass);
static void iiop_connection_accept (GIOPConnection * connection);
static void iiop_connection_destroy (GIOPConnection * connection);

static void iiop_unlink_unix_sockets (void);

#ifdef USE_TCP_WRAPPERS
static const char *argv0_val = NULL;
#endif /* USE_TCP_WRAPPERS */

static GQuark iiop_type_quark;

/*
 * iiop_init
 *
 *    Inputs: None
 *    Outputs: None
 *
 *    Side effects: Initializes iiop_unix_socket_list
 *    Global data structures used: iiop_unix_socket_list
 *
 *    Description: Initializes iiop_unix_socket_list.
 *                 Registers Unix domain sockets for
 *                 removal at server termination.
 */
void
iiop_init (const char *argv0)
{

#ifdef USE_TCP_WRAPPERS
  argv0_val = g_strdup (g_basename (argv0));	/* For TCP wrappers */
#endif /* USE_TCP_WRAPPERS */

  iiop_type_quark = g_quark_from_static_string ("IIOPConnection");  

  if (giop_unlink_unix_sockets_atexit)
    g_atexit (iiop_unlink_unix_sockets);
}

/*
 * iiop_connection_new
 *
 *    Side effects: allocates and initializes 'connection'
 *
 *    Description: Performs the IIOP-specific initialization of an
 *                 IIOPConnection. giop_connection_init is called.
 *
 */
static GIOPConnection*
iiop_connection_new (GIOPConnectionClass cnxclass)
{
  GIOPConnection *connection = GIOP_CONNECTION (g_new0 (IIOPConnection, 1));

  giop_connection_init (connection, iiop_type_quark, cnxclass);

  connection->destroy_func = iiop_connection_destroy;
  connection->accept_func = iiop_connection_accept;

  IIOP_ADDRESS (connection) = NULL;

  return connection;
}

/*
 * iiop_connection_from_fd
 *
 *    Inputs: 'fd' - a file descriptor that attention should be paid to
 *    Outputs: 'fd_cnx' - the created connection 
 *
 *    Description: This is intended to be used on a file descriptor
 *	           that has been accept()'d. It creates the connection
 *	           and fills in the connection information, then adds
 *	           it to the active list.
 */
static GIOPConnection *
iiop_connection_from_fd (GIOPConnectionClass cnxclass, int fd)
{
  GIOPConnection *cnx;
  socklen_t n = MAX_SIZEOF_SOCKADDR;
  gpointer address;
  gint error;

  g_return_val_if_fail (fd >= 0, NULL);

  address = g_malloc (MAX_SIZEOF_SOCKADDR);
  
  if (cnxclass == GIOP_CONNECTION_SERVER)
    error = getsockname (fd, address, &n);
  else /* cnxclass == GIOP_CONNECTION_CLIENT */
    error = getpeername (fd, address, &n);
  
  g_assert (n <= MAX_SIZEOF_SOCKADDR);
  
  if (error)
    {
      ORBit_critical (ORBIT_LOG_DOMAIN_IIOP,
		      "iiop_connection_from_fd: "
		      "get(peer|sock)name error: %s.",
		      g_strerror (errno));
      g_free (address);
      return NULL;
    }
  
  cnx = iiop_connection_new (cnxclass);
  cnx->channel = g_io_channel_unix_new (fd);
  IIOP_ADDRESS (cnx) = address;
  
  fcntl (fd, F_SETFD, fcntl (fd, F_GETFD, 0) | FD_CLOEXEC);
  fcntl (fd, F_SETFL, fcntl (fd, F_GETFL, 0) | O_NONBLOCK);
  
  giop_connection_add_to_list (cnx);

  return cnx;
}

static gint
iiop_connection_server_fd (const gchar* unixpath, const gchar* ipport)
{
  gint fd = -1, error;
  struct addrinfo hints, *info, *cur_info;

  g_return_val_if_fail (unixpath || ipport, -1);

  memset (&hints, '\0', sizeof (hints));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = unixpath ? AF_UNIX : AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  
  info = NULL;
  error = getaddrinfo (NULL, unixpath ? unixpath : ipport, &hints, &info);
  if (error)
    {
      ORBit_critical (ORBIT_LOG_DOMAIN_IIOP,
		      "iiop_connection_server: getaddrinfo error: %s.",
		      gai_strerror (error));
      return -1;
    }
  
  for (cur_info = info; cur_info; cur_info = cur_info->ai_next)
    {
      if (cur_info->ai_family == AF_INET || 
#ifdef AF_INET6
	  cur_info->ai_family == AF_INET6 ||
#endif /* AF_INET6 */
	  (unixpath && cur_info->ai_family == AF_UNIX))
	
	{
	  errno = 0;
	  
	  fd = socket (cur_info->ai_family, cur_info->ai_socktype, 
		       cur_info->ai_protocol);
	  if (fd < 0)
	    continue;
	  
	  if (bind (fd, cur_info->ai_addr, cur_info->ai_addrlen))
	    {
	      close (fd);
	      continue;
	    }
	  
	  if (listen (fd, 5) == 0)
	    break;
	  
	  close (fd);
	}
    }
  
  freeaddrinfo (info);
  
  if (!cur_info)
    {
      if (unixpath && errno == EADDRINUSE)
	{
	  ORBit_message (ORBIT_LOG_DOMAIN_IIOP,
			 "iiop_connection_server_fd: unixpath %s already in "
			 "use.", unixpath);
	}
      else
	{
	  ORBit_critical (ORBIT_LOG_DOMAIN_IIOP,
			  "iiop_connection_server_fd: socket/bind/listen "
			  "error: %s.", g_strerror (errno));
	}
      return -1;
    }

  return fd;
}

G_LOCK_DEFINE_STATIC (reserved_ipport);
static gint reserved_ipport = -1;

gboolean
iiop_connection_reserve_ipport (const gushort port)
{
  gboolean success;
  char port_string[10];
  g_snprintf(port_string, sizeof (port_string), "%d", port);

  G_LOCK (reserved_ipport);

  if (reserved_ipport > 0)
    close (reserved_ipport);

  reserved_ipport = iiop_connection_server_fd (NULL, port_string);
  success = reserved_ipport >= 0;  

  G_UNLOCK (reserved_ipport);

  return success;
}

GIOPConnection *
iiop_connection_server_ip ()
{
  if (reserved_ipport < 0)
    if (!iiop_connection_reserve_ipport (0))
      return NULL;

  return iiop_connection_from_fd (GIOP_CONNECTION_SERVER, reserved_ipport);
}

GIOPConnection *
iiop_connection_server_unix (const gchar* unixpath)
{
  gint fd;

  fd = iiop_connection_server_fd (unixpath, NULL);
  
  if (fd < 0)
    return NULL;

  return iiop_connection_from_fd (GIOP_CONNECTION_SERVER, fd);
}

static gint
iiop_unlink_unix_sockets_int (gconstpointer connection, gconstpointer data)
{
  if (GIOP_CONNECTION (connection)->class == GIOP_CONNECTION_SERVER &&
      IIOP_ADDRESS (connection)->sa_family == AF_UNIX)
    unlink (iiop_connection_unix_path (connection));

  return -1;
}

static void
iiop_unlink_unix_sockets (void)
{
  giop_connection_find_in_list (iiop_type_quark, NULL,
				iiop_unlink_unix_sockets_int);
}

static gint
iiop_connection_compare (gconstpointer connection, gconstpointer data)
{
  const struct sockaddr* first = IIOP_ADDRESS (connection);
  const struct sockaddr* second = data;

  if (first->sa_family != second->sa_family)
    return -1;

  switch (first->sa_family)
    {
    case AF_UNIX:
      return strcmp (((struct sockaddr_un*)first)->sun_path,
		     ((struct sockaddr_un*)second)->sun_path);
    case AF_INET:
      if (((struct sockaddr_in*)first)->sin_port !=
	  ((struct sockaddr_in*)second)->sin_port)
	return -1;
      return memcmp (&((struct sockaddr_in*)first)->sin_addr,
		     &((struct sockaddr_in*)second)->sin_addr, 
		     sizeof (struct in_addr));
#ifdef AF_INET6
    case AF_INET6:
      if (((struct sockaddr_in6*)first)->sin6_port !=
	  ((struct sockaddr_in6*)second)->sin6_port)
	return -1;
      return memcmp (&((struct sockaddr_in6*)first)->sin6_addr,
		     &((struct sockaddr_in6*)second)->sin6_addr, 
		     sizeof (struct in6_addr));      
#endif /* AF_INET6 */
    default:
      ORBit_critical (ORBIT_LOG_DOMAIN_IIOP, "iiop_connection_compare: "
		      "Unknown sockaddr family.");
      return -1;
      
    }
}

/*
 * iiop_connection_get
 *
 * Inputs: 'host' - the hostname (or dotted quad) of the remote host that
 *                  will be connected
 *         'port' - the port number on the above host to connect to.
 *         'existing_only' - don't create a new connection if
 *                           an existing one with the specified host:port
 *                           doesn't exist.
 *
 * Outputs: 'cnx' - the connection to the specified host:port, or
 *                  NULL upon error.
 *
 * Description: Returns an IIOPConnection that is connected to the
 *              specified host:port. If a connection already exists to the
 *	 	host:port, just returns it. Otherwise, calls
 *	 	'iiop_connection_create' to create a new connection
 *	 	to host:port.
 */
GIOPConnection *
iiop_connection_get (const char *host, const char* port, 
		     gboolean existing_only)
{
  GIOPConnection *cnx;
  gint fd = -1, error;
  struct addrinfo hints, *info, *cur_info;

  g_return_val_if_fail (host != NULL || port != NULL, NULL);

  memset (&hints, '\0', sizeof (hints));
  hints.ai_family = host ? AF_UNSPEC : AF_UNIX;
  hints.ai_socktype = SOCK_STREAM;
  
  error = getaddrinfo (host, port, &hints, &info);
  if (error)
    {
      ORBit_critical (ORBIT_LOG_DOMAIN_IIOP,
		      "iiop_connection_get: getaddrinfo error: %s.",
		      gai_strerror (error));
      return NULL;
    }
 
  for (cur_info = info; cur_info; cur_info = cur_info->ai_next)
    {
      cnx = giop_connection_find_in_list (iiop_type_quark, 
					  cur_info->ai_addr,
					  iiop_connection_compare);
      if (cnx)
	{
	  freeaddrinfo (info);
	  return cnx;
	}
    }
  
  if (existing_only)
    {
      freeaddrinfo (info); 
      return NULL;
    }

  for (cur_info = info; cur_info; cur_info = cur_info->ai_next)
    {
      fd = socket (cur_info->ai_family, cur_info->ai_socktype, 
		   cur_info->ai_protocol);
      if (fd < 0)
	  continue;
      
      if (connect (fd, cur_info->ai_addr, cur_info->ai_addrlen) == 0)
	break;
     
      close (fd);    
    }

  freeaddrinfo (info);

  if (!cur_info)
    {
      ORBit_critical (ORBIT_LOG_DOMAIN_IIOP,
		      "iiop_connection_get(%s, %s): socket/connect error: %s.",
		      host ? host : "(null)", port ? port : "(null)", 
		      g_strerror (errno));
      return NULL;
    }

  cnx = iiop_connection_from_fd (GIOP_CONNECTION_CLIENT, fd);

  cnx->was_initiated = TRUE;
  cnx->is_auth = TRUE;

  return cnx;
}

/*
 * iiop_connection_accept
 *    Inputs: 'connection' - a server IIOPConnection.
 *
 *    Description: Performs accept(), TCP wrapper, access checking and related
 *                 duties on a connection
 */
#ifdef USE_TCP_WRAPPERS
int allow_severity = LOG_INFO, deny_severity = LOG_NOTICE;

G_LOCK_DEFINE_STATIC (tcp_wrappers_usage);
#endif /* USE_TCP_WRAPPERS */

static void
iiop_connection_accept (GIOPConnection * connection)
{
  int fd;
  GIOPConnection *cnx;

  fd = accept (giop_connection_fd (connection), NULL, NULL);

#ifdef USE_TCP_WRAPPERS
  /* tcp wrappers access checking */
  switch (IIOP_ADDRESS (connection)->sa_family)
    {
    case AF_INET:
      {
	struct request_info request;

	G_LOCK (tcp_wrappers_usage);

	request_init (&request, RQ_DAEMON, argv0_val, RQ_FILE, fd, 0);

	fromhost (&request);
	if (!hosts_access (&request))
	  {
	    syslog (deny_severity, "[orbit] refused connect from %s.",
		    eval_client (&request));
	    close (fd);
	    fd = -1;
	  }
	else
	  syslog (allow_severity, "[orbit] connect from %s.",
		  eval_client (&request));

	G_UNLOCK (tcp_wrappers_usage);
      }
      break;
    default:
      /* No access controls for these transports */
      break;
    }
#endif /* USE_TCP_WRAPPERS */

  if (fd >= 0)
    {
      cnx = iiop_connection_from_fd (GIOP_CONNECTION_CLIENT, fd);
      cnx->orb_data = connection->orb_data;

      switch (IIOP_ADDRESS (connection)->sa_family)
	{
	case AF_UNIX:
	  cnx->is_auth = TRUE;
	  break;
	default:
	  break;
	}
    }
}

/*
 * iiop_connection_destroy
 *
 *    Inputs: 'iiop_connection' - an IIOPConnection to be finalized
 *
 *    Side effects: invalidates 'iiop_connection' for use as an IIOPConnection
 *
 *    Description: Performs the IIOP-specific parts of connection shutdown,
 *    including sending a CLOSECONNECTION message to the remote side.
 */
static void
iiop_connection_destroy (GIOPConnection *connection)
{
  if (IIOP_ADDRESS (connection)->sa_family == AF_UNIX)
    {
      /* why do we check if fd is > 0 here?
         the orb code tries to reuse existing socket connection points.
         If binding to any of those fails because another process is using it,
         we don't want to unlink the other server's socket!
         if the bind fails, iiop_connection_server_unix closes the fd for us */
      if (connection->class == GIOP_CONNECTION_SERVER
	  && giop_connection_fd (connection) >= 0)
	unlink (iiop_connection_unix_path (connection));
    }

  if (connection->channel)
    shutdown (giop_connection_fd (connection), 2);

  g_free (IIOP_ADDRESS (connection));  
}

gint 
iiop_connection_family (const GIOPConnection* cnx)
{
  return IIOP_ADDRESS (cnx)->sa_family;
}

gchar *
iiop_connection_unix_path (const GIOPConnection * cnx)
{
  struct sockaddr_un *unixaddr = 
    (struct sockaddr_un *)IIOP_ADDRESS (cnx);

  g_return_val_if_fail (unixaddr->sun_family == AF_UNIX, NULL);
  g_return_val_if_fail (cnx->type == iiop_type_quark, NULL);
  g_return_val_if_fail (cnx->class == GIOP_CONNECTION_SERVER, NULL);

  return unixaddr->sun_path;
}

CORBA_unsigned_short 
iiop_connection_inet_port(const GIOPConnection* cnx)
{
  struct sockaddr *addr = (struct sockaddr *)IIOP_ADDRESS (cnx);

  g_return_val_if_fail (cnx->type == iiop_type_quark, 0);
  g_return_val_if_fail (cnx->class == GIOP_CONNECTION_SERVER, 0);
  
  switch (addr->sa_family)
    {
    case AF_INET:
      return g_ntohs (((struct sockaddr_in *)addr)->sin_port);
#ifdef AF_INET6
    case AF_INET6:
      return g_ntohs (((struct sockaddr_in6 *)addr)->sin6_port);
#endif /* AF_INET6 */
    default:
      ORBit_critical (ORBIT_LOG_DOMAIN_IIOP, "iiop_connection_inet_port: "
		      "Unknown sockaddr family.");
      return 0;
    }
}

gchar*
iiop_connection_hostname ()
{
  static gchar* hostname;
  G_LOCK_DEFINE_STATIC (hostname);

  G_LOCK (hostname);
  if (!hostname)
    {
      char temp [1024];
      struct addrinfo hints, *info;
      gint error;

      gethostname (temp, sizeof (temp) - 1);
      
      memset (&hints, '\0', sizeof (hints));
      hints.ai_flags = AI_CANONNAME;

      error = getaddrinfo (temp, NULL, &hints, &info);

      if (error)
	{
	  ORBit_critical (ORBIT_LOG_DOMAIN_IIOP,
			  "iiop_connection_hostname: getaddrinfo error: %s.",
			  gai_strerror (error));
	  return NULL;
	}

      if (strchr (info->ai_canonname, '.') == NULL)
	{
	  /* This doesn't seem to be a FQDN. We take the numeric value */
	  getnameinfo (info->ai_addr, info->ai_addrlen, temp, sizeof (temp),
		       NULL, 0, NI_NUMERICHOST);
	  if (strcmp (temp, "127.0.0.1") == 0)
	    /* If however we only get 127.0.0.1, we rather take the
             * not-fqdn */
	    hostname = g_strdup (info->ai_canonname);
	  else
	    hostname = g_strdup (temp);
	}
      else
	hostname = g_strdup (info->ai_canonname);

      freeaddrinfo (info);
    }
  G_UNLOCK (hostname);

  return hostname;
}
