#ifndef LINC_PRIVATE_H
#define LINC_PRIVATE_H 1

#include "config.h"
#include <linc/linc.h>

#ifdef LINC_SSL_SUPPORT
#include <openssl/ssl.h>
extern SSL_METHOD *linc_ssl_method;
extern SSL_CTX *linc_ssl_ctx;
#endif

#endif
