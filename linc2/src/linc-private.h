#ifndef LINC_PRIVATE_H
#define LINC_PRIVATE_H 1

#include "config.h"
#include <linc/linc-config.h>
#include <linc/linc-types.h>

extern GMainLoop *linc_loop;

#if LINC_SSL_SUPPORT
#include <openssl/ssl.h>
extern SSL_METHOD *linc_ssl_method;
extern SSL_CTX *linc_ssl_ctx;
#endif

#endif
