#include "linc-private.h"

#if LINC_SSL_SUPPORT
#include <openssl/ssl.h>
#endif

GMainLoop *linc_loop = NULL;

void
linc_init(void)
{
  linc_loop = g_main_new(FALSE);

#if LINC_SSL_SUPPORT
  SSLeay_add_ssl_algorithms();
  linc_ssl_method = SSLv23_method();
  linc_ssl_ctx = SSL_CTX_new(linc_ssl_method);
#endif
}
