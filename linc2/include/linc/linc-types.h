#ifndef LINC_TYPES_H
#define LINC_TYPES_H 1

#include <glib.h>
#include <glib-object.h>
#include <linc/linc-config.h>

typedef enum {
  LINC_CONNECTION_SSL
} LINCConnectionOptions;

#ifdef LINC_THREADSAFE

#include <pthread.h>

#define O_MUTEX_DEFINE(x) GMutex* x
#define O_MUTEX_DEFINE_STATIC(x) static GMutex* x
#define O_MUTEX_DEFINE_EXTERN(x) extern GMutex* x
#define O_MUTEX_INIT(x) (x) = g_mutex_new()
#define O_MUTEX_LOCK(x) g_mutex_lock(x)
#define O_MUTEX_UNLOCK(x) g_mutex_unlock(x)
#define O_MUTEX_DESTROY(x) g_mutex_free(x)

#else

#define O_MUTEX_DEFINE(x)
#define O_MUTEX_DEFINE_STATIC(x)
#define O_MUTEX_DEFINE_EXTERN(x)
#define O_MUTEX_INIT(x)
#define O_MUTEX_LOCK(x)
#define O_MUTEX_UNLOCK(x)
#define O_MUTEX_DESTROY(x)

#endif

#endif
