#ifndef GIOP_PROTOCOL_H
#define GIOP_PROTOCOL_H 1

#include <orbit/IIOP/giop-types.h>
#include <sys/socket.h>
#include <netdb.h>

typedef enum {
  GIOP_PROTOCOL_SECURE = 1<<0,
  GIOP_PROTOCOL_NEEDS_BIND = 1<<1
} GIOPProtocolFlags;

typedef struct {
  const char *name;
  int family;
  int addr_len;
  int stream_proto_num;
  GIOPProtocolFlags flags;
  void (* destroy)(int fd, struct sockaddr *saddr);
  int (* getaddrinfo)(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
  int (* getnameinfo)(const struct sockaddr *sa, socklen_t salen,
		      char *host, size_t hostlen, char *serv, size_t servlen, int flags);
} GIOPProtocolInfo;

GIOPProtocolInfo * const giop_protocol_find(const char *name);
GIOPProtocolInfo * const giop_protocol_find_num(const int family);
int giop_getaddrinfo(const char *nodename, const char *servname,
		     const struct addrinfo *hints, struct addrinfo **res);
int giop_getnameinfo(const struct sockaddr *sa, socklen_t sa_len,
		     char *host, size_t hostlen, char *serv, size_t servlen,
		     int flags);

#endif
