#include "linc-private.h"

GMainLoop *linc_loop = NULL;

#if LINC_SSL_SUPPORT
SSL_METHOD *linc_ssl_method;
SSL_CTX *linc_ssl_ctx;
#endif

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
