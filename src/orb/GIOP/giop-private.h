#ifndef GIOP_PRIVATE_H
#define GIOP_PRIVATE_H 1

#include "config.h"
#include <orbit/orbit-config.h>
#include <orbit/IIOP/giop-types.h>

extern GMainLoop *giop_loop;

#if ORBIT_SSL_SUPPORT
#include <openssl/ssl.h>
SSL_METHOD *giop_ssl_method;
SSL_CTX *giop_ssl_ctx;
#endif

#endif
