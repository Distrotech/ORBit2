#ifndef LINC_THREADS_H
#define LINC_THREADS_H 1

#include <glib.h>
#include <linc/linc-config.h>

#undef LINC_THREADSAFE

#if defined(G_THREADS_ENABLED) && defined (LINC_ENABLE_THREADSAFE)
#define LINC_THREADSAFE 1
#endif

#ifdef LINC_THREADSAFE

#include <pthread.h>

#define O_MUTEX_DEFINE(x)          pthread_mutex_t x
#define O_MUTEX_DEFINE_STATIC(x)   static pthread_mutex_t x
#define O_MUTEX_DEFINE_EXTERN(x)   extern pthread_mutex_t x
#define O_MUTEX_INIT(x)            pthread_mutex_init(&(x), &linc_mutex_attrs)
#define O_MUTEX_LOCK(x)            pthread_mutex_lock(&(x))
#define O_MUTEX_UNLOCK(x)          pthread_mutex_unlock(&(x))
#define O_MUTEX_DESTROY(x)         pthread_mutex_destroy(&(x))
#define O_THREAD_DEFINE(x)         pthread_t x
#define O_CONDVAR_DEFINE(x)        pthread_cond_t x
#define O_CONDVAR_DEFINE_STATIC(x) static pthread_cond_t x
extern pthread_mutexattr_t         linc_mutex_attrs;

#else /* LINC_THREADSAFE */

#define O_MUTEX_DEFINE(x)
#define O_MUTEX_DEFINE_STATIC(x)
#define O_MUTEX_DEFINE_EXTERN(x)
#define O_MUTEX_INIT(x)
#define O_MUTEX_LOCK(x)
#define O_MUTEX_UNLOCK(x)
#define O_MUTEX_DESTROY(x)
#define O_THREAD_DEFINE(x)
#define O_CONDVAR_DEFINE(x)
#define O_CONDVAR_DEFINE_STATIC(x)

#endif /* LINC_THREADSAFE */

#endif /* LINC_THREADS_H */
