#ifndef IIOP_CONNECTION_H
#define IIOP_CONNECTION_H 1

#include <IIOP/giop-types.h>

/* returns AF_INET, AF_INET6 or AF_UNIX */
gint iiop_connection_family(const GIOPConnection* cnx);
/* returns the path for a AF_UNIX server connection */
gchar *iiop_connection_unix_path(const GIOPConnection* cnx);
/* returns the inet port of a AF_INET or AF_INET6 server connection */
CORBA_unsigned_short iiop_connection_inet_port(const GIOPConnection* cnx);

/* reserves a iiop port for later use by IIOP. Returns TRUE on success */
gboolean iiop_connection_reserve_ipport (const gushort port);

/* creates a server connection for AF_INET6 or AF_INET (in that order)
 * using the reserved IP-port, if available, a random port otherwise */
GIOPConnection *iiop_connection_server_ip (void);

/* creates a server connection for AF_UNIX with the specified unixpath */
GIOPConnection *iiop_connection_server_unix (const gchar* unixpath);

/* You use this to get a pointer to a new (or existing) connection
   that has the specified host/port characteristics */
GIOPConnection *iiop_connection_get(const gchar *host, const gchar *port,
				    gboolean existing_only);

void iiop_init (const char *argv0);

/* returns a pointer to a static buffer with the FQDN of the local host */
gchar* iiop_connection_hostname (void);

#endif /* IIOP_CONNECTION_H */
