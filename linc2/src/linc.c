#include "linc-private.h"

GMainLoop *linc_loop = NULL;

#ifdef LINC_SSL_SUPPORT
SSL_METHOD *linc_ssl_method;
SSL_CTX *linc_ssl_ctx;
#endif

#ifdef LINC_THREADSAFE
pthread_mutexattr_t linc_mutex_attrs;
#endif

void
linc_init(void)
{
#ifdef LINC_THREADSAFE
  pthread_mutexattr_init(&linc_mutex_attrs);
  pthread_mutexattr_settype(&linc_mutex_attrs, PTHREAD_MUTEX_RECURSIVE);
#endif
  if (!g_thread_supported()) g_thread_init (NULL);

  g_type_init();

  linc_loop = g_main_new(FALSE);

#ifdef LINC_SSL_SUPPORT
  SSLeay_add_ssl_algorithms();
  linc_ssl_method = SSLv23_method();
  linc_ssl_ctx = SSL_CTX_new(linc_ssl_method);
#endif
}
