#ifndef _LINC_PROTOCOL_H_
#define _LINC_PROTOCOL_H_

#include <glib/gmacros.h>

G_BEGIN_DECLS

#include <linc/linc-types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

typedef enum {
	LINC_PROTOCOL_SECURE     = 1<<0,
	LINC_PROTOCOL_NEEDS_BIND = 1<<1
} LINCProtocolFlags;

typedef void (*LINCProtocolSetupFunc)       (int                     fd,
					     LINCConnectionOptions   cnx_flags);
typedef void (*LINCProtocolDestroyFunc)     (int                     fd,
					     const char             *host_info,
					     const char             *serv_info);
typedef int  (*LINCProtocolGetAddrInfoFunc) (const char             *nodename,
					     const char             *servname,
					     const struct addrinfo  *hints,
					     struct addrinfo       **res);
typedef int  (*LINCProtocolGetNameInfoFunc) (const struct sockaddr  *sa,
					     socklen_t               salen,
					     char                   *host,
					     size_t                  hostlen,
					     char                   *serv,
					     size_t                  servlen,
					     int                     flags);
typedef struct sockaddr *
	     (*LINCProtocolGetSockAddrFunc) (const LINCProtocolInfo       *proto,
					     const char                   *hostname,
					     const char                   *service,
					     socklen_t                    *saddr_len);

struct _LINCProtocolInfo {
	const char                 *name;
	int                         family;
	int                         addr_len;
	int                         stream_proto_num;
	LINCProtocolFlags           flags;

	LINCProtocolSetupFunc       setup;
	LINCProtocolDestroyFunc     destroy;
	LINCProtocolGetAddrInfoFunc getaddrinfo;
	LINCProtocolGetNameInfoFunc getnameinfo;
	LINCProtocolGetSockAddrFunc get_sockaddr;
};

LINCProtocolInfo * const linc_protocol_find     (const char *name);
LINCProtocolInfo * const linc_protocol_find_num (const int family);
LINCProtocolInfo * const linc_protocol_all      (void);

int linc_getaddrinfo (const char             *nodename,
		      const char             *servname,
		      const struct addrinfo  *hints,
		      struct addrinfo       **res);
int linc_getnameinfo (const struct sockaddr  *sa,
		      socklen_t               sa_len,
		      char                   *host,
		      size_t                  hostlen,
		      char                   *serv,
		      size_t                  servlen,
		      int                     flags);
void linc_set_tmpdir (const char             *dir);

G_END_DECLS

#endif /* _LINC_PROTOCOL_H_ */
