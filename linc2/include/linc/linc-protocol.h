#ifndef LINC_PROTOCOL_H
#define LINC_PROTOCOL_H 1

#include <linc/linc-types.h>
#include <sys/socket.h>
#include <netdb.h>

typedef enum {
  LINC_PROTOCOL_SECURE = 1<<0,
  LINC_PROTOCOL_NEEDS_BIND = 1<<1
} LINCProtocolFlags;

typedef struct {
  const char *name;
  int family;
  int addr_len;
  int stream_proto_num;
  LINCProtocolFlags flags;
  void (* setup)(int fd, LINCConnectionOptions cnx_flags);
  void (* destroy)(int fd, const char *host_info, const char *serv_info);
  int (* getaddrinfo)(const char *nodename, const char *servname,
		      const struct addrinfo *hints, struct addrinfo **res);
  int (* getnameinfo)(const struct sockaddr *sa, socklen_t salen,
		      char *host, size_t hostlen,
		      char *serv, size_t servlen, int flags);
} LINCProtocolInfo;

LINCProtocolInfo * const linc_protocol_find(const char *name);
LINCProtocolInfo * const linc_protocol_find_num(const int family);
LINCProtocolInfo * const linc_protocol_all(void);
int linc_getaddrinfo(const char *nodename, const char *servname,
		     const struct addrinfo *hints, struct addrinfo **res);
int linc_getnameinfo(const struct sockaddr *sa, socklen_t sa_len,
		     char *host, size_t hostlen, char *serv, size_t servlen,
		     int flags);
void linc_set_tmpdir(const char *dir);

#endif
