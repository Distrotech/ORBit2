#ifndef LINC_TYPES_H
#define LINC_TYPES_H 1

#include <glib.h>
#include <glib-object.h>
#include <linc/linc-config.h>
#include <sys/types.h>

#ifdef G_THREADS_ENABLED
#define LINC_THREADSAFE 1
#endif

typedef enum {
  LINC_CONNECTION_SSL = 1<<0,
  LINC_CONNECTION_NONBLOCKING = 1<<1
} LINCConnectionOptions;

/* Make Slowaris happy */
#include <time.h>

#ifdef LINC_THREADSAFE

#include <pthread.h>

#define O_MUTEX_DEFINE(x) pthread_mutex_t x
#define O_MUTEX_DEFINE_STATIC(x) static pthread_mutex_t x
#define O_MUTEX_DEFINE_EXTERN(x) extern pthread_mutex_t x
#define O_MUTEX_INIT(x) pthread_mutex_init(&(x), &linc_mutex_attrs)
#define O_MUTEX_LOCK(x) pthread_mutex_lock(&(x))
#define O_MUTEX_UNLOCK(x) pthread_mutex_unlock(&(x))
#define O_MUTEX_DESTROY(x) pthread_mutex_destroy(&(x))
#define O_THREAD_DEFINE(x) pthread_t x
#define O_CONDVAR_DEFINE(x) pthread_cond_t x
#define O_CONDVAR_DEFINE_STATIC(x) static pthread_cond_t x
extern pthread_mutexattr_t linc_mutex_attrs;

#else

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

#endif

typedef struct _LincWatch LincWatch;

#endif
