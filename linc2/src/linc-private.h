#ifndef _LINC_PRIVATE_H_
#define _LINC_PRIVATE_H_

#include "config.h"
#include <linc/linc.h>

#ifdef LINC_SSL_SUPPORT

#include <openssl/ssl.h>
#include <openssl/bio.h>
extern SSL_METHOD *linc_ssl_method;
extern SSL_CTX *linc_ssl_ctx;

#endif /* LINC_SSL_SUPPORT */

#endif /* _LINC_PRIVATE_H */
