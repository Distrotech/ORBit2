#include "giop-private.h"

#if ORBIT_SSL_SUPPORT
#include <openssl/ssl.h>
#endif

GMainLoop *giop_loop = NULL;

void
giop_init(void)
{
  giop_loop = g_main_new(FALSE);

#if ORBIT_SSL_SUPPORT
  SSLeay_add_ssl_algorithms();
  giop_ssl_method = SSLv23_method();
  giop_ssl_ctx = SSL_CTX_new(giop_ssl_method);
#endif
}
